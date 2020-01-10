#include "ImgProcROIFeature.h"
#include "include/NFIQException.h"
#include "include/Timer.hpp"

#include <sstream>

using namespace NFIQ;
using namespace cv;

ImgProcROIFeature::~ImgProcROIFeature()
{

}

std::list<NFIQ::QualityFeatureResult> ImgProcROIFeature::computeFeatureData(
	const NFIQ::FingerprintImageData & fingerprintImage, ImgProcROIFeature::ImgProcROIResults & imgProcResults)
{
	std::list<NFIQ::QualityFeatureResult> featureDataList;

	// check if input image has 500 dpi
	if (fingerprintImage.m_ImageDPI != NFIQ::e_ImageResolution_500dpi)
		throw NFIQ::NFIQException(NFIQ::e_Error_FeatureCalculationError, "Only 500 dpi fingerprint images are supported!");

	Mat img;
	try
	{
		// get matrix from fingerprint image
		img = Mat(fingerprintImage.m_ImageHeight, fingerprintImage.m_ImageWidth, CV_8UC1, (void*)fingerprintImage.data());
	}
	catch (cv::Exception & e)
	{
		std::stringstream ssErr;
		ssErr << "Cannot get matrix from fingerprint image: " << e.what();
		throw NFIQ::NFIQException(NFIQ::e_Error_FeatureCalculationError, ssErr.str());
	}

	NFIQ::Timer timer;
	timer.startTimer();

	// ---------------------------------------------
	// compute ROI (and other features based on ROI)
	// ---------------------------------------------
	try
	{
		imgProcResults = computeROI(img, 16); // block size = 16x16 pixels

		NFIQ::QualityFeatureData fd_roi_pixel_area_mean;
		fd_roi_pixel_area_mean.featureID = "ImgProcROIArea_Mean";
		fd_roi_pixel_area_mean.featureDataType = NFIQ::e_QualityFeatureDataTypeDouble;
		fd_roi_pixel_area_mean.featureDataDouble = imgProcResults.meanOfROIPixels;
		NFIQ::QualityFeatureResult res_roi_pixel_area_mean;
		res_roi_pixel_area_mean.featureData = fd_roi_pixel_area_mean;
		res_roi_pixel_area_mean.returnCode = 0;

		featureDataList.push_back(res_roi_pixel_area_mean);

		if (m_bOutputSpeed)
		{
			NFIQ::QualityFeatureSpeed speed;
			speed.featureIDGroup = "Region of interest";
			speed.featureIDs.push_back("ImgProcROIArea_Mean");
			speed.featureSpeed = timer.endTimerAndGetElapsedTime();
			m_lSpeedValues.push_back(speed);
		}
	}
	catch (cv::Exception & e)
	{
		std::stringstream ssErr;
		ssErr << "Cannot compute feature (ImgProc)ROI area: " << e.what();
		throw NFIQ::NFIQException(NFIQ::e_Error_FeatureCalculationError, ssErr.str());
	}
	catch (NFIQ::NFIQException & e)
	{
		throw e;
	}
	catch (...)
	{
		throw NFIQ::NFIQException(NFIQ::e_Error_FeatureCalculationError, "Unknown exception occurred!");
	}

	return featureDataList;
}

std::string ImgProcROIFeature::getModuleID()
{
	return "NFIQ2_ImgProcROI";
}

std::list<std::string> ImgProcROIFeature::getAllFeatureIDs()
{
	std::list<std::string> featureIDs;
	featureIDs.push_back("ImgProcROIArea_Mean");
	return featureIDs;
}

ImgProcROIFeature::ImgProcROIResults ImgProcROIFeature::computeROI(cv::Mat & img, unsigned int bs)
{
	ImgProcROIResults roiResults;

	// 1. erode image to get fingerprint details more clearly
	Mat erodedImg;
	Mat element(5, 5, CV_8U, Scalar(1));
	erode(img, erodedImg, element);

	// 2. Gaussian blur to get important area
	Mat blurImg;
	GaussianBlur(erodedImg, blurImg, Size(41,41), 0.0);

	// 3. Binarize image with Otsu method
	Mat threshImg;
	threshold(blurImg, threshImg, 0, 255, THRESH_OTSU);

	// 4. Blur image again
	Mat blurImg2;
	GaussianBlur(threshImg, blurImg2, Size(91,91), 0.0);

	// 5. Binarize image again with Otsu method
	Mat threshImg2;
	threshold(blurImg2, threshImg2, 0, 255, THRESH_OTSU);

	// 6. try find white holes in black image
	Mat contImg = threshImg2.clone();
	std::vector<std::vector<Point> > contours;
	std::vector<Vec4i> hierarchy;

	// find contours in image
	findContours( contImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0,0) );

	// if holes are found -> close holes
	if (hierarchy.size() > 2)
	{
		Mat filledImg;
		cvtColor(threshImg2, filledImg, CV_GRAY2BGR);

		for (unsigned int idx = 0; idx < (hierarchy.size() - 2); idx++)
		{	
			drawContours( filledImg, contours, idx, Scalar(0,0,0,0), CV_FILLED, 8, hierarchy );
		}

		cvtColor(filledImg, threshImg2, CV_BGR2GRAY);
	}

	// 7. remove smaller blobs at the edges that are not part of the fingerprint
	Mat ffImg = threshImg2.clone();
	Point point;
	std::vector<Rect> vecRects;
	std::vector<Point> vecPoints;
	while (isBlackPixelAvailable(ffImg, point))
	{
		// execute flood fill algorithm starting with discovered seed
		// and save flooded area on copied image
		Rect rect;
		floodFill(ffImg, point, Scalar(255,255,255,0), &rect);
		vecRects.push_back(rect);
		vecPoints.push_back(point);
	}

	// find largest region based on returned area
	unsigned int maxIdx = 0;
	int maxSize = 0;
	for (unsigned int i = 0; i < vecRects.size(); i++)
	{
		if ((vecRects.at(i).width * vecRects.at(i).height) > maxSize)
		{
			maxIdx = i;
			maxSize = (vecRects.at(i).width * vecRects.at(i).height);
		}
	}

	// now apply floodfill algorithm on all areas that are not the biggest one
	for (unsigned int i = 0; i < vecRects.size(); i++)
	{
		if (i != maxIdx)
		{
			// apply floodfill on original image
			// start seed first detected point
			floodFill(threshImg2, Point(vecPoints.at(i).x, vecPoints.at(i).y), Scalar(255,255,255,0));
		}
	}

	// count ROI pixels ( = black pixels)
	// and get mean value of ROI pixels
	unsigned int noOfROIPixels = 0;
	double meanOfROIPixels = 0.0;
	for (int i = 0; i < threshImg2.rows; i++)
	{
		for (int j = 0; j < threshImg2.cols; j++)
		{
			if (((int)threshImg2.at<uchar>(i,j)) == 0)
			{
				noOfROIPixels++;
				// get gray value of original image (0 = black, 255 = white)
				meanOfROIPixels += (int)img.at<uchar>(i,j);
			}
		}
	}
	// divide value by absolute number of ROI pixels to get mean
	if (noOfROIPixels <= 0)
		meanOfROIPixels = 255.0; // "white" image
	else
		meanOfROIPixels = (meanOfROIPixels/(double)noOfROIPixels);

	// get standard deviation of ORI pixels
	double sumSquare = 0.0;
	for (int i = 0; i < threshImg2.rows; i++)
	{
		for (int j = 0; j < threshImg2.cols; j++)
		{
			if (((int)threshImg2.at<uchar>(i,j)) == 0)
			{
				// get gray value of original image (0 = black, 255 = white)
				unsigned int x = (unsigned int)img.at<uchar>(i,j);
				sumSquare += (((double)x - meanOfROIPixels) * ((double)x - meanOfROIPixels));
			}
		}
	}
	sumSquare = (1.0 / ((double)noOfROIPixels - 1.0) * sumSquare);
	double stdDevOfROIPixels = 0.0;
	if (sumSquare >= 0)
		stdDevOfROIPixels = sqrt(sumSquare);

	// 8. compute and draw blocks
	unsigned int width = img.cols;
	unsigned int height = img.rows;
	Mat bsImg(height, width, CV_8UC1, Scalar(255, 0, 0, 0));

	unsigned int noOfAllBlocks = 0;
	unsigned int noOfCompleteBlocks = 0;
	for (unsigned int i = 0; i < height; i += bs)
	{
		for (unsigned int j = 0; j < width; j += bs)
		{
			unsigned int takenBS_X = bs;
			unsigned int takenBS_Y = bs;
			if ((width - j) < bs)
				takenBS_X = (width - j);
			if ((height - i) < bs)
				takenBS_Y = (height - i);

			Mat block = threshImg2(Rect(j, i, takenBS_X, takenBS_Y));
			noOfAllBlocks++;
			if (takenBS_X == bs && takenBS_Y == bs)
				noOfCompleteBlocks++;
			// count number of black pixels in block
			Scalar m = mean(block);
			if (m.val[0] < 255)
			{
				// take block
				rectangle(bsImg, Point(j, i), Point(j + takenBS_X, i + takenBS_Y), Scalar(0,0,0,0), CV_FILLED);
				roiResults.vecROIBlocks.push_back(Rect(j, i, takenBS_X, takenBS_Y));
			}

		}
	}

	roiResults.chosenBlockSize = bs;
	roiResults.noOfAllBlocks = noOfAllBlocks;
	roiResults.noOfCompleteBlocks = noOfCompleteBlocks;
	roiResults.noOfImagePixels = (img.cols * img.rows);
	roiResults.noOfROIPixels = noOfROIPixels;
	roiResults.meanOfROIPixels = meanOfROIPixels;
	roiResults.stdDevOfROIPixels = stdDevOfROIPixels;

	return roiResults;
}

bool ImgProcROIFeature::isBlackPixelAvailable(cv::Mat & img, cv::Point & point)
{
	bool found = false;
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			if (((int)img.at<uchar>(i,j)) == 0)
			{
				point.x = j;
				point.y = i;
				found = true;
				break;
			}
		}
		if (found)
			break;
	}
	return found;
}

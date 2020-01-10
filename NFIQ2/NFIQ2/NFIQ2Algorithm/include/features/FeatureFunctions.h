#ifndef FEATUREFUNCTIONS_H
#define FEATUREFUNCTIONS_H

#include <list>

#include "include/stdint.h"
#include "include/InterfaceDefinitions.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

void ridgesegment (const cv::Mat& Image, int blksze, double thresh,
				   cv::OutputArray NormImage, cv::Mat& MaskImage,
				   cv::OutputArray MaskIndex);

double ridgeorient (double a, double b, double c);

void diffGrad(const cv::Mat& inBlock, cv::Mat& outBlock);

typedef enum {
	SOBEL_OPERATOR = 0,
	CENTERED_DIFFERENCES,
} ocl_type;

void covcoef (const cv::Mat& imblock, double& a, double& b, double& c, ocl_type compMethod);

double ocl (double a, double b, double c);

uint8_t allfun (const cv::Mat& image);

void getRotatedBlock(const cv::Mat& block, const double orientation, bool padFlag, cv::Mat& rotatedBlock);

void getRidgeValleyStructure (const cv::Mat& blockCropped, std::vector<uint8_t>& ridval,
							  std::vector<double>& dt);
void Conv2D(const cv::Mat& im, const cv::Mat& filter, cv::Mat &ConvOut,
			const cv::Size& imageSize, const cv::Size& dftSize, bool imDFTFlag);
void GaborFilterCx(const int ksize, const double theta, const double freq, const int sigma,
				   cv::Mat& FilterOut);

double calccoh(double gxx, double gyy, double gxy);

double calcof(double gsxavg, double gsyavg);

cv::Mat computeNumericalGradientX(const cv::Mat & mat);
void computeNumericalGradients(const cv::Mat & mat, cv::Mat & grad_x, cv::Mat & grad_y);

void addSamplingFeatures(std::list<NFIQ::QualityFeatureResult> & featureDataList, std::string featurePrefix,
						 std::vector<double> & dataVector);
void addHistogramFeatures(std::list<NFIQ::QualityFeatureResult> & featureDataList, std::string featurePrefix,
						  std::vector<double> & binBoundaries, std::vector<double> & dataVector, int binCount);
void addSamplingFeatureNames(std::list<std::string> & featureNames, const char *prefix);
void addHistogramFeatureNames(std::list<std::string> & featureNames, const char *prefix, int binCount);
#endif

/******************************************************************************/

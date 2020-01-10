#ifndef LCSFEATURE_H
#define LCSFEATURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>

#include "include/stdint.h"
#include "include/InterfaceDefinitions.h"
#include "include/FingerprintImageData.h"
#include "include/features/BaseFeature.h"

static double LCSHISTLIMITS[9] = { 0, 0.70, 0.74, 0.77, 0.79, 0.81, 0.83, 0.85, 0.87 };

class LCSFeature : BaseFeature
{

public:
	LCSFeature(bool bOutputSpeed, std::list<NFIQ::QualityFeatureSpeed> & speedValues)
		: BaseFeature(bOutputSpeed, speedValues)
		, blocksize(32), threshold(0.1), scannerRes(500), padFlag(false)
	{
	};
	virtual ~LCSFeature();

	virtual std::list<NFIQ::QualityFeatureResult> computeFeatureData(
		const NFIQ::FingerprintImageData & fingerprintImage);

	virtual std::string getModuleID();

	virtual void initModule() {};

	virtual std::list<std::string> getAllFeatureIDs();

protected:
	int blocksize;
	double threshold;
	int scannerRes;
	bool padFlag;
};


#endif

/******************************************************************************/

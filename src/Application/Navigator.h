/*
 * demo.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_NAVIGATOR_H_
#define SRC_NAVIGATOR_H_

#include <cstdio>
#include <cmath>
#include <cstdarg>

#include "../AI/_FCN.h"
#include "../Autopilot/_AutoPilot.h"
#include "../Camera/Camera.h"
#include "../Interface/_MavlinkInterface.h"
#include "../UI/UIMonitor.h"
#include "../Utility/util.h"
#include "../Vision/_Flow.h"
#include "../Detector/_DepthDetector.h"
#include "../Detector/_MarkerDetector.h"
#include "../Tracker/_ROITracker.h"
#include "../AI/_SSD.h"
#include "../Navigation/_Universe.h"

#define APP_NAME "NAVIGATOR"

using namespace kai;

namespace kai
{

class Navigator
{
public:
	Navigator();
	~Navigator();

	int m_key;
	bool m_bRun;

	int m_bShowScreen;
	int m_bFullScreen;
	int m_waitKey;

	_Stream* m_pCamFront;
	_AutoPilot* m_pAP;
	_MavlinkInterface* m_pMavlink;
	_CascadeDetector* m_pCascade;
	_Flow* m_pFlow;
	_ROITracker* m_pROITracker;
	_DepthDetector* m_pDD;
	_MarkerDetector* m_pMD;
	_Universe* m_pUniverse;
	_FCN* m_pFCN;
	_SSD* m_pSSD;

	Frame* m_pFrame;
	Mat m_showMat;


	bool start(JSON* pJson);
	void showScreen(void);

	void handleMouse(int event, int x, int y, int flags);
	void showInfo(Mat* pDisplayMat);
	void handleKey(int key);

};

}
#endif

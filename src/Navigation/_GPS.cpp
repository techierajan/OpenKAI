/*
 * _GPS.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_GPS.h"

namespace kai
{

_GPS::_GPS()
{
	m_pZED = NULL;
	m_pMavlink = NULL;
	m_pSF40 = NULL;
	m_initLL.init();
	m_LL.init();
	m_UTM.init();
	m_mavDSfreq = 30;
	m_tStarted = 0;
	m_tNow = 0;
	m_apmMode = 0;
	m_posDiffMax = 10.0;
}

_GPS::~_GPS()
{
}

bool _GPS::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("mavDSfreq", &m_mavDSfreq));
	F_INFO(pK->v("posDiffMax", &m_posDiffMax));
	m_posDiffMax = abs(m_posDiffMax);

	Kiss* pI = pK->o("initLL");
	IF_T(pI->empty());
	F_INFO(pI->v("lat", &m_initLL.m_lat));
	F_INFO(pI->v("lng", &m_initLL.m_lng));
	setLL(&m_initLL);

	m_initUTM = *getUTM();
	m_tStarted = get_time_usec();

	return true;
}

bool _GPS::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink= (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _GPS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _GPS::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_tNow = get_time_usec();

		detect();

		this->autoFPSto();
	}
}

void _GPS::detect(void)
{
	NULL_(m_pMavlink);

	getMavGPS();

	//reset init pos in mode change
	uint32_t apmMode = m_pMavlink->m_msg.heartbeat.custom_mode;
	if(apmMode != m_apmMode)
	{
		m_apmMode = apmMode;
		setLL(&m_initLL);

		if(m_pZED)
			m_pZED->startTracking();

		if(m_pSF40)
			m_pSF40->reset();

		LOG_I("ZED TRACKING START: APM mode: " + i2str(m_apmMode));
	}

	UTM_POS utm = *getUTM();

	if(m_pSF40)
	{
		m_pSF40->setHeading(m_LL.m_hdg);

		//estimate position
		vDouble2 dPos = m_pSF40->getPosDiff();

		utm.m_easting += constrain(dPos.m_x, -m_posDiffMax, m_posDiffMax);
		utm.m_northing += constrain(dPos.m_y, -m_posDiffMax, m_posDiffMax);
	}
	else if(m_pZED)
	{
//		vDouble3 ypr;
//		ypr.init();
//		ypr.m_x = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01 * DEG_RAD;
//		ypr.m_y = m_pMavlink->m_msg.attitude.pitch;
//		ypr.m_z = m_pMavlink->m_msg.attitude.roll;
//		m_pZED->setAttitude(&ypr);

		m_pZED->setHeading(m_LL.m_hdg * DEG_RAD);

		//estimate position
		vDouble4 dPos = m_pZED->getAccumulatedPos();

		utm.m_easting += constrain(dPos.m_x, -m_posDiffMax, m_posDiffMax);
		utm.m_northing += constrain(dPos.m_z, -m_posDiffMax, m_posDiffMax);
		utm.m_alt += constrain(dPos.m_y, -m_posDiffMax, m_posDiffMax);
	}

	setUTM(&utm);
	setMavGPS();

	double dE = m_UTM.m_easting - m_initUTM.m_easting;
	double dN = m_UTM.m_northing - m_initUTM.m_northing;
	double dA = m_UTM.m_alt - m_initUTM.m_alt;
	LOG_I("Dist: E=" + f2str(dE) + ", N=" + f2str(dN) + ", A=" + f2str(dA));
}

/*
time_week
time_week_ms
lat
lon
alt (optional)
hdop (optinal)
vdop (optinal)
vn, ve, vd (optional)
speed_accuracy (optional)
horizontal_accuracy (optional)
satellites_visible <-- required
fix_type <-- required
 */
void _GPS::setMavGPS(void)
{
	NULL_(m_pMavlink);

	mavlink_gps_input_t mavGPS;
	mavGPS.lat = m_LL.m_lat * 1e7;
	mavGPS.lon = m_LL.m_lng * 1e7;
	mavGPS.alt = 0.0;
	mavGPS.gps_id = 0;
	mavGPS.fix_type = 3;
	mavGPS.satellites_visible = 10;
	mavGPS.time_week = 1;
	mavGPS.time_week_ms = (m_tNow-m_tStarted) / 1000;
	mavGPS.ignore_flags = 0b11111111;

	m_pMavlink->gps_input(&mavGPS);
}

void _GPS::getMavGPS(void)
{
	NULL_(m_pMavlink);

	if(m_tNow - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_mavDSfreq);
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_mavDSfreq);
		return;
	}

	m_LL.m_hdg = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
}

void _GPS::setLL(LL_POS* pLL)
{
	NULL_(pLL);
	m_LL = *pLL;

	char pUTMzone[8];
	UTM::LLtoUTM(m_LL.m_lat, m_LL.m_lng, m_UTM.m_northing, m_UTM.m_easting, pUTMzone);
	m_UTM.m_zone = pUTMzone;
	m_UTM.m_alt = m_LL.m_alt;
}

void _GPS::setUTM(UTM_POS* pUTM)
{
	NULL_(pUTM);
	m_UTM = *pUTM;

	UTM::UTMtoLL(m_UTM.m_northing, m_UTM.m_easting, m_UTM.m_zone.c_str(), m_LL.m_lat, m_LL.m_lng);
	m_LL.m_alt = m_UTM.m_alt;
}

LL_POS* _GPS::getLL(void)
{
	return &m_LL;
}

UTM_POS* _GPS::getUTM(void)
{
	return &m_UTM;
}

bool _GPS::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	double dX = m_UTM.m_easting - m_initUTM.m_easting;
	double dY = m_UTM.m_northing - m_initUTM.m_northing;
	double dZ = m_UTM.m_alt - m_initUTM.m_alt;

	pWin->tabNext();
	msg = "Pos: lat=" + f2str(m_LL.m_lat) + ", lng=" + f2str(m_LL.m_lng) + ", alt=" + f2str(m_LL.m_alt) + ", hdg=" + f2str(m_LL.m_hdg);
	pWin->addMsg(&msg);
	msg = "Dist: X=" + f2str(dX) + ", Y=" + f2str(dY) + ", Z=" + f2str(dZ);
	pWin->addMsg(&msg);
	pWin->tabPrev();


	return true;
}

}

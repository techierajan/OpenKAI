/*
 * _GPS.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION_GPS_H_
#define SRC_NAVIGATION_GPS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Filter/FilterBase.h"
#include "../Stream/_ZED.h"
#include "../Sensor/_Lightware_SF40.h"
#include "../Protocol/_Mavlink.h"
#include "../include/UTM.h"

/*
APM setting to use MAV_GPS
GPS_TYPE = MAV (not GPS2_TYPE)
EK2_GPS_TYPE = 2
BRD_SER2_RTSCTS = 0
SERIAL2_BAUD = 115
SERIAL2_PROTOCOL = 1
*/

namespace kai
{

struct LL_POS
{
	double	m_lat;
	double	m_lng;
	double	m_alt;
	double	m_hdg;

	void init(void)
	{
		m_lat = 0.0;
		m_lng = 0.0;
		m_alt = 0.0;
		m_hdg = 0.0;
	}
};

struct UTM_POS
{
	double m_easting;
	double m_northing;
	double m_alt;
	string m_zone;

	void init(void)
	{
		m_easting = 0.0;
		m_northing = 0.0;
		m_alt = 0.0;
		m_zone = "";
	}
};

class _GPS: public _ThreadBase
{
public:
	_GPS(void);
	virtual ~_GPS();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	void setLL(LL_POS* pLL);
	void setUTM(UTM_POS* pUTM);
	LL_POS* getLL(void);
	UTM_POS* getUTM(void);

private:
	void setMavGPS(void);
	void getMavGPS(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_GPS *) This)->update();
		return NULL;
	}

public:
	_ZED*	m_pZED;
	_Lightware_SF40* m_pSF40;
	_Mavlink* m_pMavlink;
	int	m_mavDSfreq;
	uint32_t m_apmMode;

	uint64_t m_tStarted;
	uint64_t m_tNow;

	LL_POS	m_initLL;
	LL_POS	m_LL;
	UTM_POS m_UTM;

	UTM_POS m_initUTM;
	double	m_posDiffMax;


};

}

#endif

#include "APMcopter_base.h"

namespace kai
{

APMcopter_base::APMcopter_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_flightMode = 0;

	m_pidRoll.reset();
	m_pidPitch.reset();
	m_pidYaw.reset();
	m_pidThr.reset();

	m_ctrlRoll.reset();
	m_ctrlPitch.reset();
	m_ctrlYaw.reset();
	m_ctrlThr.reset();
}

APMcopter_base::~APMcopter_base()
{
}

bool APMcopter_base::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	Kiss* pCC;
	APMcopter_PID cPID;

	pCC = pK->o("roll");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidRoll = cPID;

	pCC = pK->o("pitch");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidPitch = cPID;

	pCC = pK->o("thr");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	m_pidThr = cPID;

	pCC = pK->o("yaw");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidYaw = cPID;

	//init controls
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	return true;
}

bool APMcopter_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APMcopter_base::update(void)
{
	this->ActionBase::update();

	NULL_(m_pMavlink);

	sendHeartbeat();

	//update APM status from heartbeat msg
	m_flightMode = m_pMavlink->m_msg.heartbeat.custom_mode;
}

void APMcopter_base::sendHeartbeat(void)
{
	NULL_(m_pMavlink);

	//Sending Heartbeat at 1Hz
	uint64_t timeNow = get_time_usec();
	if (timeNow - m_lastHeartbeat >= USEC_1SEC)
	{
		m_pMavlink->sendHeartbeat();
		m_lastHeartbeat = timeNow;
	}
}

void APMcopter_base::updateAttitude(void)
{
	NULL_(m_pMavlink);

	float pAttitude[3];
	float pRate[3];

	pAttitude[0] = constrain(m_ctrlRoll.m_v, m_pidRoll.m_lim, -m_pidRoll.m_lim);
	pAttitude[1] = constrain(m_ctrlPitch.m_v, m_pidRoll.m_lim, -m_pidRoll.m_lim);
	pAttitude[2] = constrain(m_ctrlYaw.m_v, m_pidRoll.m_lim, -m_pidRoll.m_lim);

	pRate[0] = m_pidRoll.m_rate;
	pRate[1] = m_pidPitch.m_rate;
	pRate[2] = m_pidYaw.m_rate;

	m_pMavlink->set_attitude_target(pAttitude, pRate, 0, 0b01000100);

	/*
	 type_mask
	 uint8_t	Mappings:
	 If any of these bits are set, the corresponding input should be ignored:
	 bit 1: body roll rate,
	 bit 2: body pitch rate,
	 bit 3: body yaw rate.
	 bit 4-bit 6: reserved,
	 bit 7: throttle,
	 bit 8: attitude
	 */
}

void APMcopter_base::updateDistanceSensor(DISTANCE_SENSOR* pSensor)
{
	NULL_(m_pMavlink);
	NULL_(pSensor);

	m_pMavlink->distance_sensor(pSensor->m_type,
			pSensor->m_orientation,
			pSensor->m_maxDistance,
			pSensor->m_minDistance,
			pSensor->m_distance);
}

bool APMcopter_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = *this->getName()+": Flight Mode=" + i2str(m_flightMode);
	pWin->addMsg(&msg);

	return true;
}



}

#include "HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pHM = NULL;
	m_pIN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_grassBoxL.init();
	m_grassBoxF.init();
	m_grassBoxR.init();

	m_grassMinProb = 0.5;
	m_grassClassIdx = 1;

	m_turnTimer = USEC_1SEC;
	m_tTurnSet = 0;
	m_rpmSteer = 0;

	m_nTurnRand = 10;
	m_tTurnRandRange = 100000;
	m_tTurnRandLen = 0;
	m_tTurnRandSet = 0;

	m_iGrassClass = -1;
	m_pGrassL = NULL;
	m_pGrassF = NULL;
	m_pGrassR = NULL;

}

HM_grass::~HM_grass()
{
}

bool HM_grass::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("turnTimer", (int* )&m_turnTimer));
	F_INFO(pK->v("minProb", &m_grassMinProb));
	F_INFO(pK->v("iClass", &m_grassClassIdx));
	F_INFO(pK->v("nTurnRand", &m_nTurnRand));
	F_INFO(pK->v("tTurnRandRange", &m_tTurnRandRange));
	F_INFO(pK->v("iGrassClass", &m_iGrassClass));

	Kiss* pG;

	pG = pK->o("grassBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxL.m_x));
	F_INFO(pG->v("top", &m_grassBoxL.m_y));
	F_INFO(pG->v("right", &m_grassBoxL.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxL.m_w));

	pG = pK->o("grassBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxF.m_x));
	F_INFO(pG->v("top", &m_grassBoxF.m_y));
	F_INFO(pG->v("right", &m_grassBoxF.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxF.m_w));

	pG = pK->o("grassBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxR.m_x));
	F_INFO(pG->v("top", &m_grassBoxR.m_y));
	F_INFO(pG->v("right", &m_grassBoxR.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxR.m_w));

	return true;
}

bool HM_grass::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	if (!m_pIN)
	{
		LOG_E("_ImageNet not found");
		return true;
	}

	OBJECT gBlk;

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxL;
	m_pGrassL = m_pIN->add(&gBlk);
	NULL_F(m_pGrassL);

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxF;
	m_pGrassF = m_pIN->add(&gBlk);
	NULL_F(m_pGrassF);

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxR;
	m_pGrassR = m_pIN->add(&gBlk);
	NULL_F(m_pGrassR);

	return true;
}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pIN);

	IF_(!isActive());
	uint64_t t = get_time_usec();

	//we have/found a good place to go
	if (m_pGrassF->m_prob >= m_grassMinProb)
	{
		IF_(m_tTurnSet == 0);

		if (m_tTurnRandSet == 0)
		{
			m_tTurnRandLen = (rand() % m_nTurnRand) * m_tTurnRandRange;
			m_tTurnRandSet = t;
		}
		else if (t - m_tTurnRandSet > m_tTurnRandLen)
		{
			//reset the timer once finished the random extra turning
			m_tTurnSet = 0;
			m_tTurnRandSet = 0;
			return;
		}
	}

	//set new turn timer and decide the direction
	if (m_tTurnSet == 0)
	{
		m_tTurnSet = t;
		if (m_pGrassL->m_prob > m_pGrassR->m_prob)
			m_rpmSteer = m_steerP;
		else
			m_rpmSteer = -m_steerP;

		return;
	}

	//not yet the time to turn
	IF_(t - m_tTurnSet < m_turnTimer);

	//keep turning
	m_pHM->m_motorPwmL = -m_rpmSteer;
	m_pHM->m_motorPwmR = m_rpmSteer;

}

bool HM_grass::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName() + ": turnTime:" + i2str((int) m_tTurnSet);
	pWin->addMsg(&msg);

	NULL_T(m_pIN);

	Rect r;
	Scalar col;
	int bold;

	vInt42rect(&m_pGrassL->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassL->m_iClass == m_iGrassClass)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	vInt42rect(&m_pGrassF->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassF->m_iClass == m_iGrassClass)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	vInt42rect(&m_pGrassR->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassR->m_iClass == m_iGrassClass)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	return true;
}

}

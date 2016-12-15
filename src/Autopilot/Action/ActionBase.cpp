#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pAM = NULL;
	m_iActiveState = -1;
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));

	return true;
}

bool ActionBase::link(void)
{
	CHECK_F(!this->BASE::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName="";
	F_INFO(pK->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pK->root()->getChildInstByName(&iName));

	CHECK_T(m_pAM==NULL);
	iName="";
	CHECK_T(pK->v("activeState", &iName)==false);
	m_iActiveState = m_pAM->getStateIdx(&iName);

	return true;
}

void ActionBase::update(void)
{
}

bool ActionBase::draw(void)
{
	return this->BASE::draw();
}


}

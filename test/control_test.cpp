#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Control, SetSpell)
{
	dvl::spselflag = true;
	dvl::currSkill = dvl::SPL_FIREBOLT;
	dvl::currSkillType = dvl::RSPLTYPE_CHARGES;
	dvl::SetSkill(false, true);
	EXPECT_EQ(dvl::plr[dvl::myplr]._pAltAtkSkill, dvl::SPL_FIREBOLT);
	EXPECT_EQ(dvl::plr[dvl::myplr]._pAltAtkSkillType, dvl::RSPLTYPE_CHARGES);
	EXPECT_EQ(dvl::gbSkillListFlag, false);
	//EXPECT_EQ(dvl::gbRedrawFlags, REDRAW_ALL);
}

TEST(Control, ClearPanel)
{
	dvl::gbSkillListFlag = true;
	dvl::PressEscKey();
	EXPECT_EQ(dvl::gbSkillListFlag, false);
}

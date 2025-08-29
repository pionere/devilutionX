#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

TEST(Control, StartSkillList)
{
	dvl::HandleSkillBtn(false);
	EXPECT_EQ(dvl::gbSkillListFlag, true);
}

TEST(Control, SetSpell)
{
	dvl::HandleSkillBtn(false);
	// dvl::myplr._pSkillLvlBase[dvl::SPL_FIREBOLT] = 10;
	// dvl::myplr._pMemSkills |= SPELL_MASK(dvl::SPL_FIREBOLT);
	EXPECT_EQ(dvl::gbSkillListFlag, true);

	dvl::SetSkill(false);
	// EXPECT_EQ(dvl::myplr._pAltAtkSkill, dvl::SPL_FIREBOLT);
	// EXPECT_EQ(dvl::myplr._pAltAtkSkillType, dvl::RSPLTYPE_CHARGES);
	EXPECT_EQ(dvl::gbSkillListFlag, false);
}

TEST(Control, EndSkillList)
{
	dvl::HandleSkillBtn(false);

	dvl::PressEscKey();
	EXPECT_EQ(dvl::gbSkillListFlag, false);
}

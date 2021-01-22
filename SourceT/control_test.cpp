#include <gtest/gtest.h>
#include "all.h"

namespace dvl {
extern int pSplType;
}

TEST(Control, SetSpell) {
	dvl::spselflag = true;
	dvl::pSpell = dvl::SPL_FIREBOLT;
	dvl::pSplType = dvl::RSPLTYPE_CHARGES;
	dvl::SetSpell();
	EXPECT_EQ(dvl::spselflag, false);
	EXPECT_EQ(dvl::plr[dvl::myplr]._pRSpell, dvl::SPL_FIREBOLT);
	EXPECT_EQ(dvl::plr[dvl::myplr]._pRSplType, dvl::RSPLTYPE_CHARGES);
	EXPECT_EQ(dvl::spselflag, false);
	//EXPECT_EQ(dvl::gbRedrawFlags, REDRAW_ALL);
}

TEST(Control, ClearPanel) {
	dvl::pinfoflag = true;
	dvl::ClearPanel();
	EXPECT_EQ(dvl::spselflag, false);
}

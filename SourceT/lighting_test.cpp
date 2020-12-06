#include <gtest/gtest.h>
#include "all.h"

TEST(Lighting, CrawlTables) {
	int CrawlNum[19] = { 0, 3, 12, 45, 94, 159, 240, 337, 450, 579, 724, 885, 1062, 1255, 1464, 1689, 1930, 2187, 2460 };

	bool added[40][40];
	memset(added, 0, sizeof(added));

	for (int j = 0; j < 19; j++) {
		int x = 20;
		int y = 20;
		int cr = CrawlNum[j] + 1;
		for (int i = dvl::CrawlTable[cr - 1]; i > 0; i--, cr += 2) {
			int dx = x + dvl::CrawlTable[cr];
			int dy = y + dvl::CrawlTable[cr + 1];
			sprintf(dvl::tempstr, "location %d:%d added twice.", i, j);
			EXPECT_EQ(added[dx][dy], FALSE) << dvl::tempstr;
			added[dx][dy] = TRUE;
		}
	}

	int missed = 0;
	for (int i = -18; i <= 18; i++) {
		for (int j = -18; j <= 18; j++) {
			if (added[i][j])
				continue;
			missed++;
			sprintf(dvl::tempstr, "while checking location %d:%d.", i, j);
			EXPECT_EQ(FALSE, TRUE) << dvl::tempstr;
		}
	}
	sprintf(dvl::tempstr, "total missed %d.", missed);
	EXPECT_EQ(missed, 4) << dvl::tempstr;
}

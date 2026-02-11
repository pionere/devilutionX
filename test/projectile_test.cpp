#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

static int mode;
static int tx, ty;
typedef struct POS32x {
	int x;
	int y;
	int i;
} POS32x;
static std::vector<POS32x> checkPosA;
static std::vector<POS32x> checkPosB;
static std::vector<POS32x> checkPosOffA;
static std::vector<POS32x> checkPosOffB;
static bool CheckAllowPos(int x, int y)
{
	//LogErrorF("MIS", "PosCheck(%d) - %d:%d", mode, x, y);
	std::vector<POS32x>* checks;
	if (mode == 0)
		checks = &checkPosA;
	else
		checks = &checkPosB;
	if (checks->empty() || checks->back().x != x || checks->back().y != y)
		checks->push_back({ x, y, (int)checks->size() });
	return true;
}
#if 0
static bool CheckPos(int x, int y, int xoff, int yoff)
{
	//LogErrorF("MIS", "PosCheck(%d) - %d:%d", mode, x, y);
	std::vector<POS32x> *checks, *checkOff;
	if (mode == 0) {
		checks = &checkPosA;
		checkOff = &checkPosOffA;
	} else {
		checks = &checkPosB;
		checkOff = &checkPosOffB;
	}
	//if (checks->empty() || checks->back().x != x || checks->back().y != y)
		checks->push_back({x, y, (int)checks->size()});
		checkOff->push_back({xoff, yoff, (int)checkOff->size()});
	return true;
}
#endif
#define MIS_VELO_SHIFT      0
#define MIS_BASE_VELO_SHIFT 16
#define MIS_SHIFTEDVEL(x)   ((x) << MIS_VELO_SHIFT)
#if 0
static void GetMissileVel2(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	dx -= sx;
	dy -= sy;
	assert(dx != 0 || dy != 0);
	dxp = (dx - dy);
	dyp = (dy + dx);
	dr = sqrt(dxp * dxp + dyp * dyp);
	missile[mi]._mixvel = (dxp * (v << MIS_BASE_VELO_SHIFT)) / dr;
	missile[mi]._miyvel = (dyp * (v << MIS_BASE_VELO_SHIFT)) / dr;
}

static void GetMissilePos2(int mi)
{
	MissileStruct* mis;
	int mx, my, dx, dy, dqx, dqy, drx, dry;

	mis = &missile[mi];
	mx = mis->_mitxoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);
	my = mis->_mityoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);

	dx = mx + my;
	dy = my - mx;
	// -- keep in sync with LineClearF
	//dx += dx >= 0 ? 32 : -32;
	//dy += dy >= 0 ? 32 : -32;
	dqx = dx / 64;
	drx = dx % 64;
	dqy = dy / 64;
	dry = dy % 64;

	mis->_mix = dqx + mis->_misx;
	mis->_miy = dqy + mis->_misy;
	mis->_mixoff = ((drx - dry) * ASSET_MPL) >> 1;
	mis->_miyoff = ((drx + dry) * ASSET_MPL) >> 2;
}

static void GetMissileVel3(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	dx -= sx;
	dy -= sy;
	assert(dx != 0 || dy != 0);
	dxp = dx;
	dyp = dy;
	dr = sqrt(dxp * dxp + dyp * dyp);
	missile[mi]._mixvel = (dxp * (v << MIS_BASE_VELO_SHIFT) + (dxp >= 0 ? dr - 1 : -(dr - 1))) / dr;
	missile[mi]._miyvel = (dyp * (v << MIS_BASE_VELO_SHIFT) + (dyp >= 0 ? dr - 1 : -(dr - 1))) / dr;
}

static void GetMissilePos3(int mi)
{
	MissileStruct* mis;
	int dx, dy, dqx, dqy, drx, dry;

	mis = &missile[mi];
	dx = mis->_mitxoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);
	dy = mis->_mityoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);

	// -- keep in sync with LineClearF
	//dx += dx >= 0 ? 32 : -32;
	//dy += dy >= 0 ? 32 : -32;
	dqx = dx / 64;
	drx = dx % 64;
	dqy = dy / 64;
	dry = dy % 64;

	mis->_mix = dqx + mis->_misx;
	mis->_miy = dqy + mis->_misy;
	mis->_mixoff = ((drx - dry) * ASSET_MPL) >> 1;
	mis->_miyoff = ((drx + dry) * ASSET_MPL) >> 2;
}

static void MoveMissile(int mi)
{
	missile[mi]._mitxoff += missile[mi]._mixvel;
	missile[mi]._mityoff += missile[mi]._miyvel;
}
#endif
static void MoveMissile4(int mi)
{
	*(double*)&missile[mi]._mitxoff += *(double*)&missile[mi]._mixvel;
	*(double*)&missile[mi]._miVar1 += *(double*)&missile[mi]._miMinDam;
}

static void GetMissileVel4(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	dx -= sx;
	dy -= sy;
	assert(dx != 0 || dy != 0);
	dxp = dx;
	dyp = dy;
	dr = sqrt(dxp * dxp + dyp * dyp);
	*(double*)&missile[mi]._mixvel = (dxp * v) / dr;
	*(double*)&missile[mi]._miMinDam = (dyp * v) / dr;
}

static void GetMissilePos4(int mi)
{
	MissileStruct* mis;
	int dx, dy, dqx, dqy, drx, dry;

	mis = &missile[mi];

	dx = round(*(double*)&mis->_mitxoff);
	dy = round(*(double*)&mis->_miVar1);

	// -- keep in sync with LineClearF
	//dx += dx >= 0 ? 32 : -32;
	//dy += dy >= 0 ? 32 : -32;
	dqx = dx / 64;
	drx = dx % 64;
	dqy = dy / 64;
	dry = dy % 64;

	mis->_mix = dqx + mis->_misx;
	mis->_miy = dqy + mis->_misy;
	mis->_mixoff = ((drx - dry) * ASSET_MPL) >> 1;
	mis->_miyoff = ((drx + dry) * ASSET_MPL) >> 2;
}
#if 0
static void FilterPosExtra(std::vector<POS32x>& av, std::vector<POS32x>& bv)
{
	for (int i = 0; i < (int)av.size() - 2; i++) {
		for (int j = 0; j < (int)bv.size() - 1; j++) {
			if (av[i + 0].x == bv[j + 0].x && av[i + 0].y == bv[j + 0].y
			 //&& av[i + 1].x == bv[j + 1].x && av[i + 1].y == bv[j + 1].y
			 && (!(av[i + 0].x == av[i + 2].x || av[i + 0].y == av[i + 2].y))
			 && abs(av[i + 1].x - av[i + 0].x) + abs(av[i + 1].y - av[i + 0].y) <= 1
			 && abs(av[i + 1].x - av[i + 2].x) + abs(av[i + 1].y - av[i + 2].y) <= 1
			 && av[i + 2].x == bv[j + 1].x && av[i + 2].y == bv[j + 1].y) {
				av.erase(av.begin() + i + 1);
				i--;
				break;
			}
		}
	}
}

static void FilterPosMatch()
{
	// filter consecutive matching pairs at the end
	while (checkPosA.size() >= 2 && checkPosB.size() >= 2
	 && checkPosA[checkPosA.size() - 1].x == checkPosB[checkPosB.size() - 1].x && checkPosA[checkPosA.size() - 1].y == checkPosB[checkPosB.size() - 1].y
	 && checkPosA[checkPosA.size() - 2].x == checkPosB[checkPosB.size() - 2].x && checkPosA[checkPosA.size() - 2].y == checkPosB[checkPosB.size() - 2].y) {
		checkPosA.pop_back();
		checkPosB.pop_back();
	}
	// filter consecutive matching pairs at the front
	while (checkPosA.size() >= 2 && checkPosB.size() >= 2
	 && checkPosA[0].x == checkPosB[0].x && checkPosA[0].y == checkPosB[0].y
	 && checkPosA[1].x == checkPosB[1].x && checkPosA[1].y == checkPosB[1].y) {
		checkPosA.erase(checkPosA.begin());
		checkPosB.erase(checkPosB.begin());
	}

	// filter matching trios
	for (int i = 0; i < (int)checkPosA.size() - 2; i++) {
		for (int j = 0; j < (int)checkPosB.size() - 2; j++) {
			if (checkPosA[i + 0].x == checkPosB[j + 0].x && checkPosA[i + 0].y == checkPosB[j + 0].y
			 && checkPosA[i + 1].x == checkPosB[j + 1].x && checkPosA[i + 1].y == checkPosB[j + 1].y
			 && checkPosA[i + 2].x == checkPosB[j + 2].x && checkPosA[i + 2].y == checkPosB[j + 2].y) {
				checkPosA.erase(checkPosA.begin() + i + 1);
				checkPosB.erase(checkPosB.begin() + j + 1);
				i--;
				break;
			}
		}
	}
}

struct {
	bool operator()(POS32x a, POS32x b) const
	{
		int av = abs(a.x) + abs(a.y);
		av <<= 3;
		av |= a.x < a.y ? 1 << 0 : 0;
		av |= a.x < 0 ? 1 << 1 : 0;
		av |= a.y < 0 ? 1 << 2 : 0;
		int bv = abs(b.x) + abs(b.y);
		bv <<= 3;
		bv |= b.x < b.y ? 1 << 0 : 0;
		bv |= b.x < 0 ? 1 << 1 : 0;
		bv |= b.y < 0 ? 1 << 2 : 0;
		return av < bv;
	}
} checkComp;

static bool CheckRuns()
{
	unsigned i = 0;

	std::sort(checkPosA.begin(), checkPosA.end(), checkComp);
	std::sort(checkPosB.begin(), checkPosB.end(), checkComp);
	FilterPosExtra(checkPosB, checkPosA);
	FilterPosMatch();
	if (checkPosA.size() == checkPosB.size()) {
		for ( ; i < checkPosA.size(); i++) {
			if (checkPosA[i].x != checkPosB[i].x || checkPosA[i].y != checkPosB[i].y)
				break;
			//if (checkPosOffA[i].x != checkPosOffB[i].x || checkPosOffA[i].y != checkPosOffB[i].y)
			//	break;
		}
		if (i == checkPosA.size())
			return true;
		for ( ; i < checkPosA.size(); i++) {
			if (checkPosA[i].x != checkPosB[checkPosA.size() - (i + 1)].x || checkPosA[i].y != checkPosB[checkPosA.size() - (i + 1)].y)
				break;
			//if (checkPosOffA[i].x != checkPosOffB[i].x || checkPosOffA[i].y != checkPosOffB[i].y)
			//	break;
		}
		if (i == checkPosA.size())
			return true;
	}
	// LogErrorF("MIS", "Run to %d:%d failed (%d vs %d) at %d.", tx, ty, checkPosA.size(), checkPosB.size(), i);
	for (i = 0; i < checkPosA.size(); i++) {
		//LogErrorF("MIS", "RunA %2d. %d:%d .. %d:%d", i, checkPosA[i].x, checkPosA[i].y, checkPosOffA[i].x, checkPosOffA[i].y);
		// LogErrorF("MIS", "RunA %2d. %d:%d", checkPosA[i].i, checkPosA[i].x, checkPosA[i].y);
	}
	for (i = 0; i < checkPosB.size(); i++) {
		//LogErrorF("MIS", "RunB %2d. %d:%d .. %d:%d", i, checkPosB[i].x, checkPosB[i].y, checkPosOffB[i].x, checkPosOffB[i].y);
		// LogErrorF("MIS", "RunB %2d. %d:%d", checkPosB[i].i, checkPosB[i].x, checkPosB[i].y);
	}
	return false;
}
#endif
TEST(Projectile, Trajectories)
{
	for (tx = -16; tx < 16; tx++) {
		for (ty = -16; ty < 16; ty++) {
			if (tx == 0 && ty == 0)
				continue;
			//LogErrorF("Ch","Checking %d, %d", tx, ty);
			checkPosA.clear();
			checkPosB.clear();
			checkPosOffA.clear();
			checkPosOffB.clear();
			mode = 0;
			//CheckAllowPos(0, 0);
//			LineClearF(CheckAllowPos, 0, 0, tx, ty);
			//CheckAllowPos(tx, ty);
			/*memset(missile, 0, sizeof(missile));
			GetMissileVel2(0, 0, 0, tx, ty, MIS_SHIFTEDVEL(8));
			while (abs(missile[0]._mix) < abs(tx) || abs(missile[0]._miy) < abs(ty)) {
				missile[0]._mitxoff += missile[0]._mixvel;
				missile[0]._mityoff += missile[0]._miyvel;
				GetMissilePos2(0);
				if (!CheckPos(missile[0]._mix, missile[0]._miy, missile[0]._mixoff, missile[0]._miyoff))
					break;
			}*/
			mode = 1;
			memset(missile, 0, sizeof(missile));
			GetMissileVel4(0, 0, 0, tx, ty, MIS_SHIFTEDVEL(8));
			while (true) {
				MoveMissile4(0);
				GetMissilePos4(0);
				if (abs(missile[0]._mix) >= abs(tx) && abs(missile[0]._miy) >= abs(ty))
					break;
				//if (!CheckPos(missile[0]._mix, missile[0]._miy, missile[0]._mixoff, missile[0]._miyoff))
				if ((missile[0]._mix != 0 || missile[0]._miy != 0) && !CheckAllowPos(missile[0]._mix, missile[0]._miy))
					break;
			}
//			EXPECT_TRUE(CheckRuns());
			//LogErrorF("Ch","Checked %d, %d", tx, ty);
		}
	}
}

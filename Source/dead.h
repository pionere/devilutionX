/**
 * @file dead.h
 *
 * Interface of functions for placing dead monsters.
 */
#ifndef __DEAD_H__
#define __DEAD_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define STONENDX	1
extern DeadStruct dead[MAXDEAD];

void InitDead();
void AddDead(int mnum, bool stone);
void SyncDeadLight();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DEAD_H__ */

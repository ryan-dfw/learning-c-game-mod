#ifndef TE_PHASE_H
#define TE_PHASE_H

#include "game.h"

// 1/60s ticks remaining on clock
#define PVP_START_TICKS (2 * 60 * 60)
#define DEATHMATCH_START_TICKS (2 * 60)
#define MAX_PLAYERS 4

typedef enum {
    PHASE_NONE,
    PHASE_NO_DAMAGE,
    PHASE_NORMAL,
    PHASE_DEATHMATCH,
    PHASE_FINISH
} TE_Phase;

TE_Phase TE_GetPhase();
int TE_IsAlive(int ply);
int TE_GetDeathmatchTicks();
void TE_KillMachine(MachineData *md);
void TE_Phase_Install();
void TE_Phase_MatchLoaded();
void TE_Phase_MatchExit();

#endif

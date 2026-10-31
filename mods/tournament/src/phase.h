#ifndef TE_PHASE_H
#define TE_PHASE_H

#include "game.h"

#define MAX_PLAYERS 4

typedef enum {
    PHASE_NONE,       // not a City Trial match: the mod does nothing
    PHASE_NO_DAMAGE,  // this mod's core feature: machines cannot be damaged
    PHASE_NORMAL,     // resumes normal City Trial rules
    PHASE_DEATHMATCH, // overtime; clock frozen, no stat items, shrinking instakill wall
    PHASE_FINISH      // over: clock runs out normally
} TE_Phase;

TE_Phase TE_GetPhase();

// Time spent in deathmatch phase, in ticks
int TE_GetDeathmatchTicks();

// Tracks if a player or CP has not taken a death in the match
int TE_IsAlive(int ply);

// instakill handler for the overtime wall
void TE_KillMachine(MachineData *md);

void TE_Phase_Install();
void TE_Phase_MatchLoaded();
void TE_Phase_MatchExit();

#endif

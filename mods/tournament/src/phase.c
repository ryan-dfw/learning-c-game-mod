// The match phases: truce -> normal -> overtime (deathmatch) -> finish.
//
// The game clock counts down, and the phase is decided from the time left.
// Four patches into the game make this work:
// one gates damage, one watches deaths, one freezes the clock,
// and the item gate in items.c restricts spawns during the deathmatch.

#include "game.h"
#include "code_patch/code_patch.h"
#include "phase.h"

// Time left on clock when each phase starts, in ticks
// Resume normal game operations when time remaining <= 2mins
// Freeze time and resume deathmatch overtime at <= 2seconds
#define PVP_START_TICKS (2 * 60 * 60)
#define DEATHMATCH_START_TICKS (2 * 60)
#define HP_SUBTRACT_CALLSITE 0x801e1f30
#define DEATH_CALLSITE 0x801e1f74

static TE_Phase phase = PHASE_NONE;
static u8 died[MAX_PLAYERS];
static int deathmatch_ticks = 0;

TE_Phase TE_GetPhase() {
    return phase;
}

int TE_GetDeathmatchTicks() {
    return deathmatch_ticks;
}

// 1 if this player is in the match and has not taken a death yet
int TE_IsAlive(int ply) {
    return ply >= 0 && ply < MAX_PLAYERS && Ply_GetPKind(ply) != PKIND_NONE && !died[ply];
}

static int TE_CountAlive() {
    int count = 0;
    for (int i = 0; i <MAX_PLAYERS; i++) {
        if (TE_IsAlive(i)) {
            count++;
        }
    }
    return count;
}

static int TE_AnyoneDied() {
    for (int i = 0; i <MAX_PLAYERS; i++) {
        if (died[i]) {
            return 1;
        }
    }
    return 0;
}


// Replaces game's "can machines be damaged?" check.
static int TE_IsDamageEnabled() {
    if (phase == PHASE_NO_DAMAGE) {
        return 0;
    }
    return (Gm_GetGameData()->xaa5 & 0x10) != 0;
}

// Ditto for on machine break
static void TE_OnDeath(int ply, DmgLog *dmg_log, int is_bike, MachineKind machine_kind) {
    if (ply >= 0 && ply < MAX_PLAYERS) {
        died[ply] = 1;

        if (phase == PHASE_DEATHMATCH && TE_CountAlive() <= 1) {
            phase = PHASE_FINISH;
        }
    }

    Ply_AddDeath(ply, dmg_log, is_bike, machine_kind);
}

void TE_KillMachine(MachineData *md) {
    if (md->hp <= 0.0f) { return; }
    md->hp = 0.0f;
    TE_OnDeath(Machine_GetRiderPly(md), &md->dmg_log, md->is_bike, md->kind);
}

// Advances through our phases per listed times
static void TE_AdvancePhase() {
    GameData *gd = Gm_GetGameData();
    int ticks_left = gd->time_seconds * 60 - (gd->seconds_passed * 60 + gd->frames_in_second);

    if (phase == PHASE_NO_DAMAGE && ticks_left <= PVP_START_TICKS) {
        phase = PHASE_NORMAL;
    }

    if (phase == PHASE_NORMAL && ticks_left <= DEATHMATCH_START_TICKS) {
        // No need for overtime if all players but one have taken a death
        if (TE_AnyoneDied() || TE_CountAlive() <= 1) {
            phase = PHASE_FINISH;
        } else {
            phase = PHASE_DEATHMATCH;
        }
    }

}

// Runs once per game tick when not paused
// Returning 1 skips the clock for this tick, effectively pausing countdown
static int TE_ClockShouldStop() {
    TE_AdvancePhase();
    if (phase != PHASE_DEATHMATCH) {
        return 0;
    }
    deathmatch_ticks++;
    return 1;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80011460, "", TE_ClockShouldStop, "", 0, 0x80011494)

void TE_Phase_Install() {
    CODEPATCH_HOOKAPPLY(0x80011460);
    CODEPATCH_REPLACECALL(HP_SUBTRACT_CALLSITE, TE_IsDamageEnabled);
    CODEPATCH_REPLACECALL(DEATH_CALLSITE, TE_OnDeath);
}

void TE_Phase_MatchLoaded() {
    phase = Gm_IsInCity() ? PHASE_NO_DAMAGE : PHASE_NONE;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        died[i] = 0;
    }
    deathmatch_ticks = 0;
}

void TE_Phase_MatchExit() {
    phase = PHASE_NONE;
}

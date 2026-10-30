#include "game.h"
#include "code_patch/code_patch.h"
#include "phase.h"

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

static int TE_IsDamageEnabled() {
    if (phase == PHASE_NO_DAMAGE) {
        return 0;
    }
    return (Gm_GetGameData()->xaa5 & 0x10) != 0;
}

static int TE_AnyoneDied() {
    for (int i = 0; i <MAX_PLAYERS; i++) {
        if (died[i]) {
            return 1;
        }
    }
    return 0;
}

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

static int TE_ClockShouldStop() {
    GameData *gd = Gm_GetGameData();
    int ticks_left = gd->time_seconds * 60 - (gd->seconds_passed * 60 + gd->frames_in_second);

    if (phase == PHASE_NO_DAMAGE && ticks_left <= PVP_START_TICKS) {
        phase = PHASE_NORMAL;
    }

    if (phase == PHASE_NORMAL && ticks_left <= DEATHMATCH_START_TICKS) {
        if (TE_AnyoneDied() || TE_CountAlive() <= 1) {
            phase = PHASE_FINISH;
        } else {
            phase = PHASE_DEATHMATCH;
        }
    }

    if (phase == PHASE_DEATHMATCH) {
        deathmatch_ticks++;
        return 1;
    }
    return 0;
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

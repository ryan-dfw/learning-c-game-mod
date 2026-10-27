#include "game.h"
#include "phase.h"
#include "banner.h"
#include "zone.h"

#define ZONE_START_RADIUS 1300.0f
#define ZONE_CLOSE_TICKS (60 * 60)

static int announced = 0;

static float TE_ZoneProgress() {
    float p = TE_GetDeathmatchTicks() * (1.0f / ZONE_CLOSE_TICKS);
    if (p > 1.0f) {
        p = 1.0f;
    }
    return p;
}

float TE_Zone_GetRadius() {
    return ZONE_START_RADIUS * (1.0f - TE_ZoneProgress());
}

static float TE_ZoneDistSq(float x, float z) {
    float dx = x - ZONE_CENTER_X;
    float dz = z - ZONE_CENTER_Z;
    return dx * dx + dz * dz;
}

static void TE_ZoneKillOutside() {
    float radius = TE_Zone_GetRadius();

    MachineData *worst = 0;
    float worst_dist_sq = radius * radius;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!TE_IsAlive(i)) { continue; }

        GOBJ *gobj = Ply_GetMachineGObj(i);
        if (gobj == 0) { continue; }

        MachineData *md = gobj->userdata;
        if (md == 0) { continue; }

        float dist_sq = TE_ZoneDistSq(md->pos.X, md->pos.Z);
        if (dist_sq > worst_dist_sq) {
            worst_dist_sq = dist_sq;
            worst = md;
        }
    }

    if (worst != 0) {
        TE_KillMachine(worst);
    }
}

static void TE_ZoneAnnounce() {
    int ticks = TE_GetDeathmatchTicks();

    if (announced == 0) {
        if (TE_TryShowBanner("Deathmatch! The wall is closing in!", 4 * 60)) { announced = 1; }
    } else if (announced == 1 && ticks >= ZONE_CLOSE_TICKS / 2) {
        if (TE_TryShowBanner("The wall is halfway! Stay near the middle!", 4 * 60)) { announced = 2; }
    } else if (announced == 2 && ticks >= ZONE_CLOSE_TICKS - 15 * 60) {
        if (TE_TryShowBanner("15 seconds until the wall closes!", 4 * 60)) { announced = 3; }
    }
}

void TE_Zone_MatchLoaded() {
    announced = 0;
}

void TE_Zone_MatchExit() {
    announced = 0;
}

void TE_Zone_Frame() {
    if (TE_GetPhase() != PHASE_DEATHMATCH) { return; }
    TE_ZoneAnnounce();
    TE_ZoneKillOutside();
}
// the deathmatch wall: a circle from outside bounds shrinking inward
// characters outside it (edge case, seems impossible) are also killed

#include "game.h"
#include "phase.h"
#include "zone.h"

// Land is 99% inside 1231 units of the center, so the wall starts just outside of it
#define ZONE_START_RADIUS 1300.0f
#define ZONE_CLOSE_TICKS (60 * 60)

// Progress to fully closing
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

// Squared distance from center to avoid work w/ sqrts
static float TE_ZoneDistSq(float x, float z) {
    float dx = x - ZONE_CENTER_X;
    float dz = z - ZONE_CENTER_Z;
    return dx * dx + dz * dz;
}

// Kills the machine at or furthest outside the wall, if any
// limit to one kill per frame
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

void TE_Zone_Frame() {
    if (TE_GetPhase() != PHASE_DEATHMATCH) { return; }
    TE_ZoneKillOutside();
}
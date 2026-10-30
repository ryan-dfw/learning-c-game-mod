#include "hoshi/mod.h"
#include "phase.h"
#include "items.h"
#include "zone.h"
#include "zonegfx.h"

void OnMatchLoaded() {
    TE_Phase_MatchLoaded();
    TE_ZoneGfx_MatchLoaded();
}

void OnMatchExit() {
    TE_Phase_MatchExit();
    TE_ZoneGfx_MatchExit();
}

void OnFrame() {
    TE_Zone_Frame();
}

void OnBoot() {
    TE_Phase_Install();
    TE_Items_Install();
}

ModDesc mod_desc = {
    .name = "Tournament Rules",
    .author = "Ryan",
    .version.major = 1,
    .version.minor = 0,
    .OnBoot = OnBoot,
    .On3DLoadEnd = OnMatchLoaded,
    .On3DExit = OnMatchExit,
    .OnFrameStart = OnFrame
};
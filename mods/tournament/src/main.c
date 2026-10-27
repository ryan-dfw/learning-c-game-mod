#include "hoshi/mod.h"
#include "phase.h"
#include "items.h"
#include "banner.h"
#include "zone.h"
#include "zonegfx.h"

void OnMatchLoaded() {
    TE_Phase_MatchLoaded();
    TE_Banner_MatchLoaded();
    TE_Zone_MatchLoaded();
    TE_ZoneGfx_MatchLoaded();
}

void OnMatchExit() {
    TE_Phase_MatchExit();
    TE_Banner_MatchExit();
    TE_Zone_MatchExit();
    TE_ZoneGfx_MatchExit();
}

void OnFrame() {
    TE_Banner_Frame();
    TE_Zone_Frame();
}

void OnBoot() {
    TE_Phase_Install();
    TE_Items_Install();
    TE_Banner_Install();
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
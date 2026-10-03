#include "game.h"
#include "code_patch/code_patch.h"
#include "hoshi/mod.h"

#define HP_SUBTRACT_CALLSITE 0x801e1f30
#define PVP_START_TICKS (2 * 60 * 60)


int TE_IsDamageEnabled() {
    GameData *gd = Gm_GetGameData();
    grBoxGeneInfo *box_info = *stc_grBoxGeneInfo;

    if (box_info != 0 && box_info->match_subseconds_left > PVP_START_TICKS) {
        return 0;
    }

    return (gd->xaa5 & 0x10) != 0;
}

void OnBoot() {
    CODEPATCH_REPLACECALL(HP_SUBTRACT_CALLSITE, TE_IsDamageEnabled);
}

ModDesc mod_desc = {
    .name = "Tournament Rules",
    .author = "Ryan",
    .version.major = 1,
    .version.minor = 0,
    .OnBoot = OnBoot
};
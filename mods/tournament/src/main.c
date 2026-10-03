#include "game.h"
#include "code_patch/code_patch.h"
#include "hoshi/mod.h"


int TE_IsDamageEnabled() {
    return 0;
}

void OnBoot() {
    CODEPATCH_REPLACEFUNC(Gm_IsDamageEnabled, TE_IsDamageEnabled);
}

ModDesc mod_desc = {
    .name = "Tournament Rules",
    .author = "Ryan",
    .version.major = 1,
    .version.minor = 0,
    .OnBoot = OnBoot
};
#include "game.h"
#include "code_patch/code_patch.h"
#include "phase.h"
#include "items.h"


static int TE_IsPatchKind(ItemKind kind) {
    return (kind >= ITKIND_ACCEL && kind <= ITKIND_ALLUP) ||
        (kind >= ITKIND_ACCELFAKE && kind <= ITKIND_WEIGHTFAKE);
}

static int TE_ItemCreateGate(ItemDesc *desc) {
    if (TE_GetPhase() != PHASE_DEATHMATCH) {
        return 0;
    }

    if (TE_IsPatchKind(desc->kind)) {
        return 1;
    }

    if (desc->kind == ITKIND_BOXBLUE) {
        ItemKind new_kind = HSD_Randi(2) ? ITKIND_BOXRED : ITKIND_BOXGREEN;
        desc->kind = new_kind;
        if (desc->x40 == ITKIND_BOXBLUE) {
            desc->x40 = new_kind;
        }
    }

    return 0;
}

CODEPATCH_HOOKCONDITIONALCREATE(0x8024efa8, "mr 3, 29\n\t", TE_ItemCreateGate, "", 0, 0x8024efa0)

void TE_Items_Install() {
    CODEPATCH_HOOKAPPLY(0x8024efa8);
}
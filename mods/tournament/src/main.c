#include "game.h"
#include "code_patch/code_patch.h"
#include "hoshi/mod.h"
#include "text.h"

#define HP_SUBTRACT_CALLSITE 0x801e1f30
#define PVP_START_TICKS (2 * 60 * 60)
#define DEATH_CALLSITE 0x801e1f74
#define MAX_PLAYERS 4
#define DEATHMATCH_START_TICKS (2 * 60)

typedef enum {
    PHASE_NONE,
    PHASE_NO_DAMAGE,
    PHASE_NORMAL,
    PHASE_DEATHMATCH,
    PHASE_FINISH,
} TE_Phase;

static TE_Phase phase = PHASE_NONE;

static u8 died[MAX_PLAYERS];
static int death_ply = -1;

int TE_IsDamageEnabled() {
    if (phase == PHASE_NO_DAMAGE) {
        return 0;
    }
    return (Gm_GetGameData()->xaa5 & 0x10) != 0;
}

int TE_CountAlive() {
    int count = 0;
    for (int i = 0; i <MAX_PLAYERS; i++) {
        if (Ply_GetPKind(i) != PKIND_NONE && !died[i]) {
            count++;
        }
    }
    return count;
}

int TE_AnyoneDied() {
    for (int i = 0; i <MAX_PLAYERS; i++) {
        if (died[i]) {
            return 1;
        }
    }
    return 0;
}

void TE_OnDeath(int ply, DmgLog *dmg_log, int is_bike, MachineKind machine_kind) {
    if (ply >= 0 && ply < MAX_PLAYERS) {
        died[ply] = 1;
        death_ply = ply;

        if (phase == PHASE_DEATHMATCH && TE_CountAlive() <= 1) {
            phase = PHASE_FINISH;
        }
    }

    Ply_AddDeath(ply, dmg_log, is_bike, machine_kind);
}

int TE_ClockShouldStop() {
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

    return phase == PHASE_DEATHMATCH;
}
CODEPATCH_HOOKCONDITIONALCREATE(0x80011460, "", TE_ClockShouldStop, "", 0, 0x80011494)

#define EventBanner_Create ((void (*)(int, int))0x80113fb4)
#define PVP_BANNER_KIND EVKIND_SAMEITEM
#define PVP_BANNER_FRAMES (5 * 60)
#define EventText_Show ((void (*)(int))0x801169fc)
#define PVP_BANNER_MSG_ID (2 + PVP_BANNER_KIND)
#define PVP_TEXT "The truce is over! Machines can be damaged now!"

static u8 msg_head[] = {0x12, 0x18, 0x16, 0x0c, 0xbb, 0xbb, 0xbb, 0x0e, 0x00, 0xb3, 0x00, 0xb3};
static u8 msg_tail[] = {0x03, 0x0f, 0x0d, 0x17, 0x19, 0x13, 0x00};

static u8 pvp_message[160];
static int pvp_banner_on = 0;
static int pvp_announced = 1;

void TE_EncodeMessage(u8 *out, char *text) {
    int n = 0;

    for (int i = 0; i < sizeof(msg_head); i++) {
        out[n++] = msg_head[i];
    }

    for (; *text != 0; text++) {
        char c = *text;

        if (c == ' ') {
            out[n++] = 0x1a;
            continue;
        }

        out[n++] = 0x20;
        if (c >= 'A' && c <= 'Z') {
            out[n++] = c - 'A' + 0x0a;
        } else if (c >= 'a' && c <= 'z') {
            out[n++] = c - 'a' + 0x24;
        } else if (c >= '0' && c <= '9') {
            out[n++] = c - '0';
        } else if (c == '!') {
            out[n++] = 0xec;
        } else if (c == ',') {
            out[n++] = 0xe6;
        } else if (c == '\'') {
            out[n++] = 0xf3;
        } else if (c == '-') {
            out[n++] = 0xfe;
        } else {
            out[n++] = 0xe7;
        }
    }

    for (int i = 0; i < sizeof(msg_tail); i++) {
        out[n++] = msg_tail[i];
    }
}

void TE_EventText_Show(int msg_id) {
    EventText_Show(msg_id);

    if (pvp_banner_on && msg_id == PVP_BANNER_MSG_ID) {
        Text *t = (Text *)Gm_Get3dData()->x48;
        if (t != 0) {
            t->text_start = pvp_message;
        }
    }
}

void TE_ShowBanner(char *text, int frames) {
    TE_EncodeMessage(pvp_message, text);
    pvp_banner_on = 1;
    EventBanner_Create(PVP_BANNER_KIND, frames);
}

static char death_text[] = "Player 1 is down! 4 left";

void TE_ShowDeathBanner() {
    if (death_ply < 0) {
        return;
    }

    if (!Gm_IsInCity()) {
        death_ply = -1;
        return;
    }

    if (Gm_Get3dData()->xbe8 != 0) {
        return;
    }

    death_text[7] = '1' + death_ply;
    death_text[18] = '0' + TE_CountAlive();
    TE_ShowBanner(death_text, 3 * 60);
    death_ply = -1;
}

void OnMatchLoaded() {
    pvp_announced = !Gm_IsInCity();
    phase = Gm_IsInCity() ? PHASE_NO_DAMAGE : PHASE_NONE;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        died[i] = 0;
    }
    death_ply = -1;
}

void OnMatchExit() {
    phase = PHASE_NONE;
    pvp_announced = 1;
    pvp_banner_on = 0;
    death_ply = -1;
}

void OnFrame() {
    if (pvp_banner_on && Gm_Get3dData()->xbe8 == 0) {
        pvp_banner_on = 0;
    }

    TE_ShowDeathBanner();

    if (pvp_announced) {
        return;
    }

    grBoxGeneInfo *box_info = *stc_grBoxGeneInfo;
    if (box_info == 0) {
        return;
    }

    int ticks = box_info->match_subseconds_left;
    if (ticks <= 0 || ticks > PVP_START_TICKS) {
        return;
    }

    if (Gm_Get3dData()->xbe8 != 0) {
        return;
    }

    TE_ShowBanner(PVP_TEXT, PVP_BANNER_FRAMES);
    pvp_announced = 1;
}

void OnBoot() {
    CODEPATCH_HOOKAPPLY(0x80011460);
    CODEPATCH_REPLACECALL(HP_SUBTRACT_CALLSITE, TE_IsDamageEnabled);
    CODEPATCH_REPLACECALL(DEATH_CALLSITE, TE_OnDeath);
    CODEPATCH_REPLACECALL(0x80127674, TE_EventText_Show);
    CODEPATCH_REPLACECALL(0x801277b0, TE_EventText_Show);
    CODEPATCH_REPLACECALL(0x801278e8, TE_EventText_Show);
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
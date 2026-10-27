#include "game.h"
#include "code_patch/code_patch.h"
#include "text.h"
#include "phase.h"
#include "banner.h"


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
static char death_text[] = "Player 1 is down! 4 left";

static void TE_EncodeMessage(u8 *out, char *text) {
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

static void TE_EventText_Show(int msg_id) {
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

int TE_TryShowBanner(char *text, int frames) {
    if (Gm_Get3dData()->xbe8 != 0) { return 0; }
    TE_ShowBanner(text, frames);
    return 1;
}

static void TE_ShowDeathBanner() {
    int ply = TE_GetLastDeath();
    if (ply < 0) {
        return;
    }

    if (!Gm_IsInCity()) {
        TE_ClearLastDeath();
        return;
    }

    if (Gm_Get3dData()->xbe8 != 0) {
        return;
    }

    death_text[7] = '1' + ply;
    death_text[18] = '0' + TE_CountAlive();
    TE_ShowBanner(death_text, 3 * 60);
    TE_ClearLastDeath();
}

static void TE_ShowTruceBanner() {
    if (pvp_announced) { return; }
    grBoxGeneInfo *box_info = *stc_grBoxGeneInfo;
    if (box_info == 0) { return; }

    int ticks = box_info->match_subseconds_left;
    if (ticks <= 0 || ticks > PVP_START_TICKS) { return; }

    if (Gm_Get3dData()->xbe8 != 0) { return;}

    TE_ShowBanner(PVP_TEXT, PVP_BANNER_FRAMES);
    pvp_announced = 1;
}

void TE_Banner_Install() {
    CODEPATCH_REPLACECALL(0x80127674, TE_EventText_Show);
    CODEPATCH_REPLACECALL(0x801277b0, TE_EventText_Show);
    CODEPATCH_REPLACECALL(0x801278e8, TE_EventText_Show);
}

void TE_Banner_MatchLoaded() {
    pvp_announced = !Gm_IsInCity();
}

void TE_Banner_MatchExit() {
    pvp_announced = 1;
    pvp_banner_on = 0;
}

void TE_Banner_Frame() {
    if (pvp_banner_on && Gm_Get3dData()->xbe8 == 0) {
        pvp_banner_on = 0;
    }

    TE_ShowDeathBanner();
    TE_ShowTruceBanner();
}
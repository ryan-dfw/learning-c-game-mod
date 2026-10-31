// Claude wrote 100% of this; it works but i can not understand it; i am not a graphics dev
//
// Draws the deathmatch wall as a see-through red curtain around the edge of the circle.
//
// The game draws everything through "GX links": each object can register a draw callback on a
// numbered link, and every player camera calls the callbacks of its links, three times per frame
// (once per pass). We register one callback on a link the cameras use, and draw in the pass for
// see-through things.

#include "game.h"
#include "phase.h"
#include "zone.h"
#include "zonegfx.h"

// Where the wall is drawn. Link 12 is rendered by the player cameras after machines and items.
#define ZONEGFX_LINK 12
#define ZONEGFX_PRI 100

// The game draws in three passes per camera: 0 solid, 1 cut-out, 2 see-through.
#define ZONEGFX_PASS 2

// The wall is a ring of this many flat panels, each one 360 / 48 = 7.5 degrees wide.
#define WALL_SEGMENTS 48
#define COS_STEP 0.99144486f
#define SIN_STEP 0.13052619f

#define WALL_BOTTOM_Y (-50.0f)
#define WALL_TOP_Y 1500.0f

// Game functions, called by address.
#define CObj_GetCurrentCam ((COBJ *(*)())0x80402f5c)
#define CObj_GetViewMtx ((void (*)(COBJ *, Mtx *))0x80402704)
#define GX_SetBlendMode ((void (*)(GXBlendMode, GXBlendFactor, GXBlendFactor, GXLogicOp))0x803cf820)
#define GX_SetColorUpdate ((void (*)(GXBool))0x803cf874)
#define HSD_StateInvalidate ((void (*)(int))0x803f898c)

static GOBJ *wall_gobj = 0;

// Runs once per camera per pass. Draws the wall as a see-through red curtain.
static void TE_ZoneGfx_Draw(GOBJ *g, int pass) {
    if (pass != ZONEGFX_PASS) { return; }
    if (g != wall_gobj) { return; }
    if (TE_GetPhase() != PHASE_DEATHMATCH) { return; }

    COBJ *cam = CObj_GetCurrentCam();
    if (cam == 0) { return; }

    Mtx view;
    CObj_GetViewMtx(cam, &view);

    // Plain colored triangles: no lighting, no textures, colors come from the vertices.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, Register, Vertex, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // See-through, drawn in front of or behind things correctly, but never hides what is behind it.
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(1, GX_LEQUAL, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetColorUpdate(1);

    // Our vertices are world positions, so the only matrix needed is the camera's.
    GXLoadPosMtxImm(&view, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    float radius = TE_Zone_GetRadius();
    float x = radius;
    float z = 0.0f;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (WALL_SEGMENTS + 1) * 2);
    for (int i = 0; i <= WALL_SEGMENTS; i++) {
        GXPosition3f32(ZONE_CENTER_X + x, WALL_BOTTOM_Y, ZONE_CENTER_Z + z);
        GXColor4u8(255, 30, 30, 150);
        GXPosition3f32(ZONE_CENTER_X + x, WALL_TOP_Y, ZONE_CENTER_Z + z);
        GXColor4u8(255, 30, 30, 35);

        // turn (x, z) by 7.5 degrees
        float next_x = x * COS_STEP - z * SIN_STEP;
        float next_z = x * SIN_STEP + z * COS_STEP;
        x = next_x;
        z = next_z;
    }

    // The game caches its graphics settings. We changed them behind its back, so make it forget.
    HSD_StateInvalidate(-1);
}

void TE_ZoneGfx_MatchLoaded() {
    wall_gobj = 0;
    if (!Gm_IsInCity()) { return; }

    wall_gobj = GObj_Create(0, 0, 0);
    GObj_AddGXLink(wall_gobj, TE_ZoneGfx_Draw, ZONEGFX_LINK, ZONEGFX_PRI);
}

void TE_ZoneGfx_MatchExit() {
    // The game throws away every object when the scene ends, so just forget ours.
    wall_gobj = 0;
}
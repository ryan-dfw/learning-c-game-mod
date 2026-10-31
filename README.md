# Kirby Air Ride: Tournament Edition

A mod for Kirby Air Ride Deluxe Edition wherein kills are restricted to the last two minutes of gameplay, with an overtime instakill wall shrinking in over 60 seconds if nobody has died by then.

A mod to a mod using a framework to mod a gamecube game - a toy project for my first messing about in C.

Written in C on top of [hoshi](https://github.com/UnclePunch/hoshi), UnclePunch's mod framework for the game. This repo is only my src; original ISO, Deluxe edition, and Hoshi framework not included here.

## The rules

Built for a 7:00 City Trial match but will work for any time longer than 2mins

| Phase | When | What happens |
| --- | --- | --- |
| Truce | Start until 2:00 left | Impossible to kill a player/cp or die - no health loss, but normal patch loss / knockback on dmg |
| Normal | 2:00 until 0:02 left | Regular City Trial, no changes |
| Overtime | At 0:02, unless a player/CP has already taken a death | The clock freezes. Stat patches stop spawning and blue boxes turn into red or green ones. A wall of instant death closes in from the edge of the map. |
| Finish | At 0:02 if a player/CP has already taken a death, or in overtime once all players but one have | The clock just runs out like normal. |

Some details:

- "Taken a death" - particular vocabulary as each player character is indestructible, but rides a machine which can be broken. So taking a death does not remove a player from the current game.
- The wall starts just outside the playable land, shrinks to nothing over 60 seconds, and kills any machine outside it. It always forces a winner but does not lend enough time for cleaning up 2nd place's stats.
- There is deliberately no messaging - it seems more enjoyable to allow more observational players an informational advantage.
- I asked Claude for the wall graphics; gfx is beyond my current understanding and not something i imagine i'll go on to work in.

### Layout

All code is in `mods/tournament/src/`:

| File | Job |
| --- | --- |
| `main.c` | Registers the mod with hoshi and wires up the callbacks. |
| `phase.c` / `phase.h` | The phase state machine: damage gate, death tracking, clock freeze. |
| `items.c` / `items.h` | Item spawn rules for overtime. |
| `zone.c` / `zone.h` | The wall's size over time and the kill check. |
| `zonegfx.c` / `zonegfx.h` | Draws the wall. (all Claude) |

## Building

I've only built and played this on macOS with Dolphin. I have **not** tested it on real hardware.

Tested with:

- hoshi at commit `7b4a7d0`
- KAR-Deluxe at commit `a9a16a8`
- devkitPPC from devkitPro, Python 3, and an unmodified NTSC-U Kirby Air Ride ISO

Since this repo is only my source, you build it inside a hoshi project:

1. Clone [KAR-Deluxe](https://github.com/UnclePunch/KAR-Deluxe) at `a9a16a8` and set up its build following its `BUILD.md`.
2. Put hoshi at `7b4a7d0` in `externals/hoshi`. KAR-Deluxe's own submodule pointer at that commit is stale, and the build fails without the newer hoshi.
3. Copy `mods/tournament/` from this repo into KAR-Deluxe's `mods/` folder.
4. Build as KAR-Deluxe's `BUILD.md` says.

I've only ever tested it alongside KAR-Deluxe's other mods, not on its own.

<details>
<summary>Build problems I hit on macOS (and fixes)</summary>

- **BSD `cp`** rejects `cp -a -r`. Drop the `-r` in the Makefile, since `-a` already recurses.
- **`xdelta`:** Homebrew installs a binary called `xdelta3`, but the scripts call `xdelta`. Symlink it.
- **Boot crash ("Could not alloc ... from heap -1"):** hoshi's `dol_inject.py` places its loader at exactly the game's initial stack pointer. Early in boot the game saves a return address to `4(r1)`, which is the loader's second word, so it overwrites it. Whether that breaks anything depends on which source file the build happens to list first. On my machine it did. The fix is one line in `dol_inject.py`: place the payload at `align(stack_addr + 8, 32)` instead of at `stack_addr`.

</details>

## Known limits

- Dolphin only, no real-hardware testing.

## Claude's Section

I wanted to learn how to write toy C; i'm not at Ghidra level. So all reverse engineering was performed by Claude; i was provided the call and hook locations to what i asked for.
Ditto for the graphic effects - i can't learn everything at once, so let me Claude my way to that not being the blocker from getting the mod & learning i want accomplished.


Here are some of Claude's notes on what it found.

| Patch | Where | What it does |
| --- | --- | --- |
| Damage gate | Call at `0x801e1f30`, inside the machine damage function | Swaps the game's "is damage enabled?" check for ours, which says no during the truce. |
| Death hook | Call at `0x801e1f74` | Sees every machine KO so we can track who has died. |
| Clock freeze | Hook at `0x80011460` in the game's per-tick update | Skips the clock increment. It also gives a tick counter that stops while paused, which drives the wall. |
| Item gate | Hook at `0x8024efa8` in `Item_Create` | Every item spawn in the game passes through here, so one hook can cancel stat patches and swap blue boxes. |
| Wall kill | Checked every frame | Position test: is each machine's X/Z outside the circle? Kill the one furthest out. |
| Wall visuals | A draw callback on one of the game's render links | Each player camera calls it. It draws a 48-panel see-through ring using raw GX (the GameCube's graphics API) calls. |


## Credits and license

- [UnclePunch](https://github.com/UnclePunch) for hoshi and KAR-Deluxe, which this is built on.
- Kirby Air Ride is by HAL Laboratory and Nintendo. No game files are included here.

Released under the GPL-3.0, see `LICENSE`. Copyright (C) 2026 Ryan.

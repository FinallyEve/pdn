#pragma once

#include "state/state-machine.hpp"
#include "game/player.hpp"
#include "game/progress-manager.hpp"
#include "game/konami-metagame.hpp"

/*
 * KonamiMetaGame - Master progression system for FDN minigames.
 *
 * Flow:
 * 1. Player encounters FDN -> triggers app switch from Quickdraw
 * 2. First encounter: Easy mode -> Win -> Button awarded -> konamiProgress updated
 * 3. Button replay: No new rewards
 * 4. All 7 buttons -> Konami code entry -> 13 inputs -> hard mode unlocked
 * 5. Hard mode -> Win -> Boon awarded -> colorProfileEligibility updated
 * 6. Mastery replay -> Mode select -> Launch game with correct difficulty
 *
 * 35-state layout defined in konami-metagame.hpp (parent directory)
 */

// Class definition is in include/game/konami-metagame.hpp
// This file re-exports it for backwards compatibility with existing includes.

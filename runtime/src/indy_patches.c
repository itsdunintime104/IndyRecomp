/* Game patches for LEGO Indiana Jones (contract: engine windows/runtime/include/game_hooks.h).
 * HP's patches are keyed by HP call sites; Indy has none yet, so Sleep keeps its time. */
#include "game_hooks.h"

uint32_t game_patch_sleep_ms(uint32_t guest_ret, uint32_t ms)
{
    (void)guest_ret;
    return ms;
}

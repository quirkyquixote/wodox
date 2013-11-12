
#include <string.h>
#include "play_private.h"

/*----------------------------------------------------------------------------
 * Save the state of the level in the state stack.
 *----------------------------------------------------------------------------*/
void
game_push_state(void)
{
    memcpy(game.state_stack_top, &game.cs, sizeof(struct state));

    if (++game.state_stack_top == game.state_stack + STATE_STACK_LEN) {
	game.state_stack_top = game.state_stack;
    }

    if (game.state_stack_top == game.state_stack_bottom &&
	++game.state_stack_bottom == game.state_stack + STATE_STACK_LEN) {
	game.state_stack_bottom = game.state_stack;
    }
}

/*----------------------------------------------------------------------------
 * Load the state of the level from the state stack.
 *----------------------------------------------------------------------------*/
void
game_pop_state(void)
{
    if (game.state_stack_top != game.state_stack_bottom) {
	if (game.state_stack_top == game.state_stack) {
	    game.state_stack_top = game.state_stack + STATE_STACK_LEN;
	}
	--game.state_stack_top;
    }

    memcpy(&game.cs, game.state_stack_top, sizeof(struct state));
}


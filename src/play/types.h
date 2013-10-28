/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#pragma once

#include "../core/list.h"
#include "../media/media.h"


#include <unistd.h>		// For rand
#include <math.h>		// For hypot
#include <errno.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Table indices
 *----------------------------------------------------------------------------*/
static inline uint16_t idx_bk (uint16_t idx) { return idx - 1; }
static inline uint16_t idx_ft (uint16_t idx) { return idx + 1; }
static inline uint16_t idx_lf (uint16_t idx) { return idx - SIZE; }
static inline uint16_t idx_rt (uint16_t idx) { return idx + SIZE; }
static inline uint16_t idx_dn (uint16_t idx) { return idx - SIZE_2; }
static inline uint16_t idx_up (uint16_t idx) { return idx + SIZE_2; }

/*----------------------------------------------------------------------------
 * These define object flags.
 *----------------------------------------------------------------------------*/
enum {
    SPRITE		= 0x0f,	// Mask for the sprite.
    VISIBLE		= 0x10,	// If visible, there must be a sprite.
    SOLID		= 0x20,	// Prevents objects from entering its space.
    HEAVY		= 0x40,	// Affected by forces.
    ACTIVE		= 0x80,	// Generates a signal.
};

/*----------------------------------------------------------------------------
 * These define object types.
 *----------------------------------------------------------------------------*/
enum {
    EMPTY	=  (0x0),
    GHOST	=  (0x0 | SOLID),
    GROUND	=  (0x0 | VISIBLE | SOLID),
    CRATE	=  (0x1 | SOLID | HEAVY),
    PLAYER	=  (0x2 | SOLID | HEAVY),
    SMALL	=  (0x3 | HEAVY),
    BELT_LF_0	=  (0x4 | VISIBLE | SOLID),
    BELT_LF_1	=  (0x4 | VISIBLE | SOLID | ACTIVE),
    BELT_BK_0	=  (0x5 | VISIBLE | SOLID),
    BELT_BK_1	=  (0x5 | VISIBLE | SOLID | ACTIVE),
    BELT_RT_0	=  (0x6 | VISIBLE | SOLID),
    BELT_RT_1	=  (0x6 | VISIBLE | SOLID | ACTIVE),
    BELT_FT_0	=  (0x7 | VISIBLE | SOLID),
    BELT_FT_1	=  (0x7 | VISIBLE | SOLID | ACTIVE),
    MOVING_0	=  (0x8 | SOLID),
    MOVING_1	=  (0x8 | SOLID | ACTIVE),
    BUTTON_0	=  (0x9 | VISIBLE | SOLID),
    BUTTON_1	=  (0x9 | VISIBLE | SOLID | ACTIVE),
    SWITCH_0	=  (0xa | VISIBLE | SOLID),
    SWITCH_1	=  (0xb | VISIBLE | SOLID | ACTIVE),
};

/*----------------------------------------------------------------------------
 * These define object movement.
 *----------------------------------------------------------------------------*/
enum {
    STILL		= 0,	// Not moving.
    DIR_DN		= 1,	// Moving down.
    DIR_UP		= 2,	// Moving up.
    DIR_LF		= 3,	// Moving left.
    DIR_RT		= 4,	// Moving right.
    DIR_BK		= 5,	// Moving back.
    DIR_FT		= 6,	// Moving front.
    WARP		= 7,	// Warping outside the level.
};

/*----------------------------------------------------------------------------
 * These define the displacement and bounds to operate on raw data.
 *----------------------------------------------------------------------------*/
static const int16_t offset[] = { 
    [STILL] = 0,
    [DIR_DN] =  -SIZE_2,
    [DIR_UP] =  SIZE_2,
    [DIR_LF] = -SIZE,
    [DIR_RT] =  SIZE,
    [DIR_BK] = -1, 
    [DIR_FT] = 1, 
    [WARP] = 0 
};

static const uint16_t bounds[] = {
    [STILL] = 0,
    [DIR_DN] =  SIZE_3,
    [DIR_UP] =  SIZE_3,
    [DIR_LF] = SIZE_2,
    [DIR_RT] =  SIZE_2,
    [DIR_BK] = SIZE, 
    [DIR_FT] = SIZE, 
    [WARP] = 0 
};

/*----------------------------------------------------------------------------
 * Some more things...
 *----------------------------------------------------------------------------*/
#define PUSH_DELAY 1		// Number of frames before the wodox can push another object.
#define OBJECT_POOL_SIZE 1000	// Number of allocated objects.
#define STATE_STACK_LEN 10	// Size of the "undo" stack.
#define RECORD_MOVE_LIST_SIZE 10000
				// Number of moves that may be remembered to replay a level (unimplemented). 


/*----------------------------------------------------------------------------
 * Objects are movable and have states. The player, the crates, moving blocks,
 * etc. are objects. They are indexed in a grid to easily locate them in the
 * scene and as a double linked list to easily iterate over them.
 *----------------------------------------------------------------------------*/
struct object {
    uint8_t type;			// Type of object.
    uint16_t idx;			// Position.
    uint8_t dir;			// Direction of movement.
    uint8_t dsp;			// How many steps has the object moved between spaces.
};

/*----------------------------------------------------------------------------
 * Static circuits connect buttons to positions on the map.
 *----------------------------------------------------------------------------*/
struct static_circuit {
    struct static_circuit *next;// Next in list
    uint16_t size;		// Size of the tree.
    uint16_t *tree;		// Boolean operation.
    uint16_t idx;			// Position.
};

/*----------------------------------------------------------------------------
 * Dynamic circuits connect buttons to objects.
 *----------------------------------------------------------------------------*/
struct dynamic_circuit {
    struct dynamic_circuit *next;// Next in list
    uint16_t size;		// Size of the tree.
    uint16_t *tree;		// Boolean operation.
    struct object *obj;		// Object.
};

/*----------------------------------------------------------------------------
 * The level records are basically pairs <time,key>.
 *----------------------------------------------------------------------------*/
struct record {
    uint32_t time;		// When the key was pushed.
    int8_t key;			// Whick key.
};

/*----------------------------------------------------------------------------
 * The scene is represented by all the following.
 *
 * First we have the objects. They are allocated in a "pool" that has space
 * enough for all of them and indexed in two doubly-linked lists. When a new
 * object is needed it is traken from the unused object list and inserted in
 * the used object list.
 *
 * The static map defines properties for each space. The name is misleading 
 * because object movement causes changes in the spaces they occupy. 
 *
 * The forces map defines where should dynamic objects in each space move. 
 * Note that there are forces not defined in this map, specifically those 
 * generated by conveyor belts.
 *
 * The object map holds pointers to the objects occupying each world position
 * for those spaces that hold one. This one is exactly what it says in the 
 * tin.
 *
 * These four are grouped in a structure called a state. This state can be
 * easily copied to make a snapshot of what the level was in an arbitrary point
 * in time.
 *
 * The state contains also a number of other control variables that handle
 * time, the state of the player object and the solution recording.
 *----------------------------------------------------------------------------*/
struct state {
    struct object objects[OBJECT_POOL_SIZE];
    uint8_t static_map[SIZE][SIZE][SIZE];	// Static map.
    uint8_t forces_map[SIZE][SIZE][SIZE];	// Forces map.
    struct object *object_map[SIZE][SIZE][SIZE];	// Object map.

    uint32_t ticks;		// Number of iterations since the level began.
    struct record *record_ptr;	// The current record.

    int8_t pushing;		// If the wodox is attempting to push something.
    int8_t dst_ang;		// Target angle for the wodox.
    int8_t cur_ang;		// Current angle for the wodox.
};

/*----------------------------------------------------------------------------
 * And now... the game itself. 
 *
 * The keyxx and keepxx variables are control for the user input and main
 * loops. They can be reset in the most unsuspecting places, hence their
 * "globality".
 *
 * Apart from the state, the level contains some data that is not altered while
 * playing: the player object is always the same and the circuits that
 * determine the status of static and dynamic machines can not be modified
 * inside a level.
 *
 * The state stack allows quick saving and loading of a number of previous
 * states. It is a circular stack and all operations are performed on the top
 * element, meaning that saving more than the stack size overwrites old save
 * states.
 * 
 * TODO: Up to ten thousand moves may be recorded per level. Until now, this
 * only gave problems with specially dense people playing the 40th. 
 *----------------------------------------------------------------------------*/
struct game {
    int keyup;			// Key for movng up.
    int keydn;			// Key for movng down.
    int keylf;			// Key for movng left.
    int keyrt;			// Key for movng right.

    int keep_going;		// While true, reload level.
    int keep_playing;		// While true, play level.
    int must_save;		// Important.
    int warped;			// True if the player has warped.
    int replay;			// True if we are automatically replaying the level.

    struct object *po;		// Player controlled object.

    int object_count;		// Number of objects.
    struct static_circuit *static_circuits;	// Static circuits.
    struct dynamic_circuit *dynamic_circuits;	// Dynamic circuits.

    struct state cs;		// Current state.

    struct state state_stack[STATE_STACK_LEN];	// Saved states.
    struct state *state_stack_top;	// In a circular stack.
    struct state *state_stack_bottom;	// That may not be full.

    struct record record_list[RECORD_MOVE_LIST_SIZE];	// What the player has been doing.
};

/*
 * There can be only one!
 */
extern struct game game;

/*
 * Aliases for quick iteration on all elements and low level manipulation.
 */
#define MAP ((uint8_t *)game.cs.static_map)
#define FRC ((uint8_t *)game.cs.forces_map)
#define OBJ ((struct object **)game.cs.object_map)

/*
 * To record a key pressing.
 */
#define RECORD_MOVE(k) \
do \
  { \
    if (game.cs.record_ptr - game.record_list < RECORD_MOVE_LIST_SIZE) \
      { \
        game.cs.record_ptr->time = game.cs.ticks; \
        game.cs.record_ptr->key = k; \
        ++game.cs.record_ptr; \
      } \
    game.replay = 0; \
  } \
while (0)

/*
 * Each sound plays through a channel.
 */
#define CHANNEL_WODOX	1
#define CHANNEL_OPEN	2
#define CHANNEL_PRESS	3
#define CHANNEL_RELEASE	4

extern SDL_Surface *surface_levelname;

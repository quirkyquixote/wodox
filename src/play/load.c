/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013 
 */

#include "types.h"
#include "object.h"
#include "circuit.h"

/*
 * Load a level.
 */
int
load_level(const char *path)
{
    FILE *f;
    uint16_t i;
    int c;
    struct static_circuit *s;
    struct dynamic_circuit *d;
    list_node *it;

    if ((f = fopen(path, "r")) == NULL) {
	fprintf(stderr, "couldn't open %s; %s\n", path, strerror(errno));
	return 0;
    }
    // Clean up the current state.

    memset(game.cs.static_map, GROUND, sizeof(Uint8) * SIZE_3);
    memset(game.cs.forces_map, DIR_DN, sizeof(Uint8) * SIZE_3);
    memset(game.cs.object_map, 0, sizeof(struct object *) * SIZE_3);
    memset(game.cs.object_pool, 0,
	   sizeof(struct object) * OBJECT_POOL_SIZE);

    list_clear(&game.cs.objects);
    list_clear(&game.cs.unused_objects);

    for (i = 0; i < OBJECT_POOL_SIZE; ++i) {
	list_push_back(&game.cs.unused_objects,
		       (list_node *) (game.cs.object_pool + i));
    }

    list_clear(&game.static_circuits);
    list_clear(&game.dynamic_circuits);

    game.po = NULL;

    // Read all objects and set maps according to the piece in each space.

    for (i = 0; i < SIZE_3; ++i)
	switch ((c = fgetc(f))) {
	case 0:
	    MAP[i] = EMPTY;
	    break;
	case 1:
	    MAP[i] = GROUND;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 2:
	    object_insert(object_grab(CRATE, i));
	    break;
	case 3:
	    object_insert((game.po = object_grab(PLAYER, i)));
	    break;
	case 4:
	    object_insert(object_grab(SMALL, i));
	    break;
	case 5:
	    MAP[i] = BELT_LF_1;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 6:
	    MAP[i] = BELT_BK_1;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 7:
	    MAP[i] = BELT_RT_1;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 8:
	    MAP[i] = BELT_FT_1;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 9:
	    object_insert(object_grab(MOVING_0, i));
	    break;
	case 10:
	    MAP[i] = BUTTON_0;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 11:
	    MAP[i] = SWITCH_0;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 12:
	    MAP[i] = SWITCH_1;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 17:
	    MAP[i] = EMPTY;
	    FRC[i] = DIR_UP;
	    FRC[i + OFF_UP] = STILL;
	    break;
	case 18:
	    MAP[i] = EMPTY;
	    FRC[i] = WARP;
	    break;

	case EOF:
	    fprintf(stderr, "unexpected end of file in %s\n", path);
	    fclose(f);
	    return 0;

	default:
	    fprintf(stderr, "unexpected object: %x at %u in %s\n", c, i,
		    path);
	    fclose(f);
	    return 0;
	}

    // A wodoxless level can still be executed, but we send a warning.

    if (game.po == NULL) {
	fprintf(stderr, "there is no wodox in %s\n", path);
    }
    // Read circuits. They will be static or dynamic depending on what kind of
    // piece has been previously loaded in the space they are pointing to.

    while (fread(&i, sizeof(uint16_t), 1, f) == 1 && i < SIZE_3) {
	if (OBJ[i]) {
	    d = (struct dynamic_circuit *)
		malloc(sizeof(struct dynamic_circuit));
	    d->obj = OBJ[i];
	    fread(&d->size, sizeof(uint16_t), 1, f);
	    d->tree = (uint16_t *) malloc(sizeof(uint16_t) * d->size);
	    fread(d->tree, sizeof(uint16_t), d->size, f);
	    list_push_back(&game.dynamic_circuits, &(d->node));
	} else {
	    s = (struct static_circuit *)
		malloc(sizeof(struct static_circuit));
	    s->off = i;
	    fread(&s->size, sizeof(uint16_t), 1, f);
	    s->tree = (uint16_t *) malloc(sizeof(uint16_t) * s->size);
	    fread(s->tree, sizeof(uint16_t), s->size, f);
	    list_push_back(&game.static_circuits, &(s->node));
	}
    }

    fclose(f);

    // Buttons under objects are pressed.

    for (it = game.cs.objects.begin; it != game.cs.objects.end;
	 it = it->next) {
	if ((struct object *) it != game.po) {
	    press_buttons((struct object *) it);
	}
    }

    return 1;
}

/*
 * Setdown level.
 */
void
free_level()
{
    struct static_circuit *s;
    struct dynamic_circuit *d;

    while (!list_empty(&game.static_circuits)) {
	s = (struct static_circuit *) game.static_circuits.begin;
	list_pop_front(&game.static_circuits);
	free(s->tree);
	free(s);
    }

    while (!list_empty(&game.dynamic_circuits)) {
	d = (struct dynamic_circuit *) game.dynamic_circuits.begin;
	list_pop_front(&game.dynamic_circuits);
	free(d->tree);
	free(d);
    }
}

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

/* core simulation state */
typedef struct wayterra_game_state {
    float player_x;
    float player_y;

    float velocity_x;
    float velocity_y;

    bool on_ground;

    float gravity;
    float move_speed;
    float jump_velocity;
} wayterra_game_state_t;

/* lifecycle */
void game_init(wayterra_game_state_t *g);
void game_shutdown(wayterra_game_state_t *g);

/* update loop */
void game_update(
    wayterra_game_state_t *g,
    float delta_time
);

/* input interface (decoupled from renderer) */
void game_move_left(wayterra_game_state_t *g);
void game_move_right(wayterra_game_state_t *g);
void game_jump(wayterra_game_state_t *g);
void game_stop_horizontal(wayterra_game_state_t *g);

/* physics */
void game_apply_gravity(
    wayterra_game_state_t *g,
    float delta_time
);

void game_apply_velocity(
    wayterra_game_state_t *g,
    float delta_time
);

/* collision (stubs for future tile system) */
bool game_check_collision(float x, float y);
void game_resolve_collision(wayterra_game_state_t *g);

/* input abstraction */
enum wayterra_input_action {
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_JUMP
};

void game_handle_input(
    wayterra_game_state_t *g,
    enum wayterra_input_action action
);

#endif

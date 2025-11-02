#include <platf/agnostic.h>
#include <render/agnostic.h>
#include <render/draw.h>
#include <core/macros.h>
#include <core/input.h>
#include <core/time.h>
#include <core/renderTarget.h>


f32 sign(f32 a) {
    if (a >= 0)
        return 1;
    if (a <= 0)
        return -1;
    return 0;
}


FUR_platfState* platf = NULL;
FUR_renderState* render = NULL;
FUR_timer* timer = NULL;
FUR_renderTarget* targ = NULL;

typedef struct {
    v2 spos;
} Tile;

Tile* map;
u32 mapW, mapH;

struct {
    v2 pos;
    v2 vel;

    f32 deccel, accel;
    f32 nudge;
} player;

struct textures {
    FUR_texture* player;
};
struct textures tex;

void move_player(void) {
    f32 dt = timer->delta * 4;
    if (dt < 0) dt = 0;
    if (dt >= 1/24.f) dt = 1/24.f;

    f32 netHoriz = 0;

    if (fur_input_isKeyHeld(FUR_KEY_D))
        netHoriz += 1;
    if (fur_input_isKeyHeld(FUR_KEY_A))
        netHoriz -= 1;

    if (netHoriz == 0)
        player.vel.x += -sign(player.vel.x) * player.deccel * dt;
    else {
        player.vel.x += netHoriz * player.accel * dt;

        if (fur_input_isKeyPressed(FUR_KEY_LSHIFT))
            player.vel.x = netHoriz * (fabs(player.vel.x) + player.nudge);
    }

    player.pos.x += player.vel.x * dt;
}

void init(void) {
    player.pos = (v2){0,0};
    player.vel = (v2){0,0};
    
    player.deccel = 8;
    player.accel = 8;
    player.nudge = 16;

    tex.player = fur_texture_load("data/sprites/player.png",);
}

void end(void) {
    fur_texture_unload(tex.player,);
}

void update(void) {
    move_player();
}

void draw(void) {

render->defTarget = targ;

    fur_render_clear(render, .col = (v3){.2,.4,.3});

    fur_render_tex(render, .texture = tex.player, .pos = player.pos, .size = (v2){24,24}, .sample = (v4){0,0,24,24});

render->defTarget = NULL;

    fur_render_renderTarget(render, .in_target = targ);
}

int main(void) {
    platf = fur_platf_constr();
    render = fur_render_constr();
    fur_platf_setRender(platf, render);

    timer = fur_makeTimer();
    targ = fur_renderTarget_constr(320,180,);

    init();

    while (!fur_platf_shouldWindowClose(platf)) {
        fur_platf_poll(platf);
        fur_input_poll(platf);
        fur_updateTimers(&timer, 1);

        update();
        draw();

        fur_render_flush(render);
        fur_platf_present(platf);
    }   

    end();

    fur_renderTarget_destr(targ);
    fur_destroyTimer(timer);

    fur_render_destr(render);
    fur_platf_destr(platf);
}


#include <platf/agnostic.h>
#include <render/agnostic.h>
#include <render/draw.h>
#include <core/macros.h>
#include <core/input.h>
#include <core/time.h>
#include <core/renderTarget.h>
#include <stdio.h>


f32 sign(f32 a) {
    if (a >= 0)
        return 1;
    if (a <= 0)
        return -1;
    return 0;
}

f32 max(f32 a, f32 b) {
    if (a <= b)
        return b;
    return a;
}


struct {
    f32 animtick_250, animtick_100, animtick_player_speed;
} state;

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

    s8 dir;

    enum {
        PLAYER_ANIMSTATE_IDLE,
        PLAYER_ANIMSTATE_RUN,
        PLAYER_ANIMSTATE_AIR
    } animstate;
} player;

struct {
    FUR_texture* player;

    struct {
        FUR_texture* lab;
    } tiles;
} tex;

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

        player.dir = netHoriz;

        if (fur_input_isKeyPressed(FUR_KEY_LSHIFT))
            player.vel.x = netHoriz * (fabs(player.vel.x) + player.nudge);
    }

    player.pos.x += player.vel.x * dt;

    if (fabs(player.vel.x) <= 1)
        player.animstate = PLAYER_ANIMSTATE_IDLE;
    else
        player.animstate = PLAYER_ANIMSTATE_RUN;
}

void init(void) {
    player.pos = (v2){0,0};
    player.vel = (v2){0,0};
    
    player.deccel = 8;
    player.accel = 8;
    player.nudge = 16;

    player.dir = 1;

    tex.player = fur_texture_load("data/sprites/player.png",);
    tex.tiles.lab = fur_texture_load("data/sprites/lab tiles.png",);

    state.animtick_250 = 0;
    state.animtick_100 = 0;
    state.animtick_player_speed = 0;
}

void end(void) {
    fur_texture_unload(tex.player,);
}


void update(void) {
    move_player();
}

void draw(void) {

    state.animtick_250 += timer->delta * 4;
    u8 anim_250 = state.animtick_250 - floor(state.animtick_250/2)*2;

    state.animtick_100 += timer->delta * 10;
    u8 anim_100 = state.animtick_100 - floor(state.animtick_100/2)*2;

    if (player.animstate == PLAYER_ANIMSTATE_RUN)
        state.animtick_player_speed += timer->delta * max(fabs(player.vel.x), 1.f);
    u8 anim_ps = state.animtick_player_speed - floor(state.animtick_player_speed/4)*4;

render->defTarget = targ;

    fur_render_clear(render, .col = (v3){.2,.4,.3});

    for(s32 x = 0; x < 320/16 +1; ++x)
        for (s32 y = 0; y < 180/16 +1; ++y)
            fur_render_tex(render, .texture = tex.tiles.lab, .pos = (v2){x*16,y*16}, .size = (v2){16,16}, .sample = (v4){anim_250*64,48,16,16});

    u8 player_anim_sample = 0;
    switch (player.animstate) {
        case PLAYER_ANIMSTATE_IDLE: player_anim_sample = anim_250 + 0; break;
        case PLAYER_ANIMSTATE_AIR:  player_anim_sample = anim_100 + 6; break;
        case PLAYER_ANIMSTATE_RUN:  player_anim_sample = anim_ps  + 2; break;
    }

    v4 player_sample = (v4){(player_anim_sample%3) *24, (f32)floor((f32)player_anim_sample/3.f) *24, 24 * player.dir,24};
    if (player.dir < 0)
        player_sample.x += 24;

    fur_render_tex(render, .texture = tex.player, .pos = player.pos, .size = (v2){24,24}, .sample = player_sample);

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


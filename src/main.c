#include <platf/agnostic.h>
#include <render/agnostic.h>
#include <render/draw.h>
#include <core/macros.h>
#include <core/input.h>
#include <core/time.h>
#include <core/renderTarget.h>

FUR_platfState* platf = NULL;
FUR_renderState* render = NULL;
FUR_timer* timer = NULL;
FUR_renderTarget* targ = NULL;

struct textures {
    FUR_texture* player;
};
struct textures tex;

void init(void) {
    tex.player = fur_texture_load("data/sprites/player.png",);
}

void end(void) {
    fur_texture_unload(tex.player,);
}

void update(void) {
    
}

void draw(void) {

render->defTarget = targ;

    fur_render_clear(render, .col = (v3){.2,.4,.3});

    fur_render_tex(render, .texture = tex.player, .pos = (v2){64,64}, .size = (v2){24,24}, .sample = (v4){0,0,24,24});

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


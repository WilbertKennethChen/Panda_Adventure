#include <allegro5/allegro.h>
#include "setting_scene.h"
#include "menu_scene.h"
#include "utility.h"
#include "UI.h"
#include "game.h"

static Button backButton;
static Button bgmTrack;
static Button bgmKnob;
static Button bgmVolume;
static Button sfxTrack;
static Button sfxKnob; 
static Button sfxVolume;
static float bgmValue = 0.5f;
static float sfxValue = 0.5f;

#define KNOB_WIDTH 40  
#define TRACK_WIDTH 400 

static void init(void) {
    backButton = button_create(SCREEN_W / 2 - 200, 650, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    bgmTrack = button_create(SCREEN_W / 2 - 200, 300, TRACK_WIDTH, 20, "Assets/slider.png", "Assets/slider_hovered.png");
    bgmKnob = button_create(SCREEN_W / 2 - 200 + bgmValue * (TRACK_WIDTH - KNOB_WIDTH), 290, KNOB_WIDTH, 40, "Assets/slider.png", "Assets/slider_hovered.png");
    bgmVolume = button_create(
        bgmTrack.x,  
        bgmTrack.y,
        bgmKnob.x - bgmTrack.x, bgmTrack.h,
        "Assets/slider_hovered.png", "Assets/slider_hovered.png");
    sfxTrack = button_create(SCREEN_W / 2 - 200, 400, TRACK_WIDTH, 20, "Assets/slider.png", "Assets/slider.png");
    sfxKnob = button_create(SCREEN_W / 2 - 200 + sfxValue * (TRACK_WIDTH - KNOB_WIDTH), 390, KNOB_WIDTH, 40, "Assets/slider.png", "Assets/slider_hovered.png");
    sfxVolume = button_create(
        sfxTrack.x,
        sfxTrack.y,
        sfxKnob.x - sfxTrack.x, sfxTrack.h,
        "Assets/slider_hovered.png", "Assets/slider_hovered.png");
}

static void update(void) {

    update_button(&backButton);

    if (mouseState.buttons && backButton.hovered == true) {
        change_scene(create_menu_scene());
    }

    update_button(&bgmKnob);
    update_button(&sfxKnob);

    al_get_mouse_state(&mouseState);

    static bool isDraggingBGM = false;
    static bool isDraggingSFX = false;

    if (mouseState.buttons & 1) {
        if (bgmKnob.hovered && !isDraggingSFX) {
            isDraggingBGM = true;
        }

        if (isDraggingBGM) {
            float mouseX = mouseState.x;
            float knobX = mouseX - KNOB_WIDTH / 2;

            if (knobX < bgmTrack.x) {
                knobX = bgmTrack.x;
            }   
            else if (knobX > bgmTrack.x + TRACK_WIDTH - KNOB_WIDTH) {
                knobX = bgmTrack.x + TRACK_WIDTH - KNOB_WIDTH;
            }   

            bgmKnob.x = knobX; 
            bgmValue = (knobX - bgmTrack.x) / (TRACK_WIDTH - KNOB_WIDTH); 
            BGM_VOLUME = bgmValue; 
            bgmVolume.w = knobX - bgmTrack.x;
        }
    }
    else {
        isDraggingBGM = false;
    }

    if (mouseState.buttons & 1) {
        if (sfxKnob.hovered && !isDraggingBGM) {
            isDraggingSFX = true;
        }

        if (isDraggingSFX) {
            float mouseX = mouseState.x;
            float knobX = mouseX - KNOB_WIDTH / 2;

            if (knobX < sfxTrack.x) {
                knobX = sfxTrack.x;
            }
            else if (knobX > sfxTrack.x + TRACK_WIDTH - KNOB_WIDTH) {
                knobX = sfxTrack.x + TRACK_WIDTH - KNOB_WIDTH;
            }

            sfxKnob.x = knobX;
            sfxValue = (knobX - sfxTrack.x) / (TRACK_WIDTH - KNOB_WIDTH); 
            SFX_VOLUME = sfxValue; 
            sfxVolume.w = knobX - sfxTrack.x;
        }
    }
    else {
        isDraggingSFX = false;
    }
}

static void draw(void) {
 
    draw_button(backButton);

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        650 + 28 + backButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "BACK"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        650 + 31 + backButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "BACK"
    );

    draw_button(bgmTrack);
    draw_button(bgmKnob);
    draw_button(bgmVolume); 
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2, 250, ALLEGRO_ALIGN_CENTER, "BGM Volume");

    draw_button(sfxTrack); 
    draw_button(sfxKnob);  
    draw_button(sfxVolume); 
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2, 350, ALLEGRO_ALIGN_CENTER, "SFX Volume");
}

static void destroy(void) {
    destroy_button(&backButton);   
    destroy_button(&bgmTrack);     
    destroy_button(&bgmKnob);      
    destroy_button(&bgmVolume);   
    destroy_button(&sfxTrack);     
    destroy_button(&sfxKnob);      
    destroy_button(&sfxVolume);    
}

Scene create_setting_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "setting";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;

    return scene;
}

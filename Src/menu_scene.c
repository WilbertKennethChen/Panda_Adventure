#include <allegro5/allegro.h>
#include <string.h>
#include "menu_scene.h"
#include "game_scene.h"
#include "setting_scene.h"
#include "loading_scene.h"
#include "utility.h"
#include "UI.h"
#include "game.h"

int selectedWeapon;
static Button settingButton;
static Button akmButton;
static Button sniperButton;
static Button startButton;
static Button leaderBoard;
static Button backButton;
int coins_obtained;
ALLEGRO_BITMAP* akmImg;
ALLEGRO_BITMAP* sniperImg;
ALLEGRO_BITMAP* coinImg;
ALLEGRO_BITMAP* trophyImg;
bool isEquipped = false;
bool akmPurchased = true;
bool sniperPurchased = false;

static int currpage = 0;
static const int ldbpage = 10;
static Button nextPageButton;
static Button prevPageButton;

typedef struct {
    char name[50];
    float time;
} LeaderboardEntry;

#define MAX_ENTRIES 100
static LeaderboardEntry leaderboard[MAX_ENTRIES];
static int leaderboardSize = 0;

static void load_leaderboard(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        game_log("Failed to open leaderboard file: %s", filepath);
        return;
    }

    leaderboardSize = 0;
    while (leaderboardSize < MAX_ENTRIES) {
        char plyrname[50];
        float plyrtime;
        if (fscanf_s(file, "%49s %f", plyrname, (unsigned)_countof(plyrname), &plyrtime) == 2) {
            strncpy_s(leaderboard[leaderboardSize].name, sizeof(leaderboard[leaderboardSize].name), plyrname, _TRUNCATE);
            leaderboard[leaderboardSize].time = plyrtime;
            leaderboardSize++;
        }
        else {
            break;
        }
    }

    fclose(file);

    for (int i = 0; i < leaderboardSize - 1; i++) {
        for (int j = 0; j < leaderboardSize - i - 1; j++) {
            if (leaderboard[j].time > leaderboard[j + 1].time) {
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}

static void init(void) {
    settingButton = button_create(SCREEN_W / 2 - 200, 600, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    leaderBoard = button_create(SCREEN_W / 2 - 200, 500, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    change_bgm("Assets/audio/menu_bgm.mp3");
}

static void update(void) {
    update_button(&settingButton);
    update_button(&leaderBoard);
    if (keyState[ALLEGRO_KEY_ENTER]) {
        change_scene(create_weapon_scene());
    }

    /*
        [TODO HACKATHON 4-3]

        Change scene to setting scene when the button is pressed
    */
    if (settingButton.hovered && mouseState.buttons == true) {
        change_scene(create_setting_scene());
        al_rest(0.2);
    }
    if (leaderBoard.hovered && mouseState.buttons == true) {
        change_scene(create_ldb_scene());
        al_rest(0.2);
    }
}

static void draw(void) {
    // Title Text
    al_draw_text(
        TITLE_FONT,
        al_map_rgb(146, 161, 185),
        SCREEN_W / 2,
        190,
        ALLEGRO_ALIGN_CENTER,
        "NTHU-RPG ADVENTURE"
    );
    al_draw_text(
        TITLE_FONT,
        al_map_rgb(199, 207, 221),
        SCREEN_W / 2,
        185,
        ALLEGRO_ALIGN_CENTER,
        "NTHU-RPG ADVENTURE"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        400,
        ALLEGRO_ALIGN_CENTER,
        "PRESS [ENTER] TO PLAY"
    );

    // button
    draw_button(settingButton);
    // button text
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        600 + 28 + settingButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "SETTING"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        600 + 31 + settingButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "SETTING"
    );

    draw_button(leaderBoard);
    // button text
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        500 + 28 + leaderBoard.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "LEADERBOARD"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2,
        500 + 31 + leaderBoard.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "LEADERBOARD"
    );
}

static void destroy(void) {
    destroy_button(&settingButton);
}


Scene create_menu_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "menu";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;

    return scene;
}


static void initweapon(void) {
    akmButton = button_create(SCREEN_W / 2 - 310, 450, 280, 80, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    sniperButton = button_create(SCREEN_W / 2 + 40, 450, 280, 80, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    startButton = button_create(SCREEN_W / 2 - 250, 650, 500, 110, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    akmImg = al_load_bitmap("Assets/guns.png");
    akmImg = al_load_bitmap("Assets/guns.png");
    sniperImg = al_load_bitmap("Assets/sniper.png");
    coinImg = al_load_bitmap("Assets/coin_icon.png");

}

static void updateweapon(void) {
    update_button(&akmButton);
    update_button(&sniperButton);
    update_button(&startButton);
    if (akmButton.hovered && mouseState.buttons == true) {
        selectedWeapon = 1;
        isEquipped = true;
    }
    if (sniperButton.hovered && mouseState.buttons == true) {
        if (sniperPurchased || (!sniperPurchased && coins_obtained >= 3)) {
            if (!sniperPurchased) {
                coins_obtained -= 3;
                sniperPurchased = true;
            }
            selectedWeapon = 2;
            isEquipped = true;
        }
    }
    if (startButton.hovered && mouseState.buttons == true) {
        if (selectedWeapon > 0) {
            change_scene(create_loading_scene());
        }
    }
}

static void drawweapon(void) {
    char coinstr[5];
    snprintf(coinstr, sizeof(coinstr), "%02d", coins_obtained);
    // button
    draw_button(akmButton);
    draw_button(sniperButton);
    draw_button(startButton);
    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        180,
        ALLEGRO_ALIGN_CENTER,
        "CHOOSE YOUR WEAPON"
    );

    al_draw_filled_rounded_rectangle(
        SCREEN_W / 2 - 320 + 50,
        250,
        SCREEN_W / 2 - 320 + 260,
        400,
        15, 15,
        al_map_rgb(255, 255, 255)
    );

    al_draw_filled_rounded_rectangle(
        SCREEN_W / 2 + 20 + 50,
        250,
        SCREEN_W / 2 + 20 + 260,
        400,
        15, 15,
        al_map_rgb(255, 255, 255)
    );

    al_draw_scaled_bitmap(
        akmImg,
        0, 0,
        al_get_bitmap_width(akmImg),
        al_get_bitmap_height(akmImg),
        SCREEN_W / 2 - 320 + 80,
        260,
        150, 150,
        0
    );

    al_draw_scaled_bitmap(
        sniperImg,
        0, 0,
        al_get_bitmap_width(sniperImg),
        al_get_bitmap_height(sniperImg),
        SCREEN_W / 2 + 5 + 80,
        260,
        150, 150,
        0
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2 - 170,
        440 + 30 + akmButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        selectedWeapon == 1 && isEquipped ? "EQUIPPED" : "EQUIP"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2 - 170,
        440 + 33 + akmButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        selectedWeapon == 1 && isEquipped ? "EQUIPPED" : "EQUIP"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2 + 180,
        440 + 30 + sniperButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        sniperPurchased ? (selectedWeapon == 2 && isEquipped ? "EQUIPPED" : "EQUIP") : "3 COINS"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(225, 225, 225),
        SCREEN_W / 2 + 180,
        440 + 33 + sniperButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        sniperPurchased ? (selectedWeapon == 2 && isEquipped ? "EQUIPPED" : "EQUIP") : "3 COINS"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        650 + 33 + startButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        selectedWeapon > 0 ? "START" : "SELECT A WEAPON"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        650 + 30 + startButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        selectedWeapon > 0 ? "START" : "SELECT A WEAPON"
    );


    al_draw_scaled_bitmap(coinImg, 0, 0, al_get_bitmap_width(coinImg), al_get_bitmap_height(coinImg),
        0, 10, 70, 70, 0);
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), 20 + 40, 20 + 10, 0, coinstr);
}

static void destroyweapon(void) {
    destroy_button(&akmButton);
    destroy_button(&sniperButton);
}

Scene create_weapon_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "weapon";
    scene.init = &initweapon;
    scene.draw = &drawweapon;
    scene.update = &updateweapon;
    scene.destroy = &destroyweapon;

    return scene;
}

static void initldb(void) {
    backButton = button_create(SCREEN_W / 2 - 200, 650, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    nextPageButton = button_create(575, 580, 100, 60, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    prevPageButton = button_create(100, 580, 100, 60, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    load_leaderboard("Assets/ldb.txt");
    trophyImg = al_load_bitmap("Assets/trophy.png");
    if (!trophyImg) {
        game_log("Failed to load trophy image.");
    }
}

static void drawldb(void) {
    draw_button(backButton);
    draw_button(nextPageButton);
    draw_button(prevPageButton);

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

    al_draw_filled_rectangle(
        100, 200, 700, 600,
        al_map_rgb(30, 30, 30)
    );

    al_draw_rectangle(
        100, 200, 700, 600,
        al_map_rgb(200, 200, 200),
        2.0
    );

    al_draw_filled_rectangle(100, 125, 700, 640, al_map_rgb(30, 30, 30));
    al_draw_rectangle(100, 125, 700, 640, al_map_rgb(200, 200, 200), 2.0);

    int startIndex = currpage * ldbpage;
    int endIndex = startIndex + ldbpage;

    if (endIndex > leaderboardSize) endIndex = leaderboardSize;

    int entryHeight = 40;
    int startY = 140;

    for (int i = startIndex; i < endIndex; i++) {
        char entrydata[100];
        snprintf(entrydata, sizeof(entrydata), "%d. %s - %.2f", i + 1, leaderboard[i].name, leaderboard[i].time);

        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), 120, startY + (i - startIndex) * entryHeight, ALLEGRO_ALIGN_LEFT, entrydata);
    }

    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), 640, 580 + (nextPageButton.hovered ? 2 : 0), ALLEGRO_ALIGN_CENTER, "NEXT");
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), 160, 580 + (prevPageButton.hovered ? 2 : 0), ALLEGRO_ALIGN_CENTER, "PREV");

    al_draw_text(
        TITLE_FONT,
        al_map_rgb(199, 207, 221),
        SCREEN_W / 2 - 50,
        50,
        ALLEGRO_ALIGN_CENTER,
        "LEADERBOARD"
    );

    al_draw_scaled_bitmap(
        trophyImg,
        0, 0,
        32, 32,
        SCREEN_W / 2 + 215, 35,
        85, 85,
        0
    );
}

static void updateldb(void) {
    update_button(&backButton);
    update_button(&nextPageButton);
    update_button(&prevPageButton);

    if (mouseState.buttons && backButton.hovered == true) {
        change_scene(create_menu_scene());
        al_rest(0.2);
    }

    if (mouseState.buttons && nextPageButton.hovered == true) {
        if ((currpage + 1) * ldbpage < leaderboardSize) {
            currpage++;
            al_rest(0.2);
        }
    }

    if (mouseState.buttons && prevPageButton.hovered == true) {
        if (currpage > 0) {
            currpage--;
            al_rest(0.2);
        }
    }
}

static void destroyldb(void) {
    destroy_button(&backButton);
    destroy_button(&nextPageButton);
    destroy_button(&prevPageButton);
}

Scene create_ldb_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "ldb";
    scene.init = &initldb;
    scene.draw = &drawldb;
    scene.update = &updateldb;
    scene.destroy = &destroyldb;

    return scene;
}
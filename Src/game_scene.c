#include <string.h>
#include "game_scene.h"
#include "menu_scene.h"
#include "loading_scene.h"
#include "game.h"
#include "player.h"
#include "map.h"
#include "enemy.h"
#include "weapon.h"
#include "UI.h"
#include <math.h>

Player player;
Map map;
Point trophy;
enemyNode* enemyList;
BulletNode* bulletList;
ALLEGRO_BITMAP* heartIcon;
ALLEGRO_BITMAP* coinIcon;

ALLEGRO_BITMAP* pandaLoseImg;
ALLEGRO_BITMAP* pandaWinImg;
ALLEGRO_SAMPLE_ID trophy_audio_id;
ALLEGRO_SAMPLE* die;

Weapon weapon;

int coins_obtained;
Button continueButton;
Button restartButton;
Button presumeButton;
Button pmenuButton;
Button pcontinueButton;

static int timer_countdown;
static ALLEGRO_BITMAP* player_bitmap;
int current_level = 1;

bool paused = false;
static int pressed = 0;

static float total_timer = 0.0f;
static float start_timer = 0.0f;
static float save_timer = 0.0f;
static float last_update = 0.0f;
static float ldb_timer = 0.0f;
static char playerName[32] = "Player";
static char winnername[32] = "";
static bool namentered = false;

typedef enum {
    GAME_LOSING,
    GAME_PLAYING,
    GAME_WIN,
} GameState;

GameState gameState = GAME_PLAYING;

Scene create_lose_scene(void);
Scene create_win_scene(void);


static void init(void) {

    if (current_level == 1) {
        total_timer = 0.0f;
    }
    else {
        total_timer = save_timer;
    }
    start_timer = al_get_time();

    initEnemy();

    char mapPath[32];
    snprintf(mapPath, sizeof(mapPath), "Assets/map%d.txt", current_level - 1);
    map = create_map(mapPath, 0);

    player = create_player("Assets/panda2.png", map.Spawn.x, map.Spawn.y);

    coinIcon = al_load_bitmap("Assets/coin_icon.png");
    heartIcon = al_load_bitmap("Assets/heart.png");
    die = al_load_sample("Assets/audio/dies.mp3");
    if (!coinIcon || !heartIcon) {
        game_abort("Failed to load heart/coin bitmap");
    }

    enemyList = createEnemyList();
    bulletList = createBulletList();
    if (selectedWeapon == 1) weapon = create_weapon("Assets/guns.png", "Assets/yellow_bullet.png", 10, 20, 15);
    else if (selectedWeapon == 2) weapon = create_weapon("Assets/sniper.png", "Assets/orange_bullet.png", 100, 40, 100);

    for (int i = 0; i < map.EnemySpawnSize; i++) {
        Enemy enemy = createEnemy(map.EnemySpawn[i].x, map.EnemySpawn[i].y, map.EnemyCode[i]);
        insertEnemyList(enemyList, enemy);
    }

    game_log("coord x:%d \n coords y:%d \n", map.Spawn.x, map.Spawn.y);
    change_bgm("Assets/audio/game_bgm.mp3");

    presumeButton = button_create(SCREEN_W / 2 - 200, SCREEN_H - 450, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    pcontinueButton = button_create(SCREEN_W / 2 - 200, SCREEN_H - 345, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    pmenuButton = button_create(SCREEN_W / 2 - 200, SCREEN_H - 240, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
}


static void update(void) {
    float curr_timer = al_get_time();

    if (last_update == 0.0f) {
        last_update = curr_timer;
        return;
    }

    float timer_change = curr_timer - last_update;
    last_update = curr_timer;

    if (!paused) {
        total_timer += timer_change;
    }

    if (keyState[ALLEGRO_KEY_ESCAPE] && !pressed) {
        paused = !paused;
    }

    pressed = keyState[ALLEGRO_KEY_ESCAPE];

    if (paused) {
        update_button(&presumeButton);
        update_button(&pcontinueButton);
        update_button(&pmenuButton);

        if (mouseState.buttons && presumeButton.hovered) {
            paused = 0;
        }
        if (mouseState.buttons && pcontinueButton.hovered) {
            paused = 0;
            if (current_level == 1) {
                coins_obtained = 0;
                total_timer = 0.0f;
            }
            else {
                total_timer = save_timer;
            }
            change_scene(create_game_scene());
        }
        if (mouseState.buttons && pmenuButton.hovered) {
            paused = 0;
            coins_obtained = 0;
            current_level = 1;
            total_timer = 0.0f;
            save_timer = 0.0f;
            last_update = 0.0f;
            change_scene(create_menu_scene());
        }
        return;
    }

    /*
        [TODO Homework]

        Change the scene if winning/losing to win/lose scene
    */

    if (player.health <= 0 && player.status == PLAYER_DYING && player.animation_tick == 64 - 1) {
        coins_obtained = 0;
        gameState = GAME_LOSING;
        change_scene(create_lose_scene());
        return;
    }

    if (map.move) {
        save_timer = total_timer;
        last_update = 0.0f;
        current_level += 1;
        change_scene(create_weapon_scene());
        return;
    }

    if (map.win) {
        last_update = 0.0f;
        gameState = GAME_WIN;
        change_scene(create_win_scene());
        return;
    }

    update_player(&player, &map);
    Point Camera;
    /*
        [TODO HACKATHON 1-3]

        Calcualte the formula for the Camera
        Camera.x = ...
        Camera.y = ...

        Hint: Adjust it based on player position variable, then subtract it with half of the gameplay screen
    */
    Camera = (Point){ 0, 0 };
    Camera.x = player.coord.x - (SCREEN_W / 2);
    Camera.y = player.coord.y - (SCREEN_H / 2);

    updateEnemyList(enemyList, &map, &player);
    update_map(&map, player.coord, &coins_obtained);
    if (player.health != 0) {
        update_weapon(&weapon, bulletList, player.coord, Camera);
        updateBulletList(bulletList, enemyList, &map);
    }

}

static void draw(void) {
    Point Camera;
    /*
        [TODO HACKATHON 1-4]

        Calcualte the formula for the Camera
        Camera.x = ...
        Camera.y = ...

        Hint: Just copy from the [TODO HACKATHON 1-3]
    */
    Camera = (Point){ 0, 0 };
    Camera.x = player.coord.x - (SCREEN_W / 2);
    Camera.y = player.coord.y - (SCREEN_H / 2);

    // Draw
    draw_map(&map, Camera);
    drawEnemyList(enemyList, Camera);
    draw_player(&player, Camera);
    if (player.health != 0) {
        draw_weapon(&weapon, player.coord, Camera);
        drawBulletList(bulletList, Camera);
    }

    /*
        [TODO Homework]

        Draw the UI of Health and Total Coins
    */
    int healthX = 20;
    int healthY = 20;
    int coinsX = 20;
    int coinsY = 60;
    int iconWidth = 40;
    int iconHeight = 40;
    char healthstr[5];
    snprintf(healthstr, sizeof(healthstr), "%d", player.health);

    char coinstr[5];
    snprintf(coinstr, sizeof(coinstr), "%02d", coins_obtained);

    char timerstr[16];
    snprintf(timerstr, sizeof(timerstr), "%.2f", total_timer);

    al_draw_scaled_bitmap(heartIcon, 0, 0, al_get_bitmap_width(heartIcon), al_get_bitmap_height(heartIcon),
        healthX, healthY, iconWidth, iconHeight, 0);
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), healthX + 40, healthY + 10, 0, healthstr);

    al_draw_scaled_bitmap(coinIcon, 0, 0, al_get_bitmap_width(coinIcon), al_get_bitmap_height(coinIcon),
        coinsX, coinsY, iconWidth, iconHeight, 0);
    al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), coinsX + 40, coinsY + 10, 0, coinstr);

    al_draw_textf(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W - 20, 20, ALLEGRO_ALIGN_RIGHT, "Timer: %.2f", ((int)(total_timer / 60)) + (((int)total_timer % 60) / 100.0f));


    if (paused) {
        al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H, al_map_rgba(0, 0, 0, 128));
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2, 225, ALLEGRO_ALIGN_CENTER, "Paused");
        draw_button(presumeButton);
        draw_button(pcontinueButton);
        draw_button(pmenuButton);

        al_draw_text(
            P2_FONT,
            al_map_rgb(66, 76, 110),
            SCREEN_W / 2,
            SCREEN_H - 467 + (100 / 2) + presumeButton.hovered * 11,
            ALLEGRO_ALIGN_CENTER,
            "Resume"
        );
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2,
            SCREEN_H - 464 + (100 / 2) + presumeButton.hovered * 11, ALLEGRO_ALIGN_CENTER, "Resume");

        al_draw_text(
            P2_FONT,
            al_map_rgb(66, 76, 110),
            SCREEN_W / 2,
            SCREEN_H - 360 + (100 / 2) + pcontinueButton.hovered * 11,
            ALLEGRO_ALIGN_CENTER,
            "Retry"
        );
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2,
            SCREEN_H - 357 + (100 / 2) + pcontinueButton.hovered * 11, ALLEGRO_ALIGN_CENTER, "Retry");

        al_draw_text(
            P2_FONT,
            al_map_rgb(66, 76, 110),
            SCREEN_W / 2,
            SCREEN_H - 253 + (100 / 2) + pmenuButton.hovered * 11,
            ALLEGRO_ALIGN_CENTER,
            "Main Menu"
        );
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2,
            SCREEN_H - 250 + (100 / 2) + pmenuButton.hovered * 11, ALLEGRO_ALIGN_CENTER, "Main Menu");
    }
}

static void destroy(void) {
    delete_player(&player);
    delete_weapon(&weapon);
    destroy_map(&map);
    destroyBulletList(bulletList);
    destroyEnemyList(enemyList);
    terminateEnemy();
}

Scene create_game_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "game";
    scene.init = &init;
    scene.draw = &draw;
    scene.update = &update;
    scene.destroy = &destroy;

    return scene;
}


static void init_lose_scene(void) {
    change_bgm("Assets/audio/dies.mp3");
    pandaLoseImg = al_load_bitmap("Assets/panda_lose.png");
    if (!pandaLoseImg) {
        game_log("Failed to load panda lose image: Assets/panda_lose.png");
        game_abort("Failed to load panda lose image!");
    }
    continueButton = button_create(SCREEN_W / 2 - 200, SCREEN_H - 240, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
    restartButton = button_create(SCREEN_W / 2 - 200, SCREEN_H - 140, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
}

static void draw_lose_scene(void) {

    al_clear_to_color(al_map_rgb(0, 0, 0));
    const char* missionFailedText = "SKILL ISSUE??";
    al_draw_text(P2_FONT, al_map_rgb(255, 0, 0), SCREEN_W / 2, 100, ALLEGRO_ALIGN_CENTER, missionFailedText);

    int img_x = (SCREEN_W - al_get_bitmap_width(pandaLoseImg)) / 2;
    int img_y = (SCREEN_H - al_get_bitmap_height(pandaLoseImg)) / 2 - 100;
    al_draw_bitmap(pandaLoseImg, img_x, img_y, 0);
    al_draw_scaled_bitmap(pandaLoseImg,
        0, 0, 64, 64,
        SCREEN_W / 4, SCREEN_H / 4 - 60, SCREEN_W / 2, SCREEN_H / 2,
        0);


    draw_button(continueButton);
    draw_button(restartButton);
    int buttonWidth = 200;
    int buttonHeight = 70;

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        SCREEN_H - 220 + continueButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Main menu"
    );

    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        SCREEN_H - 217 + continueButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Main menu"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        SCREEN_H - 117 + restartButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Retry"
    );
    al_draw_text(
        P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        SCREEN_H - 113 + restartButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Retry"
    );
}

static void update_lose_scene(void) {
    draw_lose_scene();

    update_button(&continueButton);
    update_button(&restartButton);

    if (mouseState.buttons && continueButton.hovered) {
        current_level = 1;
        total_timer = 0.0f;
        save_timer = 0.0f;
        last_update = 0.0f;
        change_scene(create_menu_scene());
    }
    if (mouseState.buttons && restartButton.hovered) {
        if (current_level == 1) {
            total_timer = 0.0f;
            last_update = 0.0f;
            save_timer = 0.0f;
        }
        else {
            total_timer = save_timer;
            last_update = 0.0f;
        }
        change_scene(create_loading_scene());
    }
}

static void destroy_lose_scene(void) {
    al_destroy_bitmap(pandaLoseImg);
}

Scene create_lose_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "lose";
    scene.init = &init_lose_scene;
    scene.update = &update_lose_scene;
    scene.draw = &draw_lose_scene;
    scene.destroy = &destroy_lose_scene;

    return scene;
}

static void init_win_scene(void) {
    change_bgm("Assets/audio/win2.mp3");
    pandaWinImg = al_load_bitmap("Assets/panda_win.png");
    if (!pandaWinImg) {
        game_log("Failed to load panda lose image: Assets/panda_lose.png");
        game_abort("Failed to load panda lose image!");
    }
    continueButton = button_create(SCREEN_W / 2 - 200, 650, 400, 100, "Assets/UI_Button.png", "Assets/UI_Button_hovered.png");
}

static void draw_win_scene(void) {

    al_clear_to_color(al_map_rgb(0, 0, 0));


    const char* missionFailedText = "GG Gaming!!";
    al_draw_text(P2_FONT, al_map_rgb(0, 255, 0), SCREEN_W / 2, 100, ALLEGRO_ALIGN_CENTER, missionFailedText);


    int img_x = (SCREEN_W - al_get_bitmap_width(pandaWinImg)) / 2;
    int img_y = (SCREEN_H - al_get_bitmap_height(pandaWinImg)) / 2 - 100;
    al_draw_bitmap(pandaWinImg, img_x, img_y, 0);

    al_draw_scaled_bitmap(pandaWinImg,
        0, 0, 64, 64,
        SCREEN_W / 4, SCREEN_H / 4 - 60, SCREEN_W / 2, SCREEN_H / 2,
        0);

    if (namentered) {
        char displayMessage[64];
        snprintf(displayMessage, sizeof(displayMessage), "Congrats: %s", playerName);
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 0), SCREEN_W / 2, 550, ALLEGRO_ALIGN_CENTER, displayMessage);
    }
    else {
        al_draw_text(P2_FONT, al_map_rgb(200, 200, 200), SCREEN_W / 2, 550, ALLEGRO_ALIGN_CENTER, "Enter your name:");
        al_draw_text(P2_FONT, al_map_rgb(255, 255, 255), SCREEN_W / 2, 600, ALLEGRO_ALIGN_CENTER, winnername);
    }

    draw_button(continueButton);
    int buttonWidth = 200;
    int buttonHeight = 70;

    al_draw_text(
        P2_FONT,
        al_map_rgb(66, 76, 110),
        SCREEN_W / 2,
        650 + 28 + continueButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Main Menu"
    );

    al_draw_text(P2_FONT,
        al_map_rgb(255, 255, 255),
        SCREEN_W / 2,
        650 + 31 + continueButton.hovered * 11,
        ALLEGRO_ALIGN_CENTER,
        "Main Menu"
    );

}
void save_score_to_file(const char* filename, const char* playerName, float total_timer) {
    FILE* file = fopen(filename, "a");
    if (!file) {
        game_log("Failed to open file: %s", filename);
        return;
    }
    fprintf(file, "%s %.2f\n", playerName, total_timer);
    fclose(file);
    game_log("Score saved to file: %s", filename);
}

static void update_win_scene(void) {
    draw_win_scene();

    update_button(&continueButton);

    if (!namentered) {
        for (int i = ALLEGRO_KEY_A; i <= ALLEGRO_KEY_Z; i++) {
            if (keyState[i]) {
                size_t len = strlen(winnername);
                if (len < sizeof(winnername) - 1) {
                    winnername[len] = 'A' + (i - ALLEGRO_KEY_A);
                    winnername[len + 1] = '\0';
                }
                keyState[i] = false;
            }
        }

        for (int i = ALLEGRO_KEY_0; i <= ALLEGRO_KEY_9; i++) {
            if (keyState[i]) {
                size_t len = strlen(winnername);
                if (len < sizeof(winnername) - 1) {
                    winnername[len] = '0' + (i - ALLEGRO_KEY_0);
                    winnername[len + 1] = '\0';
                }
                keyState[i] = false;
            }
        }

        if (keyState[ALLEGRO_KEY_BACKSPACE] && strlen(winnername) > 0) {
            winnername[strlen(winnername) - 1] = '\0';
            keyState[ALLEGRO_KEY_BACKSPACE] = false;
        }

        if (keyState[ALLEGRO_KEY_ENTER]) {
            namentered = true;
            strncpy_s(playerName, sizeof(playerName), winnername, _TRUNCATE);
            playerName[sizeof(playerName) - 1] = '\0';
            save_score_to_file("Assets/ldb.txt", playerName, ((int)(total_timer / 60)) + (((int)total_timer % 60) / 100.0f));
            keyState[ALLEGRO_KEY_ENTER] = false;
        }
    }

    if (namentered && mouseState.buttons && continueButton.hovered) {
        coins_obtained = 0;
        current_level = 1;
        total_timer = 0.0f;
        save_timer = 0.0f;
        last_update = 0.0f;
        namentered = false;
        change_scene(create_menu_scene());
    }
}
static void destroy_win_scene(void) {
    al_destroy_bitmap(pandaWinImg);
    al_stop_sample(&trophy_audio_id);
}
Scene create_win_scene(void) {
    Scene scene;
    memset(&scene, 0, sizeof(Scene));

    scene.name = "win";
    scene.init = &init_win_scene;
    scene.update = &update_win_scene;
    scene.draw = &draw_win_scene;
    scene.destroy = &destroy_win_scene;

    return scene;
}
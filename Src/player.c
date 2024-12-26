#include "player.h"
#include "game.h"
#include "utility.h"
#include "map.h"

#include <math.h>

static bool isCollision(Player* player, Map* map);

Player create_player(char * path, int row, int col){
    Player player;
    memset(&player, 0, sizeof(player));
    
    player.coord = (Point){
        col * TILE_SIZE,
        row * TILE_SIZE
    };
    player.direction = 0;
    player.speed = 4;
    player.health = 100;
    game_log("Player created with health: %d\n", player.health);
    player.image = al_load_bitmap(path);
    if(!player.image){
        game_abort("Error Load Bitmap with path : %s", path);
    }
    
    return player;
}

void update_player(Player * player, Map* map){

    Point original = player->coord;
    player->status = PLAYER_IDLE;
    if (player->health <= 0) {
        player->status = PLAYER_DYING;
        player->animation_tick += 1;
        if (player->animation_tick >= 64) {
            player->animation_tick -= 5;
        }
        return;
    }
    // Knockback effect
    if(player->knockback_CD > 0){

        player->knockback_CD--;
        int next_x = player->coord.x + player->speed * cos(player->knockback_angle);
        int next_y = player->coord.y + player->speed * sin(player->knockback_angle);
        Point next;
        next = (Point){next_x, player->coord.y};
        
        if(!isCollision(player, map)){
            player->coord = next;
        }
        
        next = (Point){player->coord.x, next_y};
        if(!isCollision(player, map)){
            player->coord = next;
        }
    }
    /*
        [TODO HACKATHON 1-1] 
        
        Player Movement
        Adjust the movement by player->speed

        if (keyState[ALLEGRO_KEY_W]) {
            player->coord.y = ...
            player->direction = ...
        }
        if (keyState[ALLEGRO_KEY_S]) {
            player->coord.y = ...
            player->direction = ...
        }
    */
    if (keyState[ALLEGRO_KEY_W]) {
        player->coord.y -= player->speed;
        player->direction = UP;
        player->status = PLAYER_WALKING;
    }
    if (keyState[ALLEGRO_KEY_S]) {
        player->coord.y += player->speed;
        player->direction = DOWN;
        player->status = PLAYER_WALKING;
    }

    // if Cdollide, snap to the grid to make it pixel perfect
    if(isCollision(player, map)){
        player->coord.y = round((float)original.y / (float)TILE_SIZE) * TILE_SIZE;
    }
    
    /*
        [TODO HACKATHON 1-2] 
        
        Player Movement
        Add/Subtract the movement by player->speed

        if (keyState[ALLEGRO_KEY_A]) {
            player->coord.y = ...
            player->direction = ...
        }
        if (keyState[ALLEGRO_KEY_D]) {
            player->coord.y = ...
            player->direction = ...
        }
    */
    if (keyState[ALLEGRO_KEY_A]) {
        player->coord.x -= player->speed;
        player->direction = LEFT;
        player->status = PLAYER_WALKING;
    }
    if (keyState[ALLEGRO_KEY_D]) {
        player->coord.x += player->speed;
        player->direction = RIGHT;
        player->status = PLAYER_WALKING;
    }
    if(isCollision(player, map)){
        player->coord.x = round((float)original.x / (float)TILE_SIZE) * TILE_SIZE;
    }
    /*
        [TODO Homework] 
        
        Calculate the animation tick to draw animation later
    */
    player->animation_tick = (player->animation_tick + 1) % 64;
    printf("Animation Tick: %d\n", player->animation_tick);

}

void draw_player(Player * player, Point cam){
    int dy = player->coord.y - cam.y; 
    int dx = player->coord.x - cam.x; 
    int flag = 0;
    if (keyState[ALLEGRO_KEY_A]){
        flag = flag;
    }
    else {
        flag = 1;
    }
    int x = 0;
    int y = 0;
    int red_box = player->knockback_CD > 0 ? 0 : 255;
    ///*
    //    [TODO Homework] 
    //    
    //    Draw Animation of Dying, Walking, and Idle
    //*/
    if (player->status == PLAYER_IDLE) {
        x = (int)(player->animation_tick / 32);
    }
    else if (player->status == PLAYER_WALKING) {
        y = 1;
        x = (int)(player->animation_tick / 16);
    }
    else if (player->status == PLAYER_DYING) {
        y = 2;
        x = (int)(player->animation_tick / 16);
        red_box = 255;
    }
    int src_x = x * 32;
    int src_y = y * 32;
    al_draw_tinted_scaled_bitmap(player->image, al_map_rgb(255, red_box, red_box),
        src_x, src_y, 32, 32,
        dx, dy, TILE_SIZE, TILE_SIZE,
        flag
    );
    
    #ifdef DRAW_HITBOX
        al_draw_rectangle(
            dx, dy, dx + TILE_SIZE, dy + TILE_SIZE,
            al_map_rgb(255, 0, 0), 1
        );
    #endif
    
}

void delete_player(Player * player){
    al_destroy_bitmap(player->image);
}

static bool isCollision(Player* player, Map* map){

    if (player->coord.x < 0 ||
        player->coord.y < 0 ||
        (player->coord.x + TILE_SIZE - 1) / TILE_SIZE >= map->col ||
        (player->coord.y + TILE_SIZE - 1) / TILE_SIZE >= map->row)
        return true;

    /*

        [TODO HACKATHON 2-1] 
    
        Check every corner of enemy if it's collide or not

        We check every corner if it's collide with the wall/hole or not
        You can activate DRAW_HITBOX in utility.h to see how the collision work

        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;
        if(!isWalkable(map->map[...][...])) return true;

    */
    int topleftx = player->coord.x / TILE_SIZE;
    int toplefty = player->coord.y / TILE_SIZE;
    int toprightx = (player->coord.x + TILE_SIZE - 1) / TILE_SIZE;
    int toprighty = player->coord.y / TILE_SIZE;
    int bottomleftx = player->coord.x / TILE_SIZE;
    int bottomlefty = (player->coord.y + TILE_SIZE - 1) / TILE_SIZE;
    int bottomrightx = (player->coord.x + TILE_SIZE - 1) / TILE_SIZE;
    int bottomrighty = (player->coord.y + TILE_SIZE - 1) / TILE_SIZE;
    if (!isWalkable(map->map[toplefty][topleftx])) return true;
    if (!isWalkable(map->map[toprighty][toprightx])) return true;
    if (!isWalkable(map->map[bottomlefty][bottomleftx])) return true;
    if (!isWalkable(map->map[bottomrighty][bottomrightx])) return true;
    return false;
}

void hitPlayer(Player * player, Point enemy_coord, int damage){
    if(player->knockback_CD == 0){
        float dY = player->coord.y - enemy_coord.y;
        float dX = player->coord.x - enemy_coord.x;
        float angle = atan2(dY, dX);

        /*
            [TODO Homework]
            
            Game Logic when the player get hit or die

            player->knockback_angle = angle;
            player->knockback_CD = 32;

            player->health = ...
            if(player->health <= 0){

                player->health = 0;
                player->status = ...

                ...
            }
        */
        player->knockback_angle = angle;
        player->knockback_CD = 32;

        player->health -= 10;
            if (player->health <= 0) {
                player->health = 0;
                player->status = PLAYER_DYING;
                player->animation_tick = 0;
            }
    }
}
void hitPlayer2(Player *player , int damage, float angle) {
    player->health -= damage;
    if (player->health <= 0) {
        player->health = 0;
        player->status = PLAYER_DYING;
        player->animation_tick = 0;
    }
    player->knockback_angle = angle;
    player->knockback_CD = 16;
}
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "player.h"
#include "game_scene.h"

Player player;

/*
    [OFFSET CALCULATOR FUNCTIONS]
      You might want to modify this functions if you are using different assets
      In short, this functions help us to find which asset to draw
 */
static void get_map_offset(Map* map);
static Point get_wall_offset_assets(Map* map, int i, int j);
static Point get_floor_offset_assets(Map* map, int i, int j);
static Point get_hole_offset_assets(Map* map, int i, int j);
static const int offset = 16;


static bool tile_collision(Point player, Point tile_coord);
int button_gate_id[MAX_MAP_ROW][MAX_MAP_COL];
static int trophy_animation = 0;

Map create_map(char* path, uint8_t type) {
    Map map;

    memset(&map, 0, sizeof(Map));

    FILE* f = fopen(path, "r");
    if (!f) {
        game_abort("Can't load map file : %s", path);
    }

    game_log("Using %s to load the map", path);

    fscanf(f, "%d %d", &map.row, &map.col);
    char ch;

    // Map Array
    map.map = (uint8_t**)malloc(map.row * sizeof(uint8_t*));
    for (int i = 0; i < map.row; i++) {
        map.map[i] = (uint8_t*)malloc(map.col * sizeof(uint8_t));
    }

    // Map Offset Resource Pack Array
    map.offset_assets = (Point**)malloc(map.row * sizeof(Point*));
    for (int i = 0; i < map.row; i++) {
        map.offset_assets[i] = (Point*)malloc(map.col * sizeof(Point));
    }

    int coin_counter = 0;
    map.coin_frame = 0; // coin animation tick
    map.trophy_frame = 0; // trophy animation tick
    map.coin_timer = 0.0;
    map.trophy_timer = 0.0;
    map.portal_frame = 0; // portal animation tick
    map.portal_timer = 0.0;

    // Scan the map to the array
    int door_counter = 0;
    for (int i = 0; i < map.row; i++) {
        for (int j = 0; j < map.col; j++) {
            fscanf(f, " %c", &ch);

            switch (ch)
            {
            case '#': // Case Wall
                map.map[i][j] = WALL;
                break;

            case '.': // Case Floor
                map.map[i][j] = FLOOR;
                break;

            case 'P': // Spawn Point
                map.map[i][j] = FLOOR;
                map.Spawn = (Point){ i, j };
                break;

            case 'S': // Slime Enemy
                map.map[i][j] = FLOOR;
                map.EnemyCode[map.EnemySpawnSize] = ch;
                map.EnemySpawn[map.EnemySpawnSize++] = (Point){ i, j };
                break;
            case 'F': // Fire Slime
                map.map[i][j] = FLOOR;
                map.EnemyCode[map.EnemySpawnSize] = ch;
                map.EnemySpawn[map.EnemySpawnSize++] = (Point){ i, j };
                break;
            case 'C': // Coins
                map.map[i][j] = COIN;
                map.coin_status[i][j] = APPEAR;
                coin_counter++;
                break;
            case 'B': // Button
                map.map[i][j] = BUTTON;
                break;
            case 'T': // Trophy
                map.map[i][j] = TROPHY;
                break;
            case 'D': // Portal
                map.map[i][j] = DOOR;
                break;
            case 'G': // Door
                map.map[i][j] = GATE;
                map.door_status[i][j] = CLOSED;
                break;
            case '_': // Nothing
                map.map[i][j] = HOLE;
                break;
            case 'H': //Rumble Potion
                map.map[i][j] = HP;
                break;
            default: // If not determined, set it as black
                map.map[i][j] = NOTHING;
                break;
            }
        }
    }
    map.assets = al_load_bitmap("Assets/map_packets.png");
    if (!map.assets) {
        game_abort("Can't load map assets");
    }

    map.coin_assets = al_load_bitmap("Assets/coins.png");
    if (!map.coin_assets) {
        game_abort("Can't load coin assets");
    }

    // load the offset for each tiles type
    get_map_offset(&map);

    map.coin_audio = al_load_sample("Assets/audio/coins.mp3");
    if (!map.coin_audio) {
        game_abort("Can't load coin audio");
    }

    map.trophy_assets = al_load_bitmap("Assets/trophy.png");
    if (!map.trophy_assets) {
        game_abort("Can't load trophy assets");
    }

    map.trophy_audio = al_load_sample("Assets/audio/win2.mp3");
    if (!map.trophy_audio) {
        game_abort("Can't load trophy audio");
    }
    
    map.door_assets = al_load_bitmap("Assets/portal.png");
    if (!map.trophy_assets) {
        game_abort("Can't load trophy assets");
    }

    map.gate_assets = al_load_bitmap("Assets/Door.png");
    if (!map.gate_assets) {
        game_abort("Can't load trophy assets");
    }

    map.button_assets = al_load_bitmap("Assets/Button.png");
    if (!map.button_assets) {
        game_abort("Can't load button assets");
    }
    
    map.heart_assets = al_load_bitmap("Assets/rumble.png");
    if (!map.heart_assets) {
        game_abort("Can't load heart assets");
    }

    map.move = false;
    map.win = false;

    fclose(f);

    return map;
}

void draw_map(Map* map, Point cam) {
    al_clear_to_color(al_map_rgb(24, 20, 37));

    for (int i = 0; i < map->row; i++) {
        for (int j = 0; j < map->col; j++) {

            int dy = i * TILE_SIZE - cam.y; 
            int dx = j * TILE_SIZE - cam.x; 

            Point offset_asset = map->offset_assets[i][j];

            al_draw_scaled_bitmap(map->assets, 
                offset_asset.x, offset_asset.y, 16, 16, 
                dx, dy, TILE_SIZE, TILE_SIZE, 
                0 // flag : set 0
            );

            switch (map->map[i][j]) {
            case COIN: {
                const int frame_width = 16;  
                const int frame_height = 16; 
                int frame_x = 0;
                int frame_y = 0; 
                if (map->coin_status[i][j] == APPEAR) {
                    frame_x = map->coin_frame * frame_width;
                }
                else if (map->coin_status[i][j] == DISAPPEARING) {
                    static int tick_counter = 0;
                    int frame_duration = 5;    
                    tick_counter++;
                    if (tick_counter >= frame_duration) {
                        if (map->coin_disappear_animation[i][j] <= 8) {
                            map->coin_disappear_animation[i][j]++;
                        }
                        tick_counter = 0;
                    }
                    frame_y = 1 * frame_height;
                    frame_x = map->coin_disappear_animation[i][j] * frame_width;
                    if (map->coin_disappear_animation[i][j] == 8) {
                        map->coin_status[i][j] = DISAPPEAR;
                        map->coin_timer_disappear = 0.0;
                        map->coin_disappear_animation[i][j] = 0; 
                    }
                }
                else if (map->coin_status[i][j] == DISAPPEAR) {
                    frame_y = 1 * frame_height;
                    frame_x = 8 * frame_width;
                    map->map[i][j] = FLOOR;
                    break;
                }
                al_draw_scaled_bitmap(
                    map->coin_assets,      
                    frame_x, frame_y,      
                    frame_width, frame_height, 
                    dx, dy,                
                    TILE_SIZE, TILE_SIZE,  
                    0                     
                );
                break;
            }
            case TROPHY: {
                const int frame_width = 32;   
                const int frame_height = 32;
                const float scale_factor = 1.0; 
                const int trophy_width = TILE_SIZE * scale_factor; 
                const int trophy_height = TILE_SIZE * scale_factor; 
                int frame_x = map->trophy_frame * frame_width;
                int draw_dx = dx - (trophy_width - TILE_SIZE) / 2; 
                int draw_dy = dy - (trophy_height - TILE_SIZE) / 2; 
                al_draw_scaled_bitmap(
                    map->trophy_assets,     
                    frame_x, 0,             
                    frame_width, frame_height, 
                    draw_dx, draw_dy,      
                    trophy_width, trophy_height, 
                    0                     
                );
                break;
            }
            case DOOR: {
                const int frame_width = 1000/4; 
                const int frame_height = 592 ; 
                const float scale_factor = 1.0; 
                const int portal_width = TILE_SIZE * scale_factor; 
                const int portal_height = TILE_SIZE * scale_factor; 
                int frame_x = map->portal_frame * frame_width;
                int draw_dx = dx - (portal_width - TILE_SIZE) / 2;
                int draw_dy = dy - (portal_height - TILE_SIZE) / 2; 
                al_draw_scaled_bitmap(
                    map->door_assets,
                    frame_x, 0,                    
                    frame_width , frame_height,   
                    draw_dx, draw_dy,  
                    TILE_SIZE*scale_factor, TILE_SIZE*scale_factor, 
                    0
                );
                break;
            }
            case GATE: {
                int frame_width = 224 / 7;  
                int frame_height = 64 / 4; 
                map->door_timer[i][j] += 1.0 / 60.0;
                if (map->door_status[i][j] == OPENING) {
                    if (map->door_timer[i][j] >= 0.3) {
                        map->door_animation_frame[i][j]++;
                        if (map->door_animation_frame[i][j] >= 6) {
                            map->door_status[i][j] = OPEN;
                            map->map[i][j] = DOOR_OPEN;
                        }
                        map->door_timer[i][j] = 0.0;
                    }   
                }
                int frame_index = map->door_animation_frame[i][j];
                int source_x = frame_index * frame_width;
                al_draw_scaled_bitmap(
                    map->gate_assets,     
                    source_x, 0,    
                    frame_width, frame_height, 
                    dx, dy,               
                    TILE_SIZE, TILE_SIZE, 
                    0                   
                );
                break;
            }
            case DOOR_OPEN: {
                int frame_index = 6;
                int frame_width = 32;
                int frame_height = 16;
                int source_x = frame_index * frame_width;
                al_draw_scaled_bitmap(
                    map->gate_assets,      
                    source_x, 0,           
                    frame_width, frame_height, 
                    dx, dy,                
                    TILE_SIZE, TILE_SIZE, 
                    0                      
                );
                break;
            }
            case BUTTON: {
                if (map->button_assets) {
                    static int frame_counter = 0;
                    static int tick = 0;
                    int frames = 3;              
                    int frame_duration = 10;     
                    int source_x = 0;           
                    int source_y = 0;            
                    if (map->button_status[i][j] == HOVERED) {
                        if (frame_counter < frames - 1) {
                            tick++;
                            if (tick >= frame_duration) {
                                frame_counter++;     
                                tick = 0;      
                            }
                        }
                        int current_frame = frame_counter % frames;
                        source_x = current_frame * 16; 
                    }
                    else {
                        source_x = 0;
                        frame_counter = 0;
                        tick = 0;
                    }
                    al_draw_scaled_bitmap(
                        map->button_assets,
                        source_x, source_y, 
                        16, 16, 
                        dx, dy,                     
                        TILE_SIZE, TILE_SIZE,        
                        0                            
                    );
                }
                break;
            }
            case HP: {
                int frame_width = 452;
                int frame_height = 451;
                static int animation_tick = 0;
                static int direction = 1;    
                const int max_tick = 20;       
                const int amplitude = 5;
                int offsety = (animation_tick * amplitude) / max_tick;
                animation_tick += direction;
                if (animation_tick >= max_tick || animation_tick <= 0) {
                    direction *= -1;
                }
                al_draw_scaled_bitmap(
                    map->heart_assets,
                    0, 0,
                    frame_width, frame_height,
                    dx, dy + offsety,
                    TILE_SIZE, TILE_SIZE,
                    0
                );

            }
            default:
                break;
            }
#ifdef DRAW_HITBOX
            al_draw_rectangle(
                dx, dy, dx + TILE_SIZE, dy + TILE_SIZE,
                al_map_rgb(0, 255, 0), 1
            );
#endif
        }
    }
}

void update_map(Map* map, Point player_coord, int* total_coins) {
    /*
        Hint: To check if it's collide with object in map, you can use tile_collision function
        e.g. to update the coins if you touch it
    */
    map->coin_timer += 1.0 / 60.0; 
    if (map->coin_timer >= 0.08) { 
        map->coin_frame = (map->coin_frame + 1) % 8; 
        map->coin_timer = 0.0;
    }
    map->trophy_timer += 1.0 / 60.0; 
    if (map->trophy_timer >= 0.2) { 
        map->trophy_frame = (map->trophy_frame + 1) % 10; 
        map->trophy_timer = 0.0;
    }
    map->portal_timer += 1.0 / 60.0; 
    if (map->portal_timer >= 0.2) {
        map->portal_frame = (map->portal_frame + 1) % 4; 
        map->portal_timer = 0.0;
    }
    
    for (int i = 0; i < map->row; i++) {
        for (int j = 0; j < map->col; j++) {
            if (map->map[i][j] == COIN && map->coin_status[i][j] == APPEAR) {
                Point tile_coord = { j * TILE_SIZE, i * TILE_SIZE };
                if (tile_collision(player_coord, tile_coord)) {
                    (*total_coins)++;
                    map->coin_status[i][j] = DISAPPEARING; 
                    map->coin_disappear_animation[i][j] = 0;
                    al_play_sample(map->coin_audio, SFX_VOLUME, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
            }
            if (map->map[i][j] == TROPHY) {
                Point tile_coord = { j * TILE_SIZE, i * TILE_SIZE };
                if (tile_collision(player_coord, tile_coord)) {
                    map->win = true;
                }
            }
            if (map->map[i][j] == DOOR) {
                Point tile_coord = { j * TILE_SIZE, i * TILE_SIZE };
                if (tile_collision(player_coord, tile_coord)) {
                    map->move = true;
                }
            }

            if (map->map[i][j] == BUTTON) {
                Point tile_coord = { j * TILE_SIZE, i * TILE_SIZE };
                if (tile_collision(player_coord, tile_coord)) {
                    map->button_status[i][j] = HOVERED;
                    int button_id = button_gate_id[i][j];
                    for (int x = 0; x < map->row; x++) {
                        for (int y = 0; y < map->col; y++) {
                            if (map->map[x][y] == GATE && button_gate_id[x][y] == button_id) {
                                map->door_status[x][y] = OPENING;
                            }
                        }
                    }
                }
            }
            if (map->map[i][j] == GATE) {
                if (map->door_status[i][j] == TRIGGERED) {
                    map->door_status[i][j] = OPENING;
                }
            }
            if (map->map[i][j] == HP) {
                Point tile_coord = { j * TILE_SIZE, i * TILE_SIZE };
                if (tile_collision(player_coord, tile_coord)) {
                    map->map[i][j] = FLOOR;
                    player.health += 10;
                    player.speed += 4;
                }
            }
        }
    }
}

void destroy_map(Map* map) {
    for (int i = 0; i < map->row; i++) {
        free(map->map[i]);
        free(map->offset_assets[i]);
    }
    free(map->map);
    free(map->offset_assets);
    al_destroy_bitmap(map->assets);
    al_destroy_bitmap(map->coin_assets);
    al_destroy_sample(map->coin_audio);
    al_destroy_bitmap(map->trophy_assets);
    al_destroy_bitmap(map->door_assets);
    al_destroy_bitmap(map->button_assets);
    al_destroy_bitmap(map->gate_assets);
    al_destroy_bitmap(map->heart_assets);
}

bool isWalkable(BLOCK_TYPE block) {
    if (block == FLOOR || block == COIN || block == TROPHY || block == DOOR || block == BUTTON || block == DOOR_OPEN || block == HP) return true;
    return false;
}

/*
    DON'T CHANGE CODE BELOW UNLESS YOU ARE UNDERSTAND WELL
    OR WANT TO CHANGE TO DIFFERENT ASSETS
 */
static bool isWall(Map* map, int i, int j);
//static bool isFloorHole(Map * map, int i, int j);
static bool isFloor(Map* map, int i, int j);
static bool isNothing(Map* map, int i, int j);

static Point get_floor_offset_assets(Map* map, int i, int j) {
    bool up = isWall(map, i - 1, j);
    bool left = isWall(map, i, j - 1);
    bool right = isWall(map, i, j + 1);
    bool top_left = isWall(map, i - 1, j - 1);
    bool top_right = isWall(map, i - 1, j + 1);

    if (up && left && right) {
        return (Point) { offset * 12, offset * 3 };
    }
    if (up && left) {
        return (Point) { offset * 11, 0 };
    }
    if (up && right) {
        return (Point) { offset * 13, 0 };
    }
    if (left) {
        if (top_left)
            return (Point) { offset * 11, offset * 1 };
        else
            return (Point) { offset * 15, offset * 1 };

    }
    if (right) {
        if (top_right)
            return (Point) { offset * 13, offset * 1 };
        else
            return (Point) { offset * 14, offset * 1 };
    }
    if (up) {
        return (Point) { offset * 12, 0 };
    }
    if (top_left) {
        return (Point) { offset * 11, offset * 3 };
    }
    if (top_right) {
        return (Point) { offset * 13, offset * 3 };
    }

    return (Point) { 12 * offset, 4 * offset };
}

// Calculate the offset from the source assets
static Point get_wall_offset_assets(Map* map, int i, int j) {
    bool up = isWall(map, i - 1, j);
    bool down = isWall(map, i + 1, j);
    bool left = isWall(map, i, j - 1);
    bool right = isWall(map, i, j + 1);

    if (up && down && left && right) {
        return (Point) { 3 * offset, 5 * offset };
    }
    if (up && down && right) {
        bool left_floor = isFloor(map, i, j - 1);
        bool right_down = isWall(map, i + 1, j + 1);
        bool right_up = isWall(map, i - 1, j + 1);
        if (right_down && right_up)
            return (Point) { 2 * offset, 5 * offset };
        if (left_floor) {
            return (Point) { 1 * offset, 3 * offset };
        }
        else {
            return (Point) { 4 * offset, 5 * offset };
        }
    }
    if (up && down && left) {
        bool right_floor = isFloor(map, i, j + 1);
        bool left_down = isWall(map, i + 1, j - 1);
        bool left_up = isWall(map, i - 1, j - 1);
        if (left_down && left_up)
            return (Point) { 4 * offset, 5 * offset };
        if (right_floor) {
            return (Point) { 1 * offset, 3 * offset };
        }
        else {
            return (Point) { 2 * offset, 5 * offset };
        }
    }
    if (down && right && left) {
        bool down_right_wall = isWall(map, i + 1, j + 1);
        bool down_left_wall = isWall(map, i + 1, j - 1);
        bool down_right = isFloor(map, i + 1, j + 1);
        bool down_left = isFloor(map, i + 1, j - 1);
        if (down_right_wall && down_left_wall)
            return (Point) { 3 * offset, 4 * offset };
        if ((down_right ^ down_left) == 0)
            return (Point) { 8 * offset, 5 * offset };
        if (down_right)
            return (Point) { 4 * offset, 4 * offset };
        if (down_left)
            return (Point) { 2 * offset, 4 * offset };
    }
    if (left && right) {
        if (isFloor(map, i + 1, j))
            return (Point) { 7 * offset, 5 * offset };
        else
            return (Point) { 7 * offset, 7 * offset };
    }
    if (down && up) {
        bool left_ = isFloor(map, i, j - 1);
        bool right_ = isFloor(map, i, j + 1);
        if ((left_ ^ right_) == 0)
            return (Point) { 1 * offset, 3 * offset };
        if (left_)
            return (Point) { 2 * offset, 5 * offset };
        return (Point) { 4 * offset, 5 * offset };
    }
    if (down && right) {
        if (isFloor(map, i + 1, j + 1))
            return (Point) { 4 * offset, 5 * offset };
        else
            return (Point) { 2 * offset, 4 * offset };
    }
    if (down && left) {
        if (isFloor(map, i, j + 1) && isFloor(map, i + 1, j - 1))
            return (Point) { 1 * offset, 2 * offset };
        else if (isFloor(map, i + 1, j - 1))
            return (Point) { 2 * offset, 5 * offset };
        else
            return (Point) { 4 * offset, 4 * offset };
    }
    if (up && right) {
        if (isFloor(map, i + 1, j))
            return (Point) { 2 * offset, 6 * offset };
        else
            return (Point) { 5 * offset, 6 * offset };
    }
    if (up && left) {
        if (isFloor(map, i + 1, j))
            return (Point) { 4 * offset, 6 * offset };
        else
            return (Point) { 6 * offset, 6 * offset };
    }
    if (left || right) {
        if (isFloor(map, i + 1, j))
            return (Point) { 7 * offset, 5 * offset };
        else
            return (Point) { 7 * offset, 7 * offset };
    }
    if (down) {
        return (Point) { 1 * offset, 2 * offset };
    }
    if (up) {
        return (Point) { 1 * offset, 4 * offset };
    }

    return (Point) { 0, 0 };
}

static Point get_hole_offset_assets(Map* map, int i, int j) {
    bool up = isNothing(map, i - 1, j);
    bool U_Floor = isFloor(map, i - 1, j);

    if (up) {
        return (Point) { 0, 0 };
    }
    if (U_Floor) {
        bool UL = isFloor(map, i - 1, j - 1);
        bool UR = isFloor(map, i - 1, j + 1);
        if (UL && UR) return (Point) { offset * 13, offset * 11 };
        if (UL) return (Point) { offset * 14, offset * 11 };
        if (UR) return (Point) { offset * 12, offset * 11 };
        return (Point) { offset * 10, offset * 12 };
    }
    return (Point) { 0, 0 };
}

static Point get_nothing_offset_assets(Map* map, int i, int j) {
    bool U_Floor = isFloor(map, i - 1, j);
    if (U_Floor) {
        bool UL = isFloor(map, i - 1, j - 1);
        bool UR = isFloor(map, i - 1, j + 1);
        if (UL && UR) return (Point) { offset * 13, offset * 11 };
        if (UL) return (Point) { offset * 14, offset * 11 };
        if (UR) return (Point) { offset * 12, offset * 11 };
        return (Point) { offset * 10, offset * 12 };
    }
    return (Point) { 0, 0 };
}

static void get_map_offset(Map* map) {
    // Calculate once to use it later on when draw() function called
    for (int i = 0; i < map->row; i++) {
        for (int j = 0; j < map->col; j++) {
            switch (map->map[i][j]) {
            case WALL:
                map->offset_assets[i][j] = get_wall_offset_assets(map, i, j);
                break;
            case FLOOR:
            case COIN:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case HOLE:
                map->offset_assets[i][j] = get_hole_offset_assets(map, i, j);
                break;
            case TROPHY:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case DOOR:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case GATE:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case BUTTON:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case HP:
                map->offset_assets[i][j] = get_floor_offset_assets(map, i, j);
                break;
            case NOTHING:
            default:
                map->offset_assets[i][j] = (Point){ 0, 0 };
                break;
            }
        }
    }
}

static bool isWall(Map* map, int i, int j) {
    if (i < 0 || j < 0 || i >= map->row || j >= map->col) return false;
    if (map->map[i][j] == WALL) return true;
    return false;
}

static bool isFloor(Map* map, int i, int j) {
    if (i < 0 || j < 0 || i >= map->row || j >= map->col) return false;
    if (map->map[i][j] == FLOOR) return true;
    return false;
}

static bool isNothing(Map* map, int i, int j) {
    if (i < 0 || j < 0 || i >= map->row || j >= map->col) return true;
    if (map->map[i][j] == NOTHING || map->map[i][j] == HOLE) return true;
    return false;
}


/*
    Collision
 */
static bool tile_collision(Point player, Point tile_coord) {

    if (player.x < tile_coord.x + TILE_SIZE &&
        player.x + TILE_SIZE > tile_coord.x &&
        player.y < tile_coord.y + TILE_SIZE &&
        player.y + TILE_SIZE > tile_coord.y) {
        return true; // Rectangles collide
    }
    else {
        return false; // Rectangles do not collide
    }
}

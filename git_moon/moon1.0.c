#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <math.h>

#define WIDTH 60
#define HEIGHT 25
#define MAX_ENEMIES 8
#define MAX_PROJECTILES 15

typedef struct { int x, y; int alive; int alert; } Entity;
typedef struct { float x, y; float dx, dy; int active; } Projectile;

Entity player = {5, 5, 1, 0};
Entity enemies[MAX_ENEMIES];
Projectile darts[MAX_PROJECTILES];
int map[HEIGHT][WIDTH];

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void init_game() {
    srand(time(NULL));
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) map[y][x] = 1;
            else if (rand() % 50 == 0) map[y][x] = 1; 
            else map[y][x] = 0;
        }
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = rand() % (WIDTH - 10) + 5;
        enemies[i].y = rand() % (HEIGHT - 10) + 5;
        enemies[i].alive = 1;
        enemies[i].alert = 0;
    }
    for (int i = 0; i < MAX_PROJECTILES; i++) darts[i].active = 0;
}

void draw() {
    COORD cursorPosition = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == player.x && y == player.y) {
                set_color(11); printf("@"); // 侠客
            } else {
                int drawn = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
                        set_color(enemies[i].alert ? 4 : 12); 
                        printf("X"); 
                        drawn = 1; break;
                    }
                }
                if (drawn) continue;
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (darts[i].active && (int)darts[i].x == x && (int)darts[i].y == y) {
                        set_color(14); printf("*"); // 暗器
                        drawn = 1; break;
                    }
                }
                if (drawn) continue;
                set_color(8);
                if (map[y][x] == 1) printf("#");
                else printf(".");
            }
        }
        printf("\n");
    }
    set_color(7);
    SetConsoleOutputCP(CP_UTF8);
    printf("你不动我不动，天地俱寂，万籁无声\n");
    printf("\n[状态] %s | WASD/方向键移动 | 红色代表贼寇敌人，靠近贼寇自动斩杀\n", player.alive ? "气定神闲" : "力尽身亡");
    printf("[提示] 黄色代表敌人暗器！敌人变红代表即将发射暗器！利用地形 '#' 躲避。\n");
}

void update_world() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!darts[i].active) continue;
        darts[i].x += darts[i].dx;
        darts[i].y += darts[i].dy;

        int dx = (int)darts[i].x;
        int dy = (int)darts[i].y;

        if (dx <= 0 || dx >= WIDTH-1 || dy <= 0 || dy >= HEIGHT-1 || map[dy][dx] == 1) {
            darts[i].active = 0;
            continue;
        }
        if (abs(dx - player.x) < 1 && abs(dy - player.y) < 1) {
            player.alive = 0;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;

        if (enemies[i].x == player.x || enemies[i].y == player.y) {
            if (enemies[i].alert == 0) {
                enemies[i].alert = 1; 
            } else {
                for (int j = 0; j < MAX_PROJECTILES; j++) {
                    if (!darts[j].active) {
                        darts[j].active = 1;
                        darts[j].x = (float)enemies[i].x;
                        darts[j].y = (float)enemies[i].y;
                        darts[j].dx = (player.x > enemies[i].x) ? 1.0f : (player.x < enemies[i].x ? -1.0f : 0);
                        darts[j].dy = (player.y > enemies[i].y) ? 1.0f : (player.y < enemies[i].y ? -1.0f : 0);
                        enemies[i].alert = 0;
                        break;
                    }
                }
            }
        } else {
            enemies[i].alert = 0;
        }

        if (enemies[i].alert == 0) {
            int move_x = (player.x > enemies[i].x) ? 1 : (player.x < enemies[i].x ? -1 : 0);
            int move_y = (player.y > enemies[i].y) ? 1 : (player.y < enemies[i].y ? -1 : 0);
            if (map[enemies[i].y + move_y][enemies[i].x + move_x] == 0) {
                enemies[i].x += move_x;
                enemies[i].y += move_y;
            }
        }

        if (enemies[i].x == player.x && enemies[i].y == player.y) player.alive = 0;
    }
}

int main() {
    system("mode con cols=62 lines=32");
    CONSOLE_CURSOR_INFO cursorInfo = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    init_game();
    draw();

    while (player.alive) {
        // --- 核心变更：胜利检测 ---
        int alive_count = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].alive) alive_count++;
        }

        if (alive_count == 0) {
            set_color(10); // 亮绿色
            SetConsoleOutputCP(CP_UTF8);
            printf("\n【捷报】众寇已除，深藏身与名！ [按任意键拂袖而去]\n");
            getch();
            return 0; 
        }
        // -------------------------

        if (kbhit()) {
            int ch = getch();
            if (ch == 224) ch = getch(); 
            
            int nx = player.x, ny = player.y;
            if (ch == 'w' || ch == 72) ny--;
            else if (ch == 's' || ch == 80) ny++;
            else if (ch == 'a' || ch == 75) nx--;
            else if (ch == 'd' || ch == 77) nx++;
            else if (ch == ' ') ; 
            else continue;

            if (map[ny][nx] == 0) {
                player.x = nx;
                player.y = ny;

                update_world(); 

                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].alive && abs(enemies[i].x - player.x) <= 1 && abs(enemies[i].y - player.y) <= 1) {
                        enemies[i].alive = 0;
                    }
                }
                draw();
            }
        }
        Sleep(10);
    }

    set_color(12);
    SetConsoleOutputCP(CP_UTF8);
    printf("\n【陨落】相濡以沫，不如相忘于江湖。 [按任意键退出]\n");
    getch();
    return 0;
}
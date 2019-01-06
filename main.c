// Current Version 0.9.0 システム的には一応仮完成
//グラフィック系のデバッグが全然できてないのでバグが発生するかも
//後はもうちょっとマシなタイトルリザルトとお粗末すぎるグラフィックの改善、コードの効率化
//間に合わなかったもの
//衝突時のグラフィカルなエフェクト

//メモ
// LongRangeMissile →芋をころすためのミサイル
// GamelikeMissile →いる位置に向かって飛んでく
// ReallikeMissile →未来位置を計算してそこに飛んでいく

#include <handy.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PLAYER_SPEED 1.0
#define BEAM_SPEED 25
#define MAX_BEAM 6
#define MAX_MISSILE 10
#define GETFRAMES 5

/*-------- struct start --------*/

typedef struct Character {
    double size;
    double x;
    double y;
    int hp;
    double xmoved[GETFRAMES];
    double ymoved[GETFRAMES];
    double xvector;
    double yvector;
    double dx;
    double dy;
} Character;

typedef struct Weapon {
    int exist;
    int type;
    int fuel;
    int homing;
    double size;
    double x;
    double y;
    double velocity;
    double angle;
    double dx;
    double dy;
} Weapon;

/*-------- struct end --------*/
/*-------- declare function start --------*/

void draw_title();

void draw_player(int lid, Character player);

void draw_enemy(int lid, Character enemy);

Weapon import_beam(Character player, Weapon beam, int count);

Weapon import_missile(Character player, Weapon missile);

void draw_beam(int lid, Weapon beam);

Weapon calculate_gamelike_missile(Character player, Weapon missile);

Weapon calculate_reallike_missile(Character player, Weapon missile);

Weapon calculate_longrange_missile(Character player, Weapon missile);

void draw_missile(int lid, Weapon missile);

Weapon move_missile(Weapon missile);

void draw_hpstats(int lid, Character player, Character enemy);

int check_collision(Character character, Weapon weapon);

double Xrotate(double x, double y, double x0, double y0, double angle);

double Yrotate(double x, double y, double x0, double y0, double angle);

void draw_ending(int hp);

/*-------- declare function end --------*/
/*-------- main start --------*/

int main() {
    hgevent *event;
    doubleLayer layers;
    int windowID;
    int layerID;
    int frame = 0;
    double tmp_x_position;
    double tmp_y_position;
    Character player = {20.0, 200.0, 300.0, 5};
    Character enemy = {150.0, 850.0, 300.0, 50};
    Weapon beam[MAX_BEAM] = {};
    Weapon missile[MAX_MISSILE] = {};

    for (int i = 0; i < 5; i++) {  //この辺ハードコード 直したい
        beam[i].dx = BEAM_SPEED;
    }

    srandom(time(NULL));

    windowID = HgOpen(1000.0, 600.0);

    draw_title();

    layers = HgWAddDoubleLayer(windowID);
    HgSetEventMask(HG_KEY_DOWN);

    for (;; frame++) {
        layerID = HgLSwitch(&layers);
        HgLClear(layerID);
        event = HgEventNonBlocking();
        // printf("-------- frame = %d --------\n", frame);
        if (frame == 20) {
            frame = 0;
            for (int i = 0, count = 0; i < MAX_BEAM; i++) {
                if (beam[i].exist == 0) {
                    beam[i] = import_beam(player, beam[i], count);
                    count++;
                    if (count >= 2) {
                        count = 0;
                        break;
                    }
                }
            }
            if (random() % 2 == 0) {
                for (int i = 0; i < MAX_MISSILE; i++) {
                    if (missile[i].exist == 0) {
                        missile[i].type = random() % 2 + 1;
                        // missile[i].type = 1;
                        missile[i] = import_missile(player, missile[i]);
                        break;
                    }
                }
            }
        } else if (frame % 5 == 0) {
            if (hypot(player.x - enemy.x, player.y - enemy.y) - 650.0 > 0) {
                // printf("distance = %f\n", (hypot(player.x - enemy.x, player.y
                // - enemy.y) - 650.0) / 4.0);
                if (random() % 100 < (int)(hypot(player.x - enemy.x, player.y - enemy.y) - 650.0) / 4) {
                    // printf("芋\n");
                    for (int i = 0; i < MAX_MISSILE; i++) {
                        if (missile[i].exist == 0) {
                            missile[i].type = 3;
                            missile[i] = import_missile(player, missile[i]);
                            break;
                        }
                    }
                }
            }
        }
        // printf("距離 = %4.3f\n", hypot(player.x - enemy.x, player.y -
        // enemy.y)); メモ デフォ650.0 左端 850.0 対角線 901.38 650で0,
        // 850で50%くらいの出現率にしたい
        tmp_x_position = player.x;
        tmp_y_position = player.y;
        if (event != NULL) {
            switch (event->ch) {
                case 'w':
                case HG_U_ARROW:
                    if (hypot(player.dx, player.dy) < 14.0) {
                        // player.y += PLAYER_SPEED;
                        player.dy += PLAYER_SPEED;
                        // printf("y+moved\n");
                    } else {
                        if (hypot(player.dx, player.dy) > hypot(player.dx, player.dy + PLAYER_SPEED)) {
                            player.dy += PLAYER_SPEED;
                        }
                    }
                    break;
                case 's':
                case HG_D_ARROW:
                    if (hypot(player.dx, player.dy) < 14.0) {
                        // player.y -= PLAYER_SPEED;
                        player.dy -= PLAYER_SPEED;
                        // printf("y-moved\n");
                    } else {
                        if (hypot(player.dx, player.dy) > hypot(player.dx, player.dy - PLAYER_SPEED)) {
                            player.dy -= PLAYER_SPEED;
                        }
                    }
                    break;
                case 'd':
                case HG_R_ARROW:
                    if (hypot(player.dx, player.dy) < 14.0) {
                        // player.x += PLAYER_SPEED;
                        player.dx += PLAYER_SPEED;
                        // printf("x+moved\n");
                    } else {
                        if (hypot(player.dx, player.dy) > hypot(player.dx + PLAYER_SPEED, player.dy)) {
                            player.dx += PLAYER_SPEED;
                        }
                    }
                    break;
                case 'a':
                case HG_L_ARROW:
                    if (hypot(player.dx, player.dy) < 14.0) {
                        // player.x -= PLAYER_SPEED;
                        player.dx -= PLAYER_SPEED;
                        // printf("x-moved\n");
                    } else {
                        if (hypot(player.dx, player.dy) > hypot(player.dx - PLAYER_SPEED, player.dy)) {
                            player.dx -= PLAYER_SPEED;
                        }
                    }
                    break;
            }
            // if((event->ch == 'w') || (event->ch == HG_U_ARROW)){
            //   player.y += PLAYER_SPEED;
            // }else if((event->ch == 's') || (event->ch == HG_D_ARROW)){
            //   player.y -= PLAYER_SPEED;
            // }else if((event->ch == 'a') || (event->ch == HG_L_ARROW)){
            //   player.x -= PLAYER_SPEED;
            // }else if((event->ch == 'd') || (event->ch == HG_R_ARROW)){
            //   player.x += PLAYER_SPEED;
            // }
        }
        if (player.x > 0 && player.x < 1000.0) {
            player.x += player.dx;
        }
        if (player.y > 0 && player.y < 600.0) {
            player.y += player.dy;
        }

        player.xmoved[0] = player.x - tmp_x_position;
        player.ymoved[0] = player.y - tmp_y_position;
        // printf("%2.1f\n", player.xmoved[0]);
        for (int i = 4; i > 0; i--) {
            player.xmoved[i] = player.xmoved[i - 1];
            player.ymoved[i] = player.ymoved[i - 1];
        }
        player.xvector = (player.xmoved[0] + player.xmoved[1] + player.xmoved[2] + player.xmoved[3] + player.xmoved[4]) / 5.0;
        player.yvector = (player.ymoved[0] + player.ymoved[1] + player.ymoved[2] + player.ymoved[3] + player.ymoved[4]) / 5.0;
        // printf("xvector = %2.1f\n",
        //  (player.xmoved[0]+player.xmoved[1]+player.xmoved[2]+player.xmoved[3]+player.xmoved[4]) / 5.0);
        // printf("yvector = %2.1f\n",
        //  (player.ymoved[0]+player.ymoved[1]+player.ymoved[2]+player.ymoved[3]+player.ymoved[4]) / 5.0);
        draw_player(layerID, player);
        draw_enemy(layerID, enemy);
        for (int i = 0; i < 5; i++) {
            if (beam[i].exist == 1) {
                draw_beam(layerID, beam[i]);
                if (check_collision(enemy, beam[i]) == 1) {
                    beam[i].exist = 0;
                    enemy.hp -= 1;
                } else if (beam[i].x + BEAM_SPEED > 1000) {
                    beam[i].exist = 0;
                } else {
                    beam[i].x += beam[i].dx;
                }
            }
        }
        for (int i = 0; i < MAX_MISSILE; i++) {
            if (missile[i].exist == 1) {
                // missile[i].dx = cos(atan2(player.y - missile[i].y, player.x - missile[i].x)) * missile[i].velocity; missile[i].dy =
                // sin(atan2(player.y - missile[i].y, player.x - missile[i].x)) * missile[i].velocity;
                draw_missile(layerID, missile[i]);
                if (missile[i].type == 1) {
                    // printf("type = 1\n");
                    missile[i] = calculate_gamelike_missile(player, missile[i]);
                } else if (missile[i].type == 2) {
                    // printf("type = 2\n");
                    missile[i] = calculate_reallike_missile(player, missile[i]);
                } else {
                    missile[i] = calculate_longrange_missile(player, missile[i]);
                }
                missile[i] = move_missile(missile[i]);
                if (check_collision(player, missile[i]) == 1) {
                    missile[i].exist = 0;
                    player.hp -= 1;
                } else if (missile[i].velocity < 3.0) {
                    missile[i].exist = 0;
                }
                //ミサイル誘導切れの消去のアレを書くかも。
            }
        }
        draw_hpstats(layerID, player, enemy);

        HgSleep(1.0 / 20.0);

        if (enemy.hp <= 0 || player.hp <= 0) break;
        // printf("x = %4.2f\n", player.x);
        // printf("y = %4.2f\n", player.y);
    }

    draw_ending(enemy.hp);

    return 0;
}

/*-------- main end -------- */
/*-------- function detail start --------*/

/*
void draw_title : タイトルを描画する
引数 | なし
返値 | なし
*/
void draw_title() {
    HgText(415, 300, "タイトルを入力してください");
    HgText(450, 200, "press any key");
    HgGetChar();
    HgClear();
}

/*
void draw_player : プレイヤーを描画する
引数 | int lid, Character player : レイヤーID, プレイヤーの情報
返値 | なし
*/
void draw_player(int lid, Character player) {
    double xnose[4] = {player.x + player.size, player.x + player.size,
                       player.x + player.size * 0.75,
                       player.x + player.size * 0.75};
    double ynose[4] = {
        player.y + player.size * 0.05, player.y - player.size * 0.05,
        player.y - player.size * 0.08, player.y + player.size * 0.08};
    // HgWMoveTo(lid, player.x + player.size, player.y + player.size * 0.15);
    // HgWLineTo(lid, player.x + player.size, player.y - player.size * 0.15);
    double xbody[6] = {
        player.x + player.size * 0.75, player.x - player.size * 0.25,
        player.x - player.size * 0.75, player.x - player.size * 0.75,
        player.x - player.size * 0.25, player.x + player.size * 0.75};
    double ybody[6] = {
        player.y - player.size * 0.08,  player.y - player.size * 0.125,
        player.y - player.size * 0.125, player.y + player.size * 0.125,
        player.y + player.size * 0.125, player.y + player.size * 0.08};
    double xengine[7] = {
        player.x - player.size * 0.25, player.x - player.size * 0.20,
        player.x - player.size * 0.20, player.x - player.size * 0.25,
        player.x - player.size,        player.x - player.size,
        player.x - player.size * 0.75};
    double yleftengine[7] = {
        player.y + player.size * 0.125, player.y + player.size * 0.13,
        player.y + player.size * 0.37,  player.y + player.size * 0.375,
        player.y + player.size * 0.375, player.y + player.size * 0.125,
        player.y + player.size * 0.125};
    double yrightengine[7] = {
        player.y - player.size * 0.125, player.y - player.size * 0.13,
        player.y - player.size * 0.37,  player.y - player.size * 0.375,
        player.y - player.size * 0.375, player.y - player.size * 0.125,
        player.y - player.size * 0.125};
    double xwing[4] = {
        player.x - player.size * 0.25, player.x - player.size * 0.25,
        player.x - player.size * 0.50, player.x - player.size * 0.70};
    double yleftwing[4] = {player.y + player.size * 0.375,
                           player.y + player.size, player.y + player.size,
                           player.y + player.size * 0.375};
    double yrightwing[4] = {player.y - player.size * 0.375,
                            player.y - player.size, player.y - player.size,
                            player.y - player.size * 0.375};

    HgWPolygon(lid, 4, xnose, ynose);
    HgWLines(lid, 6, xbody, ybody);
    HgWLines(lid, 7, xengine, yleftengine);
    HgWLines(lid, 7, xengine, yrightengine);
    HgWLines(lid, 4, xwing, yleftwing);
    HgWLines(lid, 4, xwing, yrightwing);
    HgWMoveTo(lid, player.x - player.size * 0.25, player.y + player.size);
    HgWLineTo(lid, player.x + player.size * 0.60, player.y + player.size);
    HgWMoveTo(lid, player.x - player.size * 0.25, player.y - player.size);
    HgWLineTo(lid, player.x + player.size * 0.60, player.y - player.size);
    HgWSetFillColor(lid, HG_RED);
    HgWFanFill(lid, player.x - player.size, player.y + player.size * 0.25,
               player.size * 0.125, M_PI / 2.0, -M_PI / 2.0, 0);
    HgWFanFill(lid, player.x - player.size, player.y - player.size * 0.25,
               player.size * 0.125, M_PI / 2.0, -M_PI / 2.0, 0);
    // HgWCircle(lid, player.x, player.y, player.size);
}

/*
void draw_enemy : 敵を描画する
引数 | int lid, Character enemy : レイヤーID, 敵の情報
返値 | なし
*/
void draw_enemy(int lid, Character enemy) {
    HgWCircle(lid, enemy.x, enemy.y, enemy.size);
}

/*
Weapon import_beam : ビームに情報を入れる
引数 | Character player, Weapon beam, int count :
プレイヤーの位置情報、値を入れるビーム、上下の配置のカウント 返値 | beam :
情報を入れたビーム
*/
Weapon import_beam(Character player, Weapon beam, int count) {
    double offset = 20.0;
    beam.exist = 1;
    beam.x = player.x + 50.0;
    if (count == 0) {
        beam.y = player.y - offset;
    } else {
        beam.y = player.y + offset;
    }
    return beam;
}

/*
Weapon import_missile : ミサイルに情報を入れる
引数 | Character player, Weapon missile :
プレイヤーの位置情報、値を入れるミサイル 返値 | missile : 情報を入れたビーム
*/
Weapon import_missile(Character player, Weapon missile) {
    switch (missile.type) {
        case 1:
            missile.exist = 1;
            missile.fuel = 10;
            missile.homing = 1;
            missile.velocity = 1.0;
            missile.size = 10.0;
            missile.x = 850.0;
            missile.y = 250.0;
            missile.angle = atan2(player.y - missile.y, player.x - missile.x);
            break;
        case 2:
            missile.exist = 1;
            missile.fuel = 10;
            missile.homing = 1;
            missile.velocity = 1.0;
            missile.size = 10.0;
            missile.x = 850.0;
            missile.y = 350.0;
            missile.angle = atan2(player.y - missile.y, player.x - missile.x);
            // missile.angle = -181.0;
            break;
        case 3:
            missile.exist = 1;
            missile.fuel = 20;
            missile.homing = 1;
            missile.velocity = 1.0;
            missile.size = 20.0;
            missile.x = 900.0;
            missile.y = 300.0;
            missile.angle = atan2(player.y - missile.y, player.x - missile.x);
            break;
    }
    return missile;
}

/*
void draw_beam : ビームを描画する
引数 | int lid, Weapon beam : レイヤーID、ビームの情報
返値 | なし
*/
void draw_beam(int lid, Weapon beam) {
    HgWSetColor(lid, HG_RED);
    HgWMoveTo(lid, beam.x, beam.y);
    HgWLineTo(lid, beam.x - 50, beam.y);
    HgWSetColor(lid, HG_BLACK);
}

/*
Weapon calculate_gamelike_missile : まっすぐ突き進んでくるミサイルの計算
※ミサイルの旋回による減速はanglechangesがこっちにしかないためこっちでやってる
引数 | Character player, Weapon missile : プレイヤーの位置情報、ミサイルの情報
返値 | missile : ミサイルのdxdyが入って返る
*/
Weapon calculate_gamelike_missile(Character player, Weapon missile) {
    if (missile.homing == 1) {
        double tmp = missile.angle;
        double anglechanges;

        missile.angle = atan2(player.y - missile.y, player.x - missile.x);
        // printf("before = %3.2f\nafter = %3.2f\n", tmp / M_PI * 180.0,
        // missile.angle / M_PI * 180.0); printf("fabs = %3.2f\n",
        // fabs(missile.angle - tmp) /M_PI * 180.0);
        if (fabs(missile.angle - tmp) > M_PI) {
            if (missile.angle < 0)
                missile.angle += 2 * M_PI;
            else
                tmp += 2 * M_PI;
        }
        anglechanges = fabs(missile.angle - tmp);
        if (anglechanges > atan2(1, sqrt(3)))
            missile.homing = 0;  //ミサイルのシーカーがロックオンできるのが前方30度
        // printf("homing = %d\n", missile.homing);

        if (fabs((missile.angle - tmp) / M_PI * 180.0) > missile.velocity) {
            // printf("over G\n");
            if (((missile.angle - tmp) / M_PI * 180.0) < 0) {
                // printf("turn right\n");
                missile.angle = tmp - (missile.velocity * M_PI / 180.0);
            } else {
                // printf("turn left\n");
                missile.angle = tmp + (missile.velocity * M_PI / 180.0);
            }
        }
        missile.velocity -= anglechanges * pow(missile.velocity, 2) / 50.0;
        // printf("減速 %2.2f\n", anglechanges * pow(missile.velocity, 2)
        // / 50.0);

        if (missile.angle > M_PI) missile.angle -= 2 * M_PI;
    }

    missile.dx = cos(missile.angle) * missile.velocity;
    missile.dy = sin(missile.angle) * missile.velocity;
    // printf("angle = %3.3f\n", missile.angle / M_PI * 180.0);
    return missile;
}

/*
Weapon calculate_reallike_missile : 先読みしてくるミサイルの計算
※ミサイルの旋回による減速はanglechangesがこっちにしかないためこっちでやってる
※速度はこいつが減速する仕様上追いつけなくなってしまうとforが終わらなくなって
　フリーズするのでハードコードで15.0にしている
引数 | Character player, Weapon missile : プレイヤーの位置情報、ミサイルの情報
返値 | missile : ミサイルのdxdyが入って返る
*/
Weapon calculate_reallike_missile(Character player, Weapon missile) {
    if (missile.homing == 1) {
        double tmp = missile.angle;
        double anglechanges;
        // printf("y = %3.2f\n", player.y);
        for (double t = 0.0;; t += 1.0 / 20.0) {
            // tを求めてブレイク
            // printf("%3.2f\n", missile.velocity * t);
            // printf("%3.2f\n", hypot(enemy.x - (player.x + player.xvector *
            // t), enemy.y - (player.y + player.yvector * t)));
            if (fabs(15.0 * t - hypot(missile.x - (player.x + player.xvector * t), missile.y - (player.y + player.yvector * t))) < 3.0) {
                missile.angle =
                    atan2((player.y + player.yvector * t) - missile.y,
                          (player.x + player.xvector * t) - missile.x);
                // printf("angle = %3.3f\n", missile.angle / M_PI * 180.0);
                break;
            }
        }
        if (fabs(missile.angle - tmp) > M_PI) {
            if (missile.angle < 0)
                missile.angle += 2 * M_PI;
            else
                tmp += 2 * M_PI;
        }
        anglechanges = fabs(missile.angle - tmp);
        if (anglechanges > atan2(sqrt(3), 1))
            missile.homing = 0;  //ミサイルのシーカーがロックオンできるのが前方60度
        // printf("homing = %d\n", missile.homing);

        if (fabs((missile.angle - tmp) / M_PI * 180.0) > missile.velocity) {
            // printf("over G\n");
            if (((missile.angle - tmp) / M_PI * 180.0) < 0) {
                // printf("turn right\n");
                missile.angle = tmp - (missile.velocity * M_PI / 180.0);
            } else {
                // printf("turn left\n");
                missile.angle = tmp + (missile.velocity * M_PI / 180.0);
            }
        }
        missile.velocity -= anglechanges * missile.velocity / 100.0;
        // printf("減速 %2.2f\n", anglechanges * missile.velocity / 100.0);

        if (missile.angle > M_PI) missile.angle -= 2 * M_PI;
    }

    missile.dx = cos(missile.angle) * missile.velocity;
    missile.dy = sin(missile.angle) * missile.velocity;
    return missile;
}

/*
Weapon calculate_longrange_missile : 芋をころすミサイルの計算
※ミサイルの旋回による減速はanglechangesがこっちにしかないためこっちでやってる
※だいたいのロジックはgamelikeと同じだがちょちょっと数字が違う
引数 | Character player, Weapon missile : プレイヤーの位置情報、ミサイルの情報
返値 | missile : ミサイルのdxdyが入って返る
*/
Weapon calculate_longrange_missile(Character player, Weapon missile) {
    if (missile.homing == 1) {
        double tmp = missile.angle;
        double anglechanges;

        missile.angle = atan2(player.y - missile.y, player.x - missile.x);
        // printf("before = %3.2f\nafter = %3.2f\n", tmp / M_PI * 180.0,
        // missile.angle / M_PI * 180.0); printf("fabs = %3.2f\n",
        // fabs(missile.angle - tmp) /M_PI * 180.0);
        if (fabs(missile.angle - tmp) > M_PI) {
            if (missile.angle < 0)
                missile.angle += 2 * M_PI;
            else
                tmp += 2 * M_PI;
        }
        anglechanges = fabs(missile.angle - tmp);
        if (anglechanges > atan2(1, sqrt(3)))
            missile.homing = 0;  //ミサイルのシーカーがロックオンできるのが前方30度
        // printf("homing = %d\n", missile.homing);

        if (fabs((missile.angle - tmp) / M_PI * 180.0) > missile.velocity / 2.0) {
            // printf("over G\n");
            if (((missile.angle - tmp) / M_PI * 180.0) < 0) {
                // printf("turn right\n");
                missile.angle = tmp - (missile.velocity / 2.0 * M_PI / 180.0);
            } else {
                // printf("turn left\n");
                missile.angle = tmp + (missile.velocity / 2.0 * M_PI / 180.0);
            }
        }
        missile.velocity -= anglechanges * pow(missile.velocity, 2) / 50.0;
        // printf("減速 %2.2f\n", anglechanges * pow(missile.velocity, 2)
        // / 50.0);

        if (missile.angle > M_PI) missile.angle -= 2 * M_PI;
    }

    missile.dx = cos(missile.angle) * missile.velocity;
    missile.dy = sin(missile.angle) * missile.velocity;
    // printf("angle = %3.3f\n", missile.angle / M_PI * 180.0);

    return missile;
}

/*
void draw_missile : ビームを描画する
引数 | int lid, Weapon beam : レイヤーID、ビームの情報
返値 | なし
*/
void draw_missile(int lid, Weapon missile) {
    // double tmp[20] = {};
    double xnose[3];
    double ynose[3];

    double xbody[4];
    double ybody[4];

    double xband[4];
    double yband[4];

    double xhigherwing[4];
    double yhigherwing[4];
    double xlowerwing[4];
    double ylowerwing[4];

    // forで入れようとするとどうもうまいこといかんかったのでしゃーなしハードコードが続きます
    xnose[0] = Xrotate(missile.x - missile.size / 2.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    ynose[0] = Yrotate(missile.x - missile.size / 2.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    xnose[1] = Xrotate(missile.x + missile.size, missile.y, missile.x,
                       missile.y, missile.angle);
    ynose[1] = Yrotate(missile.x + missile.size, missile.y, missile.x,
                       missile.y, missile.angle);
    xnose[2] = Xrotate(missile.x - missile.size / 2.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    ynose[2] = Yrotate(missile.x - missile.size / 2.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);

    xbody[0] = Xrotate(missile.x - missile.size * 1.5, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    ybody[0] = Yrotate(missile.x - missile.size * 1.5, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    xbody[1] = Xrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    ybody[1] = Yrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    xbody[2] = Xrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    ybody[2] = Yrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    xbody[3] = Xrotate(missile.x - missile.size * 1.5, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    ybody[3] = Yrotate(missile.x - missile.size * 1.5, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);

    xband[0] = Xrotate(missile.x - missile.size / 2.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    yband[0] = Yrotate(missile.x - missile.size / 2.0, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    xband[1] = Xrotate(missile.x - missile.size * 1.5, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    yband[1] = Yrotate(missile.x - missile.size * 1.5, missile.y + missile.size,
                       missile.x, missile.y, missile.angle);
    xband[2] = Xrotate(missile.x - missile.size * 1.5, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    yband[2] = Yrotate(missile.x - missile.size * 1.5, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    xband[3] = Xrotate(missile.x - missile.size / 2.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);
    yband[3] = Yrotate(missile.x - missile.size / 2.0, missile.y - missile.size,
                       missile.x, missile.y, missile.angle);

    HgWPolygon(lid, 3, xnose, ynose);

    // HgWCircle(lid, xnose[1], ynose[1], 5);
    if (missile.type == 1) {
        HgWSetFillColor(lid, HG_RED);
        HgWPolygonFill(lid, 4, xband, yband, 1);
        xhigherwing[0] =
            Xrotate(missile.x - missile.size * 3.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[0] =
            Yrotate(missile.x - missile.size * 3.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        xhigherwing[1] = Xrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        yhigherwing[1] = Yrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        xhigherwing[2] =
            Xrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[2] =
            Yrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 3, xhigherwing, yhigherwing);
        xlowerwing[0] =
            Xrotate(missile.x - missile.size * 3.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[0] =
            Yrotate(missile.x - missile.size * 3.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        xlowerwing[1] = Xrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        ylowerwing[1] = Yrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        xlowerwing[2] =
            Xrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[2] =
            Yrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 3, xlowerwing, ylowerwing);
    } else if (missile.type == 2) {
        HgWSetFillColor(lid, HG_BLUE);
        HgWPolygonFill(lid, 4, xband, yband, 1);
        xhigherwing[0] =
            Xrotate(missile.x - missile.size * 3.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[0] =
            Yrotate(missile.x - missile.size * 3.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        xhigherwing[1] = Xrotate(missile.x - missile.size * 4.5,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        yhigherwing[1] = Yrotate(missile.x - missile.size * 4.5,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        xhigherwing[2] = Xrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        yhigherwing[2] = Yrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        xhigherwing[3] =
            Xrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[3] =
            Yrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 4, xhigherwing, yhigherwing);
        xlowerwing[0] =
            Xrotate(missile.x - missile.size * 3.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[0] =
            Yrotate(missile.x - missile.size * 3.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        xlowerwing[1] = Xrotate(missile.x - missile.size * 4.5,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        ylowerwing[1] = Yrotate(missile.x - missile.size * 4.5,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        xlowerwing[2] = Xrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        ylowerwing[2] = Yrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        xlowerwing[3] =
            Xrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[3] =
            Yrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 4, xlowerwing, ylowerwing);
    } else {
        HgWSetFillColor(lid, HG_BLACK);
        HgWPolygonFill(lid, 4, xband, yband, 1);
        xhigherwing[0] =
            Xrotate(missile.x - missile.size * 2.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[0] =
            Yrotate(missile.x - missile.size * 2.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        xhigherwing[1] = Xrotate(missile.x - missile.size * 4.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        yhigherwing[1] = Yrotate(missile.x - missile.size * 4.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        xhigherwing[2] = Xrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        yhigherwing[2] = Yrotate(missile.x - missile.size * 5.0,
                                 missile.y + missile.size * 2.0, missile.x,
                                 missile.y, missile.angle);
        xhigherwing[3] =
            Xrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        yhigherwing[3] =
            Yrotate(missile.x - missile.size * 5.0, missile.y + missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 4, xhigherwing, yhigherwing);
        xlowerwing[0] =
            Xrotate(missile.x - missile.size * 2.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[0] =
            Yrotate(missile.x - missile.size * 2.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        xlowerwing[1] = Xrotate(missile.x - missile.size * 4.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        ylowerwing[1] = Yrotate(missile.x - missile.size * 4.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        xlowerwing[2] = Xrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        ylowerwing[2] = Yrotate(missile.x - missile.size * 5.0,
                                missile.y - missile.size * 2.0, missile.x,
                                missile.y, missile.angle);
        xlowerwing[3] =
            Xrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        ylowerwing[3] =
            Yrotate(missile.x - missile.size * 5.0, missile.y - missile.size,
                    missile.x, missile.y, missile.angle);
        HgWLines(lid, 4, xlowerwing, ylowerwing);
    }

    if (missile.fuel > 0) {
        HgWSetFillColor(lid, HG_RED);
        HgWOvalFill(lid,
                    Xrotate(missile.x - missile.size * 5.0, missile.y,
                            missile.x, missile.y, missile.angle),
                    Yrotate(missile.x - missile.size * 5.0, missile.y,
                            missile.x, missile.y, missile.angle),
                    missile.size * 3.5, missile.size, missile.angle, 0);
        HgWSetFillColor(lid, HG_WHITE);
        HgWPolygonFill(
            lid, 4, xbody, ybody,
            1);  // OvalにFanみたく角度指定機能ください！塗りつぶすしかないじゃないですか！
    } else {
        HgWPolygon(lid, 4, xbody, ybody);  // HgRect使えばよかった...
    }
    // HgWCircle(lid, missile.x, missile.y, missile.size);
}

Weapon move_missile(Weapon missile) {
    missile.x += missile.dx;
    missile.y += missile.dy;
    if (missile.fuel > 0) {
        missile.velocity += 2.5;
        missile.fuel -= 1;
    }
    missile.velocity -= pow(missile.velocity, 2) / 400.0;
    // printf("velocity = %2.2f\n", missile.velocity);
    return missile;
}

/*
void draw_hpstats : 自分と敵の体力を描画する
引数 | int lid, Character player, Character enemy :
レイヤーID、自機・敵機の体力情報 返値 | なし
*/
void draw_hpstats(int lid, Character player, Character enemy) {
    int i;
    HgWSetFillColor(lid, HG_YELLOW);
    for (i = 0; i < player.hp; i++) {
        HgWBoxFill(lid, 30, 370 + i * 40, 30, 40, 1);
    }
    for (i = 0; i < enemy.hp; i++) {
        HgWBoxFill(lid, 940, 370 + i * 4, 30, 4, 1);
    }
}

/*
int check_collision : キャラと兵器の衝突判定を作る
引数 | Character character, Weapon weapon : キャラと兵器の座標・サイズ
返値 | 0 = なにもなし、1 = 衝突
*/
int check_collision(Character character, Weapon weapon) {
    if (hypot(character.x - weapon.x, character.y - weapon.y) < character.size + weapon.size)
        return 1;
    return 0;
}

// XYをx0y0中心にangle度回転させた時のX座標
double Xrotate(double x, double y, double x0, double y0, double angle) {
    double distance = hypot(x - x0, y - y0);
    // printf("%3.2f\n", distance);
    double targetangle = atan2(y - y0, x - x0);

    x = x0 + distance * cos(angle + targetangle);

    return x;
}

double Yrotate(double x, double y, double x0, double y0, double angle) {
    double distance = hypot(x - x0, y - y0);
    double targetangle = atan2(y - y0, x - x0);

    y = y0 + distance * sin(angle + targetangle);

    return y;
}

/*
void draw_ending : リザルトを描画する
引数 | hp : 自機か敵機のHP 今回は敵機のを引っ張ってくる
            ※同時に死んだ場合のデバッグはできてません
返値 | なし
*/
void draw_ending(int hp) {
    HgClear();
    if (hp <= 0) {
        HgText(420, 300, "あなたの勝ちです！");
    } else {
        HgText(420, 300, "あなたの負けです...");
    }
    HgText(400, 200, "press space key to close");
    for (int key = 0;;) {
        key = HgGetChar();
        if (key == ' ') break;
    }
    HgClose();
}

/*-------- function detail end --------*/


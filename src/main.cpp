#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <functional>
#include <queue>
#include <condition_variable>
#include <cassert>
#include <climits>
#include "SmallBullet.h"
#include "Direction.h"
#include "Player.h"
#include "EnemyBig.h"
#include "BigBullet.h"
#include "EnemySmall.h"

static const std::chrono::milliseconds frame_durtion(40); // 40 FPS
static const std::chrono::milliseconds t_between_big_enemies(12000); // new big enemy every 8 seconds
static const std::chrono::milliseconds t_between_small_enemies(4000); // new small enemy every 4 seconds
static const int t_big_enemies_bullets = 4000;
static const int t_small_enemies_bullets = 500;
static const int small_bullets_speed = 30; // rows per second
static const int big_bullets_speed = 15; //rows per sec_ond
static const int big_slow_enemy_speed = 8; // columns per second
static const int small_fast_enemy_speed = 15; // columns per second
static const int SPACE = 32;
static std::atomic_bool exit_condition(false);
static std::atomic_bool game_over(false);

static std::uniform_int_distribution<int> distribution(1, 100);
static int POINTS = 0;
static int BIG_SHIPS_DESTROYED = 0;
static int SMALL_SHIPS_DESTROYED = 0;

/// Bullets' vector
static std::vector<BigBullet *> big_bullets_vector;
static std::vector<SmallBullet *> small_bullets_vector;
static std::vector<SmallBullet *> player_bullets_vector;

/// Vector przeciwników aktywnych
static std::vector<EnemyBig *> big_slow_enemies_vector;
static std::vector<EnemySmall *> small_fast_enemies_vector;

/// Kolejka przeciwników
static std::queue<EnemyBig *> new_big_slow_enemies_queue;
static std::queue<EnemySmall *> new_small_fast_enemies_queue;

/// Kolejka nabojów
static std::queue<BigBullet *> new_big_bullets_queue;
static std::queue<SmallBullet *> new_small_bullets_queue;

/// Kolejka losowych liczb całkowitych typu unsigned short
/// generowanych z pliku /dev/urandom.
static std::queue<unsigned short> urandom_values_queue;

/// Vector wątków
static std::vector<std::thread> threads_small_enemies_vector;
static std::vector<std::thread> threads_big_enemies_vector;


/// Mother

/// Mutexes
static std::mutex player_bullets_mutex;
static std::mutex small_bullets_mutex;
static std::mutex big_bullets_mutex;
static std::mutex big_enemies_mutex;
static std::mutex small_enemies_mutex;
static std::mutex player_mutex;
static std::mutex ncurses_mutex;
static std::mutex random_numbers_queue_condition_var_mutex;

static std::mutex new_big_enemy_mutex;
static std::mutex new_big_adder_enemy_mutex;
static std::mutex new_small_enemy_mutex;
static std::mutex new_small_adder_enemy_mutex;

static std::mutex new_small_bullet_mutex;
static std::mutex new_big_bullet_mutex;
static std::mutex new_small_adder_bullet_mutex;
static std::mutex new_big_adder_bullet_mutex;


/// Condition variables
static std::condition_variable random_numbers_queue_condition_variable;

static std::condition_variable new_big_enemy_condition_variable;
static std::condition_variable new_small_enemy_condition_variable;

static std::condition_variable new_big_bullet_condition_variable;
static std::condition_variable new_small_bullet_condition_variable;


int originalHealth;
/// Colors' modes
static const short MODE_GREEN = 1;
static const short MODE_RED = 2;

void add_big_blue_enemy_to_active_game();
bool isHit(GameActor *bullet, GameActor *actor);

void handle_bullet_hits(Player &player);

void remove_destroyed_enemies();

void remove_used_bullets();

void draw_bullets();

void shoot_small_bullets();

void player_shoots(GameActor &player);

void draw_enemies();

void draw_health(Player &player);

/// Big enemies functions
void move_big_slow_enemies();

void create_big_slow_enemies_bullets();

void shoot_big_bullets();


void create_big_enemy();

/// Small enemies functions
void move_small_fast_enemies();

void create_small_fast_enemies_bullets();

void create_small_enemy();

void urandom_int_generator();

void add_big_enemy_to_active_game();

void add_small_enemy_to_active_game();

void add_small_green_enemy_to_active_game();




/// Main view rendering function and game loop
/**
 * A method to be executed in a separate thread. Used to refresh the view.
 * @param exit exit condition
 * @param player a reference to player object
 */
void refresh_view(Player &player) {

    int row = getmaxy(stdscr) / 2 - 2;
    int col = getmaxx(stdscr) / 2 - 8;

    /// Utworzenie wątku generującego całkowite liczby losowe typu
    /// unsigned short (zakres: 1-100) pochodzące z dev/urandom.
    std::thread urandom_int_creation_thread(urandom_int_generator);

    /// matka produkująca nowe duże statki
    std::thread mother_big_enemies_thread(create_big_enemy);

    /// matka produkująca nowe małe statki
    std::thread mother_small_enemies_thread(create_small_enemy);

    /// wątek nasluchujący listę nowych dużych statków i dodający je do gry
    std::thread big_enemies_creation_thread(add_big_enemy_to_active_game);

    /// wątek nasluchujący listę nowych dużych statków i dodający je do gry
    std::thread big_blue_enemies_creation_thread(add_big_blue_enemy_to_active_game);

    /// wątek nasluchujący listę nowych małych statków i dodający je do gry
    std::thread small_enemies_creation_thread(add_small_enemy_to_active_game);

    /// wątek nasluchujący listę nowych małych statków i dodający je do gry
    std::thread small_green_enemies_creation_thread(add_small_green_enemy_to_active_game);

    /// Launch big enemies movement thread
    std::thread move_big_slow_enemies_thread(move_big_slow_enemies);

    /// Launch small enemies movement thread
    std::thread move_small_fast_enemies_thread(move_small_fast_enemies);

    /// Launch small fast enemies shooting thread
    //std::thread small_fast_enemies_shooting_thread(add_small_bullet_to_active_game);

    /// Launch small bullets shooting thread
    std::thread small_bullets_thread(shoot_small_bullets);


    /// Launch big bullets shooting thread
    std::thread big_bullets_thread(shoot_big_bullets);

    /// matka produkująca nowe duże pociski kurwa mac a nie statki
    std::thread support_big_bullets_creator_thread(create_big_slow_enemies_bullets);

    /// matka produkująca nowe małe statki
    std::thread support_small_bullets_creator_thread(create_small_fast_enemies_bullets);

    while (!exit_condition) {
        clear();
        attron(A_BOLD);
        player_mutex.lock();
        ncurses_mutex.lock();
        player.drawActor();
        ncurses_mutex.unlock();
        player_mutex.unlock();
        draw_enemies();
        attroff(A_BOLD);
        remove_used_bullets();
        handle_bullet_hits(player);
        if (player.isDone()) {
            game_over = true;
        }
        remove_destroyed_enemies();

        draw_health(player);
        mvprintw(1, 0, "Zestrzelone duze statki: %d", BIG_SHIPS_DESTROYED);
        mvprintw(2, 0, "Zestrzelone male statki: %d", SMALL_SHIPS_DESTROYED);
        mvprintw(3, 0, "Ilosc punktow: %d", POINTS);
        draw_bullets();

        refresh();

        if (game_over) {
            clear();
            exit_condition = true;
            attron(A_BOLD);
            attron(COLOR_PAIR(MODE_RED));
            mvprintw(row, col, "KONIEC GRY!");
            mvprintw(row + 1, col, "Zestrzelone duze statki: %d", BIG_SHIPS_DESTROYED);
            mvprintw(row + 2, col, "Zestrzelone maze statki: %d", SMALL_SHIPS_DESTROYED);
            mvprintw(row + 3, col, "Ilosc punktow: %d", POINTS);
            attroff(COLOR_PAIR(MODE_RED));
            attroff(A_BOLD);
            refresh();
            break;
        } else {
            std::this_thread::sleep_for(frame_durtion);
        }
    }
    refresh();
    game_over = true;
    mvprintw(row + 4, col, "Konczenie watkow...");
    refresh();

    urandom_int_creation_thread.join();

    /// matka produkująca nowe duże statki
    mother_big_enemies_thread.join();

    /// matka produkująca nowe małe statki
    mother_small_enemies_thread.join();

    /// wątek nasluchujący listę nowych dużych statków i dodający je do gry
    big_enemies_creation_thread.join();

    /// wątek nasluchujący listę nowych dużych statków i dodający je do gry
    big_blue_enemies_creation_thread.join();

    /// wątek nasluchujący listę nowych małych statków i dodający je do gry
    small_enemies_creation_thread.join();

    /// wątek nasluchujący listę nowych małych statków i dodający je do gry
    small_green_enemies_creation_thread.join();

    /// Launch big enemies movement thread
    move_big_slow_enemies_thread.join();

    /// Launch small enemies movement thread
    move_small_fast_enemies_thread.join();

    /// Launch small fast enemies shooting thread
    //small_fast_enemies_shooting_thread.join();

    /// Launch small bullets shooting thread
    small_bullets_thread.join();

    /// Launch big slow enemies shooting thread
    //big_slow_enemies_shooting_thread.join();
    /// Launch big bullets shooting thread
    big_bullets_thread.join();

    /// matka produkująca nowe duże statki
    support_big_bullets_creator_thread.join();

    /// matka produkująca nowe małe statki
    support_small_bullets_creator_thread.join();

    for (auto &i : threads_small_enemies_vector) {
        i.join();
    }
    for (auto &i : threads_big_enemies_vector) {
        i.join();
    }
//    urandom_int_creation_thread.join();
//    mvprintw(row + 5, col, "- urandom integers creation thread: FINISHED");
//    mother_big_enemies_thread.join();
//    mvprintw(row + 16, col, "- mother big enemies creation thread: FINISHED");
//    mother_small_enemies_thread.join();
//    mvprintw(row + 17, col, "- mother small enemies creation thread: FINISHED");
//    big_enemies_creation_thread.join();
//    mvprintw(row + 6, col, "- big enemies creation thread: FINISHED");
//    small_enemies_creation_thread.join();
//    mvprintw(row + 7, col, "- small enemies creation thread: FINISHED");
//    move_big_slow_enemies_thread.join();
//    mvprintw(row + 8, col, "- big enemies motion thread: FINISHED");
//    move_small_fast_enemies_thread.join();
//    mvprintw(row + 9, col, "- small enemies motion thread: FINISHED");
//    big_slow_enemies_shooting_thread.join();
//    mvprintw(row + 10, col, "- big enemies shooting thread: FINISHED");
//    small_fast_enemies_shooting_thread.join();
//    mvprintw(row + 11, col, "- small enemies shooting thread: FINISHED");
//    big_bullets_thread.join();
//    mvprintw(row + 12, col, "- big bullets motion thread: FINISHED");
//    small_bullets_thread.join();
//    mvprintw(row + 13, col, "- small bullets motion thread: FINISHED");
    mvprintw(row + 14, col, "Finished all tasks!");
    mvprintw(row + 15, col, "Press 'q' to quit...");
    refresh();
}
//////////////////////////////////////////////

/// Funkcja generująca liczby całkowite z zakresu 0 - ULLONX_MAX pochodzące
/// z pliku /dev/urandom. Liczby są skalowane do zakresu 0 - 100 oraz castowane
/// na typ unsigned short.
void urandom_int_generator() {
    unsigned long long int random_value = 0;
    size_t size = sizeof(random_value);
    while (!game_over) {
        std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
        if (urandom) {
            urandom.read(reinterpret_cast<char *>(&random_value), size);
            if (urandom) {
                std::unique_lock<std::mutex> locker(random_numbers_queue_condition_var_mutex);
                unsigned short random_casted_value = static_cast<unsigned short &&>(random_value / (ULLONG_MAX / 100));

                if (random_casted_value == 0) {
                    random_casted_value = 1;
                }

                urandom_values_queue.push(random_casted_value);
                random_numbers_queue_condition_variable.notify_one();
                locker.unlock();
            }
            urandom.close();
        }
    }
}

/// Funkcja pobierająca liczbę losową z kolejki. Funkcja zawarta
/// jest w obszarze synchronizacji za pomocą std::condition_variable.
unsigned short get_random_number() {
    std::unique_lock<std::mutex> locker(random_numbers_queue_condition_var_mutex);
    random_numbers_queue_condition_variable.wait(locker, [] { return !urandom_values_queue.empty(); });
    assert(!urandom_values_queue.empty());
    unsigned short random_short = urandom_values_queue.front();
    urandom_values_queue.pop();
    locker.unlock();

    return random_short;
}

void add_big_enemy_to_active_game() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(new_big_adder_enemy_mutex);
        new_big_enemy_condition_variable.wait(locker, [] { return !new_big_slow_enemies_queue.empty(); });
        assert(!new_big_slow_enemies_queue.empty());

        big_slow_enemies_vector.push_back(new_big_slow_enemies_queue.front());

        new_big_slow_enemies_queue.pop();

        std::thread thread_enemy= big_slow_enemies_vector[big_slow_enemies_vector.size()-1]->startThread();
        threads_big_enemies_vector.push_back(std::move(thread_enemy));


        locker.unlock();
    }
}

void add_big_blue_enemy_to_active_game() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(new_big_adder_enemy_mutex);
        new_big_enemy_condition_variable.wait(locker, [] { return !new_big_slow_enemies_queue.empty(); });
        assert(!new_big_slow_enemies_queue.empty());
        EnemyBig * enemyBig = new_big_slow_enemies_queue.front();
        enemyBig->setIsBlue(true);
        big_slow_enemies_vector.push_back(enemyBig);

        new_big_slow_enemies_queue.pop();

        std::thread thread_enemy= big_slow_enemies_vector[big_slow_enemies_vector.size()-1]->startThread();
        threads_big_enemies_vector.push_back(std::move(thread_enemy));


        locker.unlock();
    }
}

void add_small_enemy_to_active_game() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(new_small_adder_enemy_mutex);
        new_small_enemy_condition_variable.wait(locker, [] { return !new_small_fast_enemies_queue.empty(); });
        assert(!new_small_fast_enemies_queue.empty());
        small_fast_enemies_vector.push_back(new_small_fast_enemies_queue.front());
        new_small_fast_enemies_queue.pop();
        std::thread thread_enemy= small_fast_enemies_vector[small_fast_enemies_vector.size()-1]->startThread();
        threads_small_enemies_vector.push_back(std::move(thread_enemy));
        locker.unlock();
    }

}

void add_small_green_enemy_to_active_game() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(new_small_adder_enemy_mutex);
        new_small_enemy_condition_variable.wait(locker, [] { return !new_small_fast_enemies_queue.empty(); });
        assert(!new_small_fast_enemies_queue.empty());
        EnemySmall * enemySmall = new_small_fast_enemies_queue.front();
        enemySmall->setIsGreen(true);
        small_fast_enemies_vector.push_back(enemySmall);
        new_small_fast_enemies_queue.pop();
        std::thread thread_enemy= small_fast_enemies_vector[small_fast_enemies_vector.size()-1]->startThread();
        threads_small_enemies_vector.push_back(std::move(thread_enemy));
        locker.unlock();
    }

}

bool isHit(GameActor *bullet, GameActor *actor) {
    int bullet_x = bullet->getPos_x();
    int bullet_y = bullet->getPos_y();
    int bullet_w = bullet->getWidth();
    int bullet_h = bullet->getHeight();
    int actor_x_min = actor->getPos_x();
    int actor_x_max = actor_x_min + actor->getWidth();
    int actor_y_min = actor->getPos_y();
    int actor_y_max = actor_y_min + actor->getHeight();

    actor_x_min -= bullet_w;
    actor_y_min -= bullet_h;

    return bullet_x > actor_x_min
           && bullet_x < actor_x_max
           && bullet_y > actor_y_min
           && bullet_y < actor_y_max;
}

void handle_bullet_hits(Player &player) {
    small_bullets_mutex.lock();
    for (SmallBullet *bullet : small_bullets_vector) {
        if (isHit(bullet, &player)) {
            bullet->setDone();
            player.setDamage(1);
        }
    }
    small_bullets_mutex.unlock();

    big_bullets_mutex.lock();
    for (BigBullet *bullet : big_bullets_vector) {
        if (isHit(bullet, &player)) {
            bullet->setDone();
            player.setDamage(5);
        }
    }
    big_bullets_mutex.unlock();

    player_bullets_mutex.lock();
    for (SmallBullet *bullet : player_bullets_vector) {
        big_enemies_mutex.lock();
        for (EnemyBig *enemy : big_slow_enemies_vector) {
            if (isHit(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                if (enemy->isDone()) {
                    BIG_SHIPS_DESTROYED++;
                }
                POINTS++;
            }
        }
        big_enemies_mutex.unlock();
        small_enemies_mutex.lock();
        for (EnemySmall *enemy : small_fast_enemies_vector) {
            if (isHit(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                SMALL_SHIPS_DESTROYED++;
                POINTS++;
            }
        }
        small_enemies_mutex.unlock();
    }
    player_bullets_mutex.unlock();
}

void remove_destroyed_enemies() {
    big_enemies_mutex.lock();
    if (!big_slow_enemies_vector.empty()) {
        auto it = big_slow_enemies_vector.begin();
        int j = 0;
        while (it != big_slow_enemies_vector.end()) {
            if (big_slow_enemies_vector[j]->isDone()) {
                big_slow_enemies_vector[j]->setDied(true);
                it = big_slow_enemies_vector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    if (!small_fast_enemies_vector.empty()) {
        auto it = small_fast_enemies_vector.begin();
        int j = 0;
        while (it != small_fast_enemies_vector.end()) {
            if (small_fast_enemies_vector[j]->isDone()) {
                small_fast_enemies_vector[j]->setDied(true);
                it = small_fast_enemies_vector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    small_enemies_mutex.unlock();
}

/**
 * Removes the bullets, which have reached their destination,
 * from the player_bullets.
 * Joins the threads connected with those bullets and removes
 * them from the player_bullet_threads_vector as well.
 *
 * Contains bullets_vector_mutex critical section
 */
void remove_used_bullets() {
    small_bullets_mutex.lock(); // Critical section - erasing data from the small bullets vectors
    if (!small_bullets_vector.empty()) {
        auto it = small_bullets_vector.begin();
        int j = 0;
        while (it != small_bullets_vector.end()) {
            if (small_bullets_vector[j]->isDone()) {
                it = small_bullets_vector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    small_bullets_mutex.unlock(); // End of critical section

    big_bullets_mutex.lock(); // Critical section - erasing data from the small bullets vectors
    if (!big_bullets_vector.empty()) {
        auto it = big_bullets_vector.begin();
        int j = 0;
        while (it != big_bullets_vector.end()) {
            if (big_bullets_vector[j]->isDone()) {
                it = big_bullets_vector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    big_bullets_mutex.unlock(); // End of critical section

    player_bullets_mutex.lock(); // Critical section - erasing data from the player bullets vectors
    if (!player_bullets_vector.empty()) {
        auto it = player_bullets_vector.begin();
        int j = 0;
        while (it != player_bullets_vector.end()) {
            if (player_bullets_vector[j]->isDone()) {
                it = player_bullets_vector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    player_bullets_mutex.unlock(); // End of critical section
}

/**
 * Prints the bullets shot by the player
 */
void draw_bullets() {
    for (SmallBullet *bullet : small_bullets_vector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(MODE_RED));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(MODE_RED));
            }
            attroff(A_BOLD);
        }
    }
    for (SmallBullet *bullet : player_bullets_vector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(MODE_GREEN));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(MODE_GREEN));
            }
            attroff(A_BOLD);
        }
    }
    for (BigBullet *bullet : big_bullets_vector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(MODE_RED));
            }
            ncurses_mutex.lock();
            bullet->drawActor();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(MODE_RED));
            }
            attroff(A_BOLD);
        }
    }
}

/**
 * Shoot the bullet on a vertical course
 * @param bullet the bullet to be shot
 * @param dir the direction, "UP" or "DOWN"
 * @param speed the bullet's speed in rows per second
 */
void shoot_small_bullets() {
    int milis_per_row = 1000 / small_bullets_speed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!game_over) {
        small_bullets_mutex.lock();
        for (SmallBullet *bullet : small_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, 1);
            }
        }
        small_bullets_mutex.unlock();
        player_bullets_mutex.lock();
        for (SmallBullet *bullet : player_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, -1);
            }
        }
        player_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}

/**
 * Creates bullets to be shot by specified player
 * and shoots them by launching threads for them.
 *
 * Contains bullets_vector_mutex critical section
 * @param player the player who shoots
 */
void player_shoots(GameActor &player) {
    // Create the bullet
    SmallBullet *bullet = new SmallBullet(short(player.getPos_x() + player.getWidth() / 2), short(player.getPos_y()), 0,
                                          getmaxx(stdscr), 0,
                                          player.getPos_y());
    bullet->move_direction = UP;
    // Shoot the bullets
    player_bullets_mutex.lock(); // Critical section - adding data to the small bullets vectors
    player_bullets_vector.push_back(bullet);
    player_bullets_mutex.unlock(); // End of critical section
}

/**
 * Draws the enemies on the screen
 *
 * Contains a enemies_mutex critical section
 */
void draw_enemies() {
    big_enemies_mutex.lock();
    for (GameActor *enemy : big_slow_enemies_vector) {
        ncurses_mutex.lock();
        enemy->drawActor();
        ncurses_mutex.unlock();
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    for (GameActor *enemy : small_fast_enemies_vector) {
        ncurses_mutex.lock();
        enemy->drawActor();
        ncurses_mutex.unlock();
    }
    small_enemies_mutex.unlock();
};

void draw_health(Player &player) {
    int hp = (player.getHit_points() * 100) / originalHealth;
    std::string s = std::to_string(hp);
    s = s + "%%";
    char const *pchar = s.c_str();
    mvprintw(0, 0, "HEALTH: ");
    if (has_colors()) {
        attron(COLOR_PAIR(MODE_GREEN));
    }
    mvprintw(0, 8, pchar);
    if (has_colors()) {
        attroff(COLOR_PAIR(MODE_GREEN));
    }
}

/// Big enemies functions
/**
 * Changes the coordinates of the big slow enemies.
 * They go from left to right, or right to left. When they reach the wall,
 * they go down one row. With 1% probbility they can change the route unexpextedly and
 * go down one row. When they reach the bottom of the screen, the game is over.
 */
void move_big_slow_enemies() {
    int milis_per_column = 1000 / big_slow_enemy_speed;
    std::chrono::milliseconds t_col(milis_per_column);
    while (!game_over) {
        for (auto enemy : big_slow_enemies_vector) {

            unsigned short random_short = get_random_number();
            if (random_short > 98) {
                enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                enemy->move(0, 1);
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = LEFT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
            if (enemy->move_direction == LEFT) {
                if (enemy->getPos_x() > enemy->getMin_x()) {
                    enemy->move(-1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = RIGHT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}

/**
 * Shoot the bullet on a vertical course
 * @param bullet the bullet to be shot
 * @param dir the direction, "UP" or "DOWN"
 * @param speed the bullet's speed in rows per second
 */
void shoot_big_bullets() {
    int milis_per_row = 1000 / big_bullets_speed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!game_over) {
        big_bullets_mutex.lock();
        for (BigBullet *bullet : big_bullets_vector) {
            if (!bullet->isDone()) {
                bullet->move(0, bullet->move_direction == DOWN ? short(1) : short(-1));
            }
        }
        big_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}


/**
 *
 */
void create_big_slow_enemies_bullets() {
    // Create the bullets
    while (!game_over) {
        auto *bullet = new BigBullet();
        bullet->move_direction = DOWN;
        // Shoot the bullets
        std::unique_lock<std::mutex> locker(new_big_bullet_mutex);
        new_big_bullets_queue.push(bullet);
        new_big_bullet_condition_variable.notify_one();
        locker.unlock();

        auto random_short = static_cast<unsigned int>(get_random_number() * 100);
        static const std::chrono::milliseconds t_between_creation_big_bullets(random_short);

        std::this_thread::sleep_for(t_between_creation_big_bullets);
    }
}

void create_small_fast_enemies_bullets() {
    while (!game_over) {
        auto *bullet = new SmallBullet();
        bullet->move_direction = DOWN;
        // Shoot the bullets
        std::unique_lock<std::mutex> locker(new_small_bullet_mutex);
        new_small_bullets_queue.push(bullet);
        new_small_bullet_condition_variable.notify_one();
        locker.unlock();

        auto random_short = static_cast<unsigned int>(get_random_number() * 100);
        static const std::chrono::milliseconds t_between_creation_big_bullets(random_short/5);

        std::this_thread::sleep_for(t_between_creation_big_bullets);
    }
    }


///// Small enemies functions
///**
// *
// * @param enemy
// */
//void small_fast_enemy_shoots(EnemySmall &enemy) {
//    // Create the bullets
//    auto *bullet = new SmallBullet(short(enemy.getPos_x() + enemy.getWidth() / 2), short(enemy.getPos_y()), 0,
//                                   getmaxx(stdscr), 0,
//                                   getmaxy(stdscr));
//    bullet->move_direction = DOWN;
//    // Shoot the bullets
//    std::unique_lock<std::mutex> locker(new_small_bullet_mutex);
//    new_small_bullets_queue.push(bullet);
//    new_small_bullet_condition_variable.notify_one();
//    locker.unlock();
//}
//
///**
// *
// */

//}
/**
 *
 */
// TODO: TU BYLA ZMIANA
void create_big_enemy() {
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);
    while (!game_over) {
        unsigned short random_short = get_random_number();
        auto *enemy_big_slow = new EnemyBig(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                            new_big_adder_bullet_mutex, new_big_bullet_condition_variable, game_over, new_big_bullets_queue,
                                            big_bullets_vector);



        enemy_big_slow->move_direction = RIGHT;
        std::unique_lock<std::mutex> locker(new_big_enemy_mutex);
        new_big_slow_enemies_queue.push(enemy_big_slow);
        new_big_enemy_condition_variable.notify_one();
        locker.unlock();
        std::this_thread::sleep_for(t_between_big_enemies);
    }
}

/**
 *
 */
void create_small_enemy() {
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);
    while (!game_over) {
        unsigned short random_short = get_random_number();
        auto enemy_small_fast = new EnemySmall(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,new_small_adder_bullet_mutex,
                new_small_bullet_condition_variable, game_over, new_small_bullets_queue,
                                               small_bullets_vector);
        enemy_small_fast->move_direction = LEFT;
        std::unique_lock<std::mutex> locker(new_small_enemy_mutex);
        new_small_fast_enemies_queue.push(enemy_small_fast);
        new_small_enemy_condition_variable.notify_one();
        locker.unlock();
        std::this_thread::sleep_for(t_between_small_enemies);
    }
}


void move_small_fast_enemies() {
    int milis_per_column = 1000 / small_fast_enemy_speed;
    std::chrono::milliseconds t_col(milis_per_column);
    while (!game_over) {
        for (auto enemy : small_fast_enemies_vector) {

            unsigned short random_short = get_random_number();
            if (random_short > 95) {
                enemy->move_direction = enemy->move_direction == RIGHT ? LEFT : RIGHT;
                enemy->move(0, 1);
            }
            if (enemy->move_direction == RIGHT) {
                if (enemy->getPos_x() + enemy->getWidth() < enemy->getMax_x()) {
                    enemy->move(1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = LEFT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
            if (enemy->move_direction == LEFT) {
                if (enemy->getPos_x() > enemy->getMin_x()) {
                    enemy->move(-1, 0);
                } else {
                    enemy->move(0, 1);
                    enemy->move_direction = RIGHT;
                }
                if (enemy->getPos_y() + enemy->getHeight() == enemy->getMax_y()) {
                    game_over = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}


///////////////////////////////////////////////////////////

int main() {

    /// Initialize ncurses
    initscr();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();

    if (has_colors()) {
        start_color();
        init_pair(MODE_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(MODE_RED, COLOR_RED, COLOR_BLACK);
    }
    /// Create player and shield
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);

    while (true) {
        if (has_colors()) attron(COLOR_PAIR(MODE_RED));
        attron(A_BOLD);
        mvprintw(stdscr_maxy / 2 - 4, stdscr_maxx / 2 - 7, "SPACE INVADERS");
        attroff(A_BOLD);
        if (has_colors()) attroff(COLOR_PAIR(MODE_RED));
        mvprintw(stdscr_maxy / 2 - 1, stdscr_maxx / 2 - 14, "Statek przusuwa sie w znakami 'a' lub 'd'");
        mvprintw(stdscr_maxy / 2, stdscr_maxx / 2 - 14, "Strzaly oddaje sie spacja");
        mvprintw(stdscr_maxy / 2 + 1, stdscr_maxx / 2 - 14, "Gra konczy sie kiedy zycie spadnie do 0%");
        mvprintw(stdscr_maxy / 2 + 2, stdscr_maxx / 2 - 14, "lub wrogi statek zejdzie na sam dol mapy");
        mvprintw(stdscr_maxy / 2 + 3, stdscr_maxx / 2 - 14, "Przycisk 'q' konczy gre");
        int c = getch();
        if (c != ERR) {
            if (c == 'q') {
                clear();
                refresh();
                return 0;
            }
            break;
        }
    }

    auto *player = new Player(stdscr_maxx / 2 - 3, stdscr_maxy - 1, 0, stdscr_maxx, 0, stdscr_maxy);
    originalHealth = player->getHit_points();
    /// Launch view refresh thread
    std::thread refresh_thread(refresh_view, std::ref(*player));

    while (true) {
        int key = getch();
        if (key == 'q') {
            exit_condition = true;
            break;
        }

        if (key == SPACE) {
            player_shoots(*player);
        }

        if (key == 'a') {
            /// Move player left
            player_mutex.lock();
            player->move(-1, 0);
            player_mutex.unlock();
        }

        if (key == 'd') {
            /// Move player right
            player_mutex.lock();
            player->move(1, 0);
            player_mutex.unlock();
        }

    }
    refresh_thread.join();
    endwin();
    return 0;
}
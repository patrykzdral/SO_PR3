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
#include "BigAlienShip.h"
#include "BigBullet.h"
#include "SmallAlienShip.h"

static const std::chrono::milliseconds frameDuration(40);
static const std::chrono::milliseconds timeBetweenNewBigAlienShips(9000);
static const std::chrono::milliseconds timeBetweenNewSmallAlienShips(5000);

/// Prędkość pocisków (wiersze na sekundę)
static const int smallBulletsSpeed = 25;
static const int bigBulletsSpeed = 18; //rows per sec_ond

/// Prędkość statków kosmitów (kolumny na sekundę)
static const int bigAlienShipsSpeed = 10; // columns per second
static const int smallAlienShipsSpeed = 20; // columns per second

static const int SPACE = 32;

/// Atomics - warunkujące zakończenie rozgrywki i koniec programu
static std::atomic_bool isExitTime(false);
static std::atomic_bool isGameOverTime(false);

/// Wektory aktywnych na mapie statków kosmitów
static std::vector<BigAlienShip *> bigActiveAlienShipsVector;
static std::vector<SmallAlienShip *> smallActiveAlienShipsVector;

/// Kolejka nadciągających statków kosmitów
static std::queue<BigAlienShip *> newBigAlienShipsQueue;
static std::queue<SmallAlienShip *> newSmallAlienShipsQueue;

/// Wektory aktywnych na mapie pocisków
static std::vector<BigBullet *> bigActiveBulletsVector;
static std::vector<SmallBullet *> smallActiveBulletsVector;
static std::vector<SmallBullet *> playerActiveBulletsVector;

/// Kolejka przygotowanych dla kosmitów pocisków
static std::queue<BigBullet *> newBigBulletsQueue;
static std::queue<SmallBullet *> newSmallBulletsQueue;

/// Kolejka losowych liczb całkowitych typu unsigned short
/// generowanych z pliku /dev/urandom.
static std::queue<unsigned short> newUrandomValuesQueue;

/// Statystyki
static int earnedPoints = 0;
static int destroyedBigAlienShips = 0;
static int destroyedSmallAlienShips = 0;

/// Wektor wątków statków kosmitycznych przechwytujących naboje.
static std::vector<std::thread> threads_enemies_vector;

/// Mutexy
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

static std::mutex new_small_adder_bullet_mutex;
static std::mutex new_big_adder_bullet_mutex;

/// Condition variables
static std::condition_variable random_numbers_queue_condition_variable;

static std::condition_variable new_big_enemy_condition_variable;
static std::condition_variable new_small_enemy_condition_variable;

static std::condition_variable new_big_bullet_condition_variable;
static std::condition_variable new_small_bullet_condition_variable;

int health;

/// Prototypy funkcji
bool isCollision(GameObject *bullet, GameObject *actor);

void playerBullets(GameObject &player);

void drawAlienShips();

void drawHealth(Player &player);

void drawBullets();

void handleBulletsHits(Player &player);

void removeDestroyedAlienShips();

void removeUsedBullets();

void moveSmallBullets();

void moveBigBullets();

void moveBigAlienShips();

void moveSmallAlienShips();

void createBigBullets();

void createSmallBullets();

void createBigAlienShip();

void createSmallAlienShip();

void addBigAlienShipToActiveGame();

void addSmallAlienShipToActiveGame();

void urandomIntGenerator();


void runApp(Player &player) {

    int row = getmaxy(stdscr) / 2 - 2;
    int col = getmaxx(stdscr) / 2 - 8;

    /// Utworzenie wątku generującego całkowite liczby losowe typu
    /// unsigned short (zakres: 1-100) pochodzące z dev/urandom.
    std::thread urandomIntCreationThread(urandomIntGenerator);

    /// Matka produkująca nowe duże statki kosmitów
    std::thread motherBigAlienShipsThread(createBigAlienShip);

    /// Matka produkująca nowe małe statki kosmitów
    std::thread motherSmallAlienShipsThread(createSmallAlienShip);

    /// Wątek nasłuchujący wektor nowych dużych statków i dodający je do gry
    std::thread addNewBigAlienShipsToActiveGameThread(addBigAlienShipToActiveGame);

    /// Wątek nasłuchujący wektor nowych małych statków i dodający je do gry
    std::thread addSmallAlienShipsToActiveGameThread(addSmallAlienShipToActiveGame);

    /// Wątek odpowiadający za poruszanie się dużych statków kosmitów
    std::thread moveBigAlienShipsThread(moveBigAlienShips);

    /// Wątek odpowiadający za poruszanie się małych statków kosmitów
    std::thread moveSmallAlienShipsThread(moveSmallAlienShips);

    /// Wątek odpowiadający za tor 'lotu' pocisków małych statków kosmitów
    std::thread moveSmallAlienShipsBulletsThread(moveSmallBullets);

    /// Wątek odpowiadający za tor 'lotu' pocisków dużych statków kosmitów
    std::thread moveBigAlienShipsBulletsThread(moveBigBullets);

    /// Matka produkująca nowe duże pociski
    std::thread supportBigBulletsCreatorThread(createBigBullets);

    /// Matka produkująca nowe małe pociski
    std::thread supportSmallBulletsCreatorThread(createSmallBullets);

    while (!isExitTime) {
        clear();
        attron(A_BOLD);
        player_mutex.lock();
        ncurses_mutex.lock();
        player.drawObject();
        ncurses_mutex.unlock();
        player_mutex.unlock();
        drawAlienShips();
        attroff(A_BOLD);

        removeUsedBullets();
        handleBulletsHits(player);
        if (player.isDone()) {
            isGameOverTime = true;
        }
        removeDestroyedAlienShips();

        drawHealth(player);
        mvprintw(1, 0, "Zestrzelone duze statki: %d", destroyedBigAlienShips);
        mvprintw(2, 0, "Zestrzelone male statki: %d", destroyedSmallAlienShips);
        mvprintw(3, 0, "Ilosc punktow: %d", earnedPoints);
        drawBullets();

        refresh();

        if (isGameOverTime) {
            clear();
            refresh();

            isExitTime = true;
            endwin();

            std::cout << "--------------------------------------------------------------------" << std::endl;
            std::cout << "Program:\n\tSpace Invaders\n" << std::endl;
            std::cout << "Autorzy:\n\tPatryk Zdral & Kamil Cieślik 2018\n" << std::endl;
            std::cout << "Zasady gry:\nGra polegająca na eliminowaniu kolejnych fal statków kosmicznych.\n";
//            mvprintw(row, col, "KONIEC GRY!");
//            mvprintw(row + 1, col, "Zestrzelone duze statki: %d", destroyedBigAlienShips);
//            mvprintw(row + 2, col, "Zestrzelone maze statki: %d", destroyedSmallAlienShips);
//            mvprintw(row + 3, col, "Ilosc punktow: %d", earnedPoints);
//            attroff(COLOR_PAIR(2));
//            attroff(A_BOLD);



            break;
        } else {
            std::this_thread::sleep_for(frameDuration);
        }
    }

    endwin();
    isGameOverTime = true;
    std::cout << "Kończenie wątków...\n" << std::endl;




    // TODO: Tu joinowanie poprawic trzeba bo sie nie konczy program.
    urandomIntCreationThread.join();

    /// matka produkująca nowe duże statki
    motherBigAlienShipsThread.join();

    /// matka produkująca nowe małe statki
    motherSmallAlienShipsThread.join();

    /// wątek nasluchujący listę nowych dużych statków i dodający je do gry
    addNewBigAlienShipsToActiveGameThread.join();

    /// wątek nasluchujący listę nowych małych statków i dodający je do gry
    addSmallAlienShipsToActiveGameThread.join();

    /// Launch big enemies movement thread
    moveBigAlienShipsThread.join();

    /// Launch small enemies movement thread
    moveSmallAlienShipsThread.join();

    /// Launch small bullets shooting thread
    moveSmallAlienShipsBulletsThread.join();

    /// Launch big slow enemies shooting thread
    //big_slow_enemies_shooting_thread.join();
    /// Launch big bullets shooting thread
    moveBigAlienShipsBulletsThread.join();

    /// matka produkująca nowe duże statki
    supportBigBulletsCreatorThread.join();

    /// matka produkująca nowe małe statki
    supportSmallBulletsCreatorThread.join();

    for (auto &i : threads_enemies_vector) {
        i.join();
    }
//    urandomIntCreationThread.join();
//    mvprintw(row + 5, col, "- urandom integers creation thread: FINISHED");
//    motherBigAlienShipsThread.join();
//    mvprintw(row + 16, col, "- mother big enemies creation thread: FINISHED");
//    motherSmallAlienShipsThread.join();
//    mvprintw(row + 17, col, "- mother small enemies creation thread: FINISHED");
//    addNewBigAlienShipsToActiveGameThread.join();
//    mvprintw(row + 6, col, "- big enemies creation thread: FINISHED");
//    addSmallAlienShipsToActiveGameThread.join();
//    mvprintw(row + 7, col, "- small enemies creation thread: FINISHED");
//    moveBigAlienShipsThread.join();
//    mvprintw(row + 8, col, "- big enemies motion thread: FINISHED");
//    moveSmallAlienShipsThread.join();
//    mvprintw(row + 9, col, "- small enemies motion thread: FINISHED");
//    big_slow_enemies_shooting_thread.join();
//    mvprintw(row + 10, col, "- big enemies shooting thread: FINISHED");
//    small_fast_enemies_shooting_thread.join();
//    mvprintw(row + 11, col, "- small enemies shooting thread: FINISHED");
//    moveBigAlienShipsBulletsThread.join();
//    mvprintw(row + 12, col, "- big bullets motion thread: FINISHED");
//    moveSmallAlienShipsBulletsThread.join();
//    mvprintw(row + 13, col, "- small bullets motion thread: FINISHED");
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "ZAKOŃCZONO DZIAŁANIE PROGRAMU" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;

}
//////////////////////////////////////////////

/// Funkcja generująca liczby całkowite z zakresu 0 - ULLONX_MAX pochodzące
/// z pliku /dev/urandom. Liczby są skalowane do zakresu 0 - 100 oraz castowane
/// na typ unsigned short.
void urandomIntGenerator() {
    unsigned long long int random_value = 0;
    size_t size = sizeof(random_value);
    while (!isGameOverTime) {
        std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
        if (urandom) {
            urandom.read(reinterpret_cast<char *>(&random_value), size);
            if (urandom) {
                std::unique_lock<std::mutex> locker(random_numbers_queue_condition_var_mutex);
                unsigned short random_casted_value = static_cast<unsigned short &&>(random_value / (ULLONG_MAX / 100));

                if (random_casted_value == 0) {
                    random_casted_value = 1;
                }

                newUrandomValuesQueue.push(random_casted_value);
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
    random_numbers_queue_condition_variable.wait(locker, [] { return !newUrandomValuesQueue.empty(); });
    assert(!newUrandomValuesQueue.empty());
    unsigned short random_short = newUrandomValuesQueue.front();
    newUrandomValuesQueue.pop();
    locker.unlock();

    return random_short;
}

/// Tworzenie małych statków kosmitów
void createSmallAlienShip() {
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);

    while (!isGameOverTime) {
        unsigned short random_short = get_random_number();
        auto enemy_small_fast = new SmallAlienShip(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                                   new_small_adder_bullet_mutex, new_small_bullet_condition_variable,
                                                   isGameOverTime, newSmallBulletsQueue,
                                                   smallActiveBulletsVector);

        enemy_small_fast->move_direction = LEFT;
        std::unique_lock<std::mutex> locker(new_small_enemy_mutex);
        newSmallAlienShipsQueue.push(enemy_small_fast);
        new_small_enemy_condition_variable.notify_one();
        locker.unlock();
        std::this_thread::sleep_for(timeBetweenNewSmallAlienShips);
    }
}

/// Tworzenie dużych statków kosmitów
void createBigAlienShip() {
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);

    while (!isGameOverTime) {
        unsigned short random_short = get_random_number();
        auto *enemy_big_slow = new BigAlienShip(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                                new_big_adder_bullet_mutex, new_big_bullet_condition_variable, isGameOverTime,
                                                newBigBulletsQueue,
                                                bigActiveBulletsVector);

        enemy_big_slow->move_direction = RIGHT;
        std::unique_lock<std::mutex> locker(new_big_enemy_mutex);
        newBigAlienShipsQueue.push(enemy_big_slow);
        new_big_enemy_condition_variable.notify_one();
        locker.unlock();
        std::this_thread::sleep_for(timeBetweenNewBigAlienShips);
    }
}

/// Funkcja pobierająca nowo wyprodukowany duży statek kosmitów i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addBigAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(new_big_adder_enemy_mutex);
        new_big_enemy_condition_variable.wait(locker, [] { return !newBigAlienShipsQueue.empty(); });
        assert(!newBigAlienShipsQueue.empty());

        bigActiveAlienShipsVector.push_back(newBigAlienShipsQueue.front());
        newBigAlienShipsQueue.pop();

        std::thread thread_enemy = bigActiveAlienShipsVector[bigActiveAlienShipsVector.size() -
                                                           1]->startInterceptionOfBulletsThreads();
        threads_enemies_vector.push_back(std::move(thread_enemy));

        locker.unlock();
    }
}

/// Funkcja pobierająca nowo wyprodukowany mały statek kosmitów i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addSmallAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(new_small_adder_enemy_mutex);
        new_small_enemy_condition_variable.wait(locker, [] { return !newSmallAlienShipsQueue.empty(); });
        assert(!newSmallAlienShipsQueue.empty());

        smallActiveAlienShipsVector.push_back(newSmallAlienShipsQueue.front());

        newSmallAlienShipsQueue.pop();

        std::thread thread_enemy = smallActiveAlienShipsVector[bigActiveAlienShipsVector.size() -
                                                             1]->startInterceptionOfBulletsThreads();
        threads_enemies_vector.push_back(std::move(thread_enemy));
        locker.unlock();
    }
}

/// Funkcja pomocnicza sprawdzająca kolizję z pojedynczym obiektem
bool isCollision(GameObject *bullet, GameObject *actor) {
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

/// Obsługa kolizji obiektów (pocisk - statek kosmita, pocisk - statek gracz), cz. 1.
/// Ustawienie flagi statku - isDone na true w sytuacji zniszczenia obiektu, flagi pocisku isDone na true
/// w sytuacji trafienia w obiekt. Dodatkowo - zwiększanie liczby punktów, liczby zniszczonych obiektów.
void handleBulletsHits(Player &player) {
    small_bullets_mutex.lock();
    for (SmallBullet *bullet : smallActiveBulletsVector) {
        if (isCollision(bullet, &player)) {
            bullet->setDone();
            player.setDamage(1);
        }
    }
    small_bullets_mutex.unlock();

    big_bullets_mutex.lock();
    for (BigBullet *bullet : bigActiveBulletsVector) {
        if (isCollision(bullet, &player)) {
            bullet->setDone();
            player.setDamage(5);
        }
    }
    big_bullets_mutex.unlock();

    player_bullets_mutex.lock();
    for (SmallBullet *bullet : playerActiveBulletsVector) {
        big_enemies_mutex.lock();
        for (BigAlienShip *enemy : bigActiveAlienShipsVector) {
            if (isCollision(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                if (enemy->isDone()) {
                    destroyedBigAlienShips++;
                }
                earnedPoints++;
            }
        }
        big_enemies_mutex.unlock();
        small_enemies_mutex.lock();
        for (SmallAlienShip *enemy : smallActiveAlienShipsVector) {
            if (isCollision(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                destroyedSmallAlienShips++;
                earnedPoints++;
            }
        }
        small_enemies_mutex.unlock();
    }
    player_bullets_mutex.unlock();
}

/// Obsługa kolizji obiektów (pocisk - statek kosmita, pocisk - statek gracz), cz. 2.
/// Zniszczenie obiektu, usuniecie z wektora w sytuacji gdy flaga isDone = true
void removeDestroyedAlienShips() {
    big_enemies_mutex.lock();
    if (!bigActiveAlienShipsVector.empty()) {
        auto it = bigActiveAlienShipsVector.begin();
        int j = 0;
        while (it != bigActiveAlienShipsVector.end()) {
            if (bigActiveAlienShipsVector[j]->isDone()) {
                it = bigActiveAlienShipsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    if (!smallActiveAlienShipsVector.empty()) {
        auto it = smallActiveAlienShipsVector.begin();
        int j = 0;
        while (it != smallActiveAlienShipsVector.end()) {
            if (smallActiveAlienShipsVector[j]->isDone()) {
                it = smallActiveAlienShipsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    small_enemies_mutex.unlock();
}

/// Obsługa kolizji obiektów, cz. 3.
/// Usuniecie zniszczonych pocisków
void removeUsedBullets() {
    small_bullets_mutex.lock();
    if (!smallActiveBulletsVector.empty()) {
        auto it = smallActiveBulletsVector.begin();
        int j = 0;
        while (it != smallActiveBulletsVector.end()) {
            if (smallActiveBulletsVector[j]->isDone()) {
                it = smallActiveBulletsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    small_bullets_mutex.unlock();

    big_bullets_mutex.lock();
    if (!bigActiveBulletsVector.empty()) {
        auto it = bigActiveBulletsVector.begin();
        int j = 0;
        while (it != bigActiveBulletsVector.end()) {
            if (bigActiveBulletsVector[j]->isDone()) {
                it = bigActiveBulletsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    big_bullets_mutex.unlock();

    player_bullets_mutex.lock();
    if (!playerActiveBulletsVector.empty()) {
        auto it = playerActiveBulletsVector.begin();
        int j = 0;
        while (it != playerActiveBulletsVector.end()) {
            if (playerActiveBulletsVector[j]->isDone()) {
                it = playerActiveBulletsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    player_bullets_mutex.unlock(); // End of critical section
}

/// Tworzenie pocisków gracza
void playerBullets(GameObject &player) {
    // Create the bullet
    auto *bullet = new SmallBullet(short(player.getPos_x() + player.getWidth() / 2), short(player.getPos_y()), 0,
                                   getmaxx(stdscr), 0,
                                   player.getPos_y());
    bullet->move_direction = UP;

    player_bullets_mutex.lock();
    playerActiveBulletsVector.push_back(bullet);
    player_bullets_mutex.unlock();
}

/// Rysowanie pocisków
void drawBullets() {
    for (SmallBullet *bullet : smallActiveBulletsVector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(2));
            }
            ncurses_mutex.lock();
            bullet->drawObject();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(2));
            }
            attroff(A_BOLD);
        }
    }

    for (BigBullet *bullet : bigActiveBulletsVector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(2));
            }
            ncurses_mutex.lock();
            bullet->drawObject();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(2));
            }
            attroff(A_BOLD);
        }
    }

    for (SmallBullet *bullet : playerActiveBulletsVector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(1));
            }
            ncurses_mutex.lock();
            bullet->drawObject();
            ncurses_mutex.unlock();
            if (has_colors()) {
                attroff(COLOR_PAIR(1));
            }
            attroff(A_BOLD);
        }
    }
}

/// Przesuwanie koordynatów małych pocisków
void moveSmallBullets() {
    int milis_per_row = 1000 / smallBulletsSpeed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!isGameOverTime) {
        small_bullets_mutex.lock();
        for (SmallBullet *bullet : smallActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, 1);
            }
        }
        small_bullets_mutex.unlock();
        player_bullets_mutex.lock();
        for (SmallBullet *bullet : playerActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, -1);
            }
        }
        player_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}

/// Przesuwanie koordynatów dużych pocisków
void moveBigBullets() {
    int milis_per_row = 1000 / bigBulletsSpeed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!isGameOverTime) {
        big_bullets_mutex.lock();
        for (BigBullet *bullet : bigActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, bullet->move_direction == DOWN ? short(1) : short(-1));
            }
        }
        big_bullets_mutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}

/// Tworzenie pocisków dużych statków co losowo określony czas
void createBigBullets() {
    while (!isGameOverTime) {
        auto *bullet = new BigBullet();
        bullet->move_direction = DOWN;

        std::unique_lock<std::mutex> locker(new_big_adder_bullet_mutex);
        newBigBulletsQueue.push(bullet);
        new_big_bullet_condition_variable.notify_one();
        locker.unlock();

        auto random_short = static_cast<unsigned int>(get_random_number() * 100);
        std::chrono::milliseconds t_between_creation_big_bullets(random_short);
        std::this_thread::sleep_for(t_between_creation_big_bullets);
    }
}

/// Tworzenie pocisków małych statków co losowo określony czas
void createSmallBullets() {
    // Create the bullets
    while (!isGameOverTime) {
        auto *bullet = new SmallBullet();
        bullet->move_direction = DOWN;

        std::unique_lock<std::mutex> locker(new_small_adder_bullet_mutex);
        newSmallBulletsQueue.push(bullet);
        new_small_bullet_condition_variable.notify_one();
        locker.unlock();

        auto random_short = static_cast<unsigned int>(get_random_number() * 20);
        std::chrono::milliseconds t_between_creating_small_bullets(random_short);
        std::this_thread::sleep_for(t_between_creating_small_bullets);
    }
}

/// Rysowanie statków kosmitów
void drawAlienShips() {
    big_enemies_mutex.lock();
    for (GameObject *enemy : bigActiveAlienShipsVector) {
        ncurses_mutex.lock();
        enemy->drawObject();
        ncurses_mutex.unlock();
    }
    big_enemies_mutex.unlock();

    small_enemies_mutex.lock();
    for (GameObject *enemy : smallActiveAlienShipsVector) {
        ncurses_mutex.lock();
        enemy->drawObject();
        ncurses_mutex.unlock();
    }
    small_enemies_mutex.unlock();
};

/// Przesuwanie koordynatów małych statków kosmitów
void moveSmallAlienShips() {
    int milis_per_column = 1000 / smallAlienShipsSpeed;
    std::chrono::milliseconds t_col(milis_per_column);
    while (!isGameOverTime) {
        for (auto enemy : smallActiveAlienShipsVector) {

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
                    isGameOverTime = true;
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
                    isGameOverTime = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}

/// Przesuwanie koordynatów dużych statków kosmitów
void moveBigAlienShips() {
    int milis_per_column = 1000 / bigAlienShipsSpeed;
    std::chrono::milliseconds t_col(milis_per_column);
    while (!isGameOverTime) {
        for (auto enemy : bigActiveAlienShipsVector) {

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
                    isGameOverTime = true;
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
                    isGameOverTime = true;
                }
                continue;
            }
        }
        std::this_thread::sleep_for(t_col);
    }
}

void drawHealth(Player &player) {
    int hp = (player.getHit_points() * 100) / health;
    std::string s = std::to_string(hp);
    s = s + "%%";
    char const *pchar = s.c_str();
    mvprintw(0, 0, "HEALTH: ");
    if (has_colors()) {
        attron(COLOR_PAIR(1));
    }
    mvprintw(0, 8, pchar);
    if (has_colors()) {
        attroff(COLOR_PAIR(1));
    }
}

int main() {

    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "Program:\n\tSpace Invaders\n" << std::endl;
    std::cout << "Autorzy:\n\tPatryk Zdral & Kamil Cieślik 2018\n" << std::endl;
    std::cout << "Zasady gry:\nGra polegająca na eliminowaniu kolejnych fal statków kosmicznych.\n"
                 "W dolnej części ekranu, na stałej wysokości, gracz ma możliwość\n"
                 "poruszania się horyzontalnie. Celem gracza jest zdobywanie\n"
                 "punktów poprzez zestrzeliwanie atakujących go statków kosmicznych.\n"
                 "Gra kończy się w sytuacji ręcznego zatrzymania, utraty całego\n"
                 "poziomu zdrowia lub zetknięcia się któregoś z wrogich statków\n"
                 "z „Ziemią” – dolną częścią ekranu.\n" << std::endl;
    std::cout << "Sterowanie:\n\ta, d - ruch w lewo i prawo,\n"
                 "\tstrzał - spacja,\n"
                 "\twyjście - q." << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    system("read -p 'Naciśnij Enter aby rozpocząć...' var");

    /// Initialize ncurses
    initscr();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
    }
    /// Create player
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);

    auto *player = new Player(stdscr_maxx / 2 - 3, stdscr_maxy - 1, 0, stdscr_maxx, 0, stdscr_maxy);
    health = player->getHit_points();
    /// Launch view refresh thread
    std::thread refresh_thread(runApp, std::ref(*player));

    while (true) {
        int key = getch();
        if (key == 'q') {
            isExitTime = true;
            break;
        }

        if (key == SPACE) {
            playerBullets(*player);
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
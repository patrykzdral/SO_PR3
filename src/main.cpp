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
#include <map>
#include "SmallBullet.h"
#include "Direction.h"
#include "Player.h"
#include "BigAlienShip.h"
#include "BigBullet.h"
#include "SmallAlienShip.h"
#include "ShipWreck.h"

static const std::chrono::milliseconds frameDuration(40);
static const std::chrono::milliseconds timeBetweenNewBigAlienShips(5000);
static const std::chrono::milliseconds timeBetweenNewSmallAlienShips(5000);


/// Prędkość pocisków (wiersze na sekundę)
static const int smallBulletsSpeed = 30;
static const int bigBulletsSpeed = 15;


/// Prędkość statków kosmitów (kolumny na sekundę)
static const int bigAlienShipsSpeed = 8;
static const int smallAlienShipsSpeed = 15;

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
/// generowanych z pliku /dev/urandom
static std::queue<unsigned short> newUrandomValuesQueue;


/// Kolejka wraków statków
static std::queue<ShipWreck *> shipWreckQueue;


/// Wektor wątków statków kosmitycznych przechwytujących naboje.
static std::vector<std::thread> threadsSmallAlienShipsVector;
static std::vector<std::thread> threadsBigAlienShipsVector;


/// Statystyki
static int earnedPoints = 0;
static int destroyedBigAlienShips = 0;
static int destroyedSmallAlienShips = 0;


/// Mutexy
static std::mutex playerMutex;
static std::mutex ncursesMutex;

static std::mutex playerBulletsMutex;
static std::mutex smallBulletsMutex;
static std::mutex bigBulletsMutex;
static std::mutex bigAlienShipsMutex;
static std::mutex smallAlienShipsMutex;

static std::mutex newUrandomValuesConditionVarMutex;

static std::mutex newBigAlienShipsMutex;
static std::mutex newBigAlienShipsAdderToActiveGameMutex;

static std::mutex newSmallAlienShipsMutex;
static std::mutex newSmallAlienShipsAdderToActiveGameMutex;

static std::mutex newSmallBulletMutex;
static std::mutex newBigBulletMutex;

static std::mutex newSmallBulletAdderToActiveGameMutex;
static std::mutex newBigBulletAdderToActiveGameMutex;

static std::mutex shipWreckMutex;


/// Condition variables
static std::condition_variable random_numbers_queue_condition_variable;

static std::condition_variable new_big_enemy_condition_variable;
static std::condition_variable new_small_enemy_condition_variable;

static std::condition_variable new_big_bullet_condition_variable;
static std::condition_variable new_small_bullet_condition_variable;

static std::condition_variable shipWreck_condition_variable;

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

void addSmallGreenAlienShipToActiveGame();

void addBigBlueAlienShipToActiveGame();

void urandomIntGenerator();


void runApp(Player &player) {

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

    /// Wątek odpowiadający za poruszanie się dużych statków kosmitów
    std::thread moveBigAlienShipsThread(moveBigAlienShips);

    /// Wątek odpowiadający za poruszanie się małych statków kosmitów
    std::thread moveSmallAlienShipsThread(moveSmallAlienShips);

    /// Wątek nasłuchujący wektor nowych dużych statków i dodający je do gry w niebieskim kolorze
    std::thread big_blue_enemies_creation_thread(addBigBlueAlienShipToActiveGame);

    /// Wątek nasłuchujący wektor nowych małych statków i dodający je do gry w zielonym kolorze
    std::thread small_green_enemies_creation_thread(addSmallGreenAlienShipToActiveGame);

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
        playerMutex.lock();
        ncursesMutex.lock();
        player.drawObject();
        ncursesMutex.unlock();
        playerMutex.unlock();
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
            isExitTime = true;

            std::cout << "KONIEC GRY!" << std::endl;
            std::cout << "Statystyki:" << std::endl;
            std::cout << "- zestrzelone duże statki: " << destroyedBigAlienShips << "," << std::endl;
            std::cout << "- zestrzelone małe statki: " << destroyedSmallAlienShips << "," << std::endl;
            std::cout << "- ilość punktów: " << earnedPoints << "." << std::endl;

            break;
        } else {
            std::this_thread::sleep_for(frameDuration);
        }
    }

    refresh();
    endwin();
    system("clear");

    isGameOverTime = true;

    std::cout << "Kończenie wątków:" << std::endl;
    moveSmallAlienShipsBulletsThread.join();
    std::cout << "- wątek poruszający małymi pociskami: zakończono," << std::endl;

    moveBigAlienShipsBulletsThread.join();
    std::cout << "- wątek poruszający dużymi pociskami: zakończono," << std::endl;

    moveBigAlienShipsThread.join();
    std::cout << "- wątek poruszający dużymi jednostkami: zakończono," << std::endl;

    moveSmallAlienShipsThread.join();
    std::cout << "- wątek poruszający małymi jednostkami: zakończono," << std::endl;

    urandomIntCreationThread.join();
    std::cout << "- wątek tworzenia liczb pseudolosowych: zakończono," << std::endl;

    motherBigAlienShipsThread.join();
    std::cout << "- wątek matki tworzącej duże statki: zakończono," << std::endl;

    motherSmallAlienShipsThread.join();
    std::cout << "- watek matki tworzącej małe statki: zakończono," << std::endl;

    supportBigBulletsCreatorThread.join();
    std::cout << "- wątek matki produkującej duże naboje: zakończono," << std::endl;

    supportSmallBulletsCreatorThread.join();
    std::cout << "- wątek matki produkującej małe naboje: zakończono," << std::endl;

    for (auto &i : threadsSmallAlienShipsVector) {
        i.join();
    }
    std::cout << "- wątki małych statków: zakończono," << std::endl;

    for (auto &i : threadsBigAlienShipsVector) {
        i.join();
    }
    std::cout << "- wątki dużych statków: zakończono," << std::endl;

    big_blue_enemies_creation_thread.join();
    std::cout << "- wątek nowych dużych statków i dodający je do gry w niebieskim kolorze: zakończono," << std::endl;

    small_green_enemies_creation_thread.join();
    std::cout << "- wątek nowych dużych statków i dodający je do gry w zielonym kolorze: zakończono," << std::endl;

    addSmallAlienShipsToActiveGameThread.join();
    std::cout << "- wątek nasłuchujący listę nowych dużych statków i dodający je do gry: zakończono," << std::endl;

    addNewBigAlienShipsToActiveGameThread.join();
    std::cout << "- wątek nasłuchujący listę nowych małych statków i dodający je do gry: zakończono.\n" << std::endl;

    std::cout << "Zakończono wszystkie wątki.\nNaciśnij 'q' aby wyjść." << std::endl;
}

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
                std::unique_lock<std::mutex> locker(newUrandomValuesConditionVarMutex);
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
unsigned short getRandomNumber() {
    std::unique_lock<std::mutex> locker(newUrandomValuesConditionVarMutex);
    random_numbers_queue_condition_variable.wait(locker, [] { return (!newUrandomValuesQueue.empty()); });
    assert(!newUrandomValuesQueue.empty());
    if (isGameOverTime) return 0;
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
        unsigned short random_short = getRandomNumber();
        auto enemy_small_fast = new SmallAlienShip(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                                   newSmallBulletAdderToActiveGameMutex,
                                                   new_small_bullet_condition_variable, isGameOverTime,
                                                   newSmallBulletsQueue,
                                                   smallActiveBulletsVector);
        enemy_small_fast->move_direction = LEFT;
        std::unique_lock<std::mutex> locker(newSmallAlienShipsMutex);
        newSmallAlienShipsQueue.push(enemy_small_fast);
        new_small_enemy_condition_variable.notify_one();

        shipWreck_condition_variable.wait(locker, [] { return !shipWreckQueue.empty(); });
        assert(!shipWreckQueue.empty());
        if(shipWreckQueue.size()>2){
            unsigned short random_short2 = getRandomNumber();
            auto enemy_small_fast2 = new SmallAlienShip(stdscr_maxx / random_short2, 0, 0, stdscr_maxx, 0, stdscr_maxy,newSmallBulletAdderToActiveGameMutex,
                                                   new_small_bullet_condition_variable, isGameOverTime, newSmallBulletsQueue,
                                                   smallActiveBulletsVector);
            newSmallAlienShipsQueue.push(enemy_small_fast2);
            shipWreckQueue.pop();
            shipWreckQueue.pop();

        }

        locker.unlock();
        std::this_thread::sleep_for(timeBetweenNewSmallAlienShips);
    }
}

/// Tworzenie dużych statków kosmitów
void createBigAlienShip() {
    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);
    while (!isGameOverTime) {
        unsigned short random_short = getRandomNumber();
        auto *enemy_big_slow = new BigAlienShip(stdscr_maxx / random_short, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                                newBigBulletAdderToActiveGameMutex, new_big_bullet_condition_variable,
                                                isGameOverTime,
                                                newBigBulletsQueue,
                                                bigActiveBulletsVector);


        enemy_big_slow->move_direction = RIGHT;
        std::unique_lock<std::mutex> locker(newBigAlienShipsMutex);
        newBigAlienShipsQueue.push(enemy_big_slow);
        new_big_enemy_condition_variable.notify_one();

        locker.unlock();

        std::this_thread::sleep_for(timeBetweenNewBigAlienShips);
    }

    std::unique_lock<std::mutex> locker_wreck(shipWreckMutex);
    shipWreck_condition_variable.wait(locker_wreck, [] { return !shipWreckQueue.empty(); });
    assert(!shipWreckQueue.empty());
    if(shipWreckQueue.size()>4){
        unsigned short random_short2 = getRandomNumber();
        auto *enemy_big_slow2 = new BigAlienShip(stdscr_maxx / random_short2, 0, 0, stdscr_maxx, 0, stdscr_maxy,
                                             newBigBulletAdderToActiveGameMutex, new_big_bullet_condition_variable, isGameOverTime, newBigBulletsQueue,
                                             bigActiveBulletsVector);
        newBigAlienShipsQueue.push(enemy_big_slow2);
        shipWreckQueue.pop();
        shipWreckQueue.pop();
        shipWreckQueue.pop();
        shipWreckQueue.pop();

    }
    locker_wreck.unlock();
}

/// Funkcja pobierająca nowo wyprodukowany duży statek kosmitów i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addBigAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(newBigAlienShipsAdderToActiveGameMutex);
        new_big_enemy_condition_variable.wait(locker,
                                              [] { return (!newBigAlienShipsQueue.empty()); });
        assert(!newBigAlienShipsQueue.empty());

        bigActiveAlienShipsVector.push_back(newBigAlienShipsQueue.front());

        newBigAlienShipsQueue.pop();

        std::thread enemy = bigActiveAlienShipsVector[bigActiveAlienShipsVector.size() -
                                                      1]->startInterceptionOfBulletsThreads();
        threadsBigAlienShipsVector.push_back(std::move(enemy));

        locker.unlock();
    }
}

/// Funkcja pobierająca nowo wyprodukowany mały statek kosmitów i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addSmallAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(newSmallAlienShipsAdderToActiveGameMutex);
        new_small_enemy_condition_variable.wait(locker,
                                                [] { return (!newSmallAlienShipsQueue.empty()); });
        assert((!newSmallAlienShipsQueue.empty()));
        smallActiveAlienShipsVector.push_back(newSmallAlienShipsQueue.front());
        newSmallAlienShipsQueue.pop();

        std::thread enemy = smallActiveAlienShipsVector[smallActiveAlienShipsVector.size() - 1]->startThread();
        threadsSmallAlienShipsVector.push_back(std::move(enemy));

        locker.unlock();
    }
}

/// Funkcja pobierająca nowo wyprodukowany duży statek kosmitów, zmieniająca jego kolor i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addBigBlueAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(newBigAlienShipsAdderToActiveGameMutex);
        new_big_enemy_condition_variable.wait(locker,
                                              [] { return (!newBigAlienShipsQueue.empty()); });
        assert((!newBigAlienShipsQueue.empty()));
        BigAlienShip *enemyBig = newBigAlienShipsQueue.front();
        enemyBig->setIsBlue(true);
        bigActiveAlienShipsVector.push_back(enemyBig);

        newBigAlienShipsQueue.pop();

        std::thread enemy = bigActiveAlienShipsVector[bigActiveAlienShipsVector.size() -
                                                      1]->startInterceptionOfBulletsThreads();
        threadsBigAlienShipsVector.push_back(std::move(enemy));

        locker.unlock();
    }
}

/// Funkcja pobierająca nowo wyprodukowany mały statek kosmitów, zmieniająca jego kolor i dodająca go do aktywnej gry.
/// Ponadto uruchamiany zostaje wątek statku, który przechwytuje nowo wygenerowane pociski.
void addSmallGreenAlienShipToActiveGame() {
    while (!isGameOverTime) {
        std::unique_lock<std::mutex> locker(newSmallAlienShipsAdderToActiveGameMutex);
        new_small_enemy_condition_variable.wait(locker,
                                                [] { return (!newSmallAlienShipsQueue.empty()); });
        assert((!newSmallAlienShipsQueue.empty()));
        SmallAlienShip *enemySmall = newSmallAlienShipsQueue.front();
        enemySmall->setIsGreen(true);
        smallActiveAlienShipsVector.push_back(enemySmall);
        newSmallAlienShipsQueue.pop();

        std::thread enemy = smallActiveAlienShipsVector[smallActiveAlienShipsVector.size() - 1]->startThread();
        threadsSmallAlienShipsVector.push_back(std::move(enemy));

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
    smallBulletsMutex.lock();
    for (SmallBullet *bullet : smallActiveBulletsVector) {
        if (isCollision(bullet, &player)) {
            bullet->setDone();
            player.setDamage(1);
        }
    }
    smallBulletsMutex.unlock();

    bigBulletsMutex.lock();
    for (BigBullet *bullet : bigActiveBulletsVector) {
        if (isCollision(bullet, &player)) {
            bullet->setDone();
            player.setDamage(5);
        }
    }
    bigBulletsMutex.unlock();

    playerBulletsMutex.lock();
    for (SmallBullet *bullet : playerActiveBulletsVector) {
        bigAlienShipsMutex.lock();
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
        bigAlienShipsMutex.unlock();
        smallAlienShipsMutex.lock();
        for (SmallAlienShip *enemy : smallActiveAlienShipsVector) {
            if (isCollision(bullet, enemy)) {
                bullet->setDone();
                enemy->setDamage(1);
                destroyedSmallAlienShips++;
                earnedPoints++;
            }
        }
        smallAlienShipsMutex.unlock();
    }
    playerBulletsMutex.unlock();
}

/// Obsługa kolizji obiektów (pocisk - statek kosmita, pocisk - statek gracz), cz. 2.
/// Zniszczenie obiektu, usuniecie z wektora w sytuacji gdy flaga isDone = true
void removeDestroyedAlienShips() {
    bigAlienShipsMutex.lock();
    if (!bigActiveAlienShipsVector.empty()) {
        auto it = bigActiveAlienShipsVector.begin();
        int j = 0;
        while (it != bigActiveAlienShipsVector.end()) {
            if (bigActiveAlienShipsVector[j]->isDone()) {
                bigActiveAlienShipsVector[j]->setDied(true);

                auto *shipWreck = new ShipWreck();
                std::unique_lock<std::mutex> locker(shipWreckMutex);
                shipWreckQueue.push(shipWreck);
                shipWreck_condition_variable.notify_one();
                locker.unlock();
                it = bigActiveAlienShipsVector.erase(it);
            } else {
                j++;
                it++;
            }
        }
    }
    bigAlienShipsMutex.unlock();

    smallAlienShipsMutex.lock();
    if (!smallActiveAlienShipsVector.empty()) {
        auto it = smallActiveAlienShipsVector.begin();
        int j = 0;
        while (it != smallActiveAlienShipsVector.end()) {
            if (smallActiveAlienShipsVector[j]->isDone()) {
                smallActiveAlienShipsVector[j]->setDied(true);

                auto *shipWreck = new ShipWreck();
                std::unique_lock<std::mutex> locker(shipWreckMutex);
                shipWreckQueue.push(shipWreck);
                shipWreck_condition_variable.notify_one();
                locker.unlock();
                it = smallActiveAlienShipsVector.erase(it);

            } else {
                j++;
                it++;
            }
        }
    }
    smallAlienShipsMutex.unlock();
}

/// Obsługa kolizji obiektów, cz. 3.
/// Usuniecie zniszczonych pocisków
void removeUsedBullets() {
    smallBulletsMutex.lock();
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
    smallBulletsMutex.unlock();

    bigBulletsMutex.lock();
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
    bigBulletsMutex.unlock();

    playerBulletsMutex.lock();
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
    playerBulletsMutex.unlock();
}

/// Tworzenie pocisków gracza
void playerBullets(GameObject &player) {
    SmallBullet *bullet = new SmallBullet(short(player.getPos_x() + player.getWidth() / 2), short(player.getPos_y()), 0,
                                          getmaxx(stdscr), 0,
                                          player.getPos_y());
    bullet->move_direction = UP;
    playerBulletsMutex.lock();
    playerActiveBulletsVector.push_back(bullet);
    playerBulletsMutex.unlock();
}

/// Rysowanie pocisków
void drawBullets() {
    for (SmallBullet *bullet : smallActiveBulletsVector) {
        if (!bullet->isDone()) {
            attron(A_BOLD);
            if (has_colors()) {
                attron(COLOR_PAIR(2));
            }

            ncursesMutex.lock();
            bullet->drawObject();
            ncursesMutex.unlock();

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

            ncursesMutex.lock();
            bullet->drawObject();
            ncursesMutex.unlock();

            if (has_colors()) {
                attroff(COLOR_PAIR(1));
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

            ncursesMutex.lock();
            bullet->drawObject();
            ncursesMutex.unlock();

            if (has_colors()) {
                attroff(COLOR_PAIR(2));
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
        smallBulletsMutex.lock();
        for (SmallBullet *bullet : smallActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, 1);
            }
        }

        smallBulletsMutex.unlock();
        playerBulletsMutex.lock();
        for (SmallBullet *bullet : playerActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, -1);
            }
        }
        playerBulletsMutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}

/// Przesuwanie koordynatów dużych pocisków
void moveBigBullets() {
    int milis_per_row = 1000 / bigBulletsSpeed;
    std::chrono::milliseconds t_row(milis_per_row);
    while (!isGameOverTime) {
        bigBulletsMutex.lock();
        for (BigBullet *bullet : bigActiveBulletsVector) {
            if (!bullet->isDone()) {
                bullet->move(0, bullet->move_direction == DOWN ? short(1) : short(-1));
            }
        }
        bigBulletsMutex.unlock();
        std::this_thread::sleep_for(t_row);
    }
}

/// Tworzenie pocisków dużych statków co losowo określony czas
void createBigBullets() {
    while (!isGameOverTime) {
        auto *bullet = new BigBullet();
        bullet->move_direction = DOWN;
        std::unique_lock<std::mutex> locker(newBigBulletMutex);
        newBigBulletsQueue.push(bullet);
        new_big_bullet_condition_variable.notify_one();
        locker.unlock();
        auto random_short = static_cast<unsigned int>(getRandomNumber() * 100);
        static const std::chrono::milliseconds t_between_creation_big_bullets(random_short);

        std::this_thread::sleep_for(t_between_creation_big_bullets);
    }
}

/// Tworzenie pocisków małych statków co losowo określony czas
void createSmallBullets() {
    while (!isGameOverTime) {
        auto *bullet = new SmallBullet();
        bullet->move_direction = DOWN;
        std::unique_lock<std::mutex> locker(newSmallBulletMutex);
        newSmallBulletsQueue.push(bullet);
        new_small_bullet_condition_variable.notify_one();
        locker.unlock();

        auto random_short = static_cast<unsigned int>(getRandomNumber() * 100);
        static const std::chrono::milliseconds t_between_creation_big_bullets(random_short / 5);

        std::this_thread::sleep_for(t_between_creation_big_bullets);
    }
}

/// Rysowanie statków kosmitów
void drawAlienShips() {
    bigAlienShipsMutex.lock();
    for (GameObject *enemy : bigActiveAlienShipsVector) {
        ncursesMutex.lock();
        enemy->drawObject();
        ncursesMutex.unlock();
    }
    bigAlienShipsMutex.unlock();

    smallAlienShipsMutex.lock();
    for (GameObject *enemy : smallActiveAlienShipsVector) {
        ncursesMutex.lock();
        enemy->drawObject();
        ncursesMutex.unlock();
    }
    smallAlienShipsMutex.unlock();
}

/// Przesuwanie koordynatów małych statków kosmitów
void moveSmallAlienShips() {
    int milis_per_column = 1000 / smallAlienShipsSpeed;
    std::chrono::milliseconds t_col(milis_per_column);
    while (!isGameOverTime) {
        for (auto enemy : smallActiveAlienShipsVector) {

            unsigned short random_short = getRandomNumber();
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
            unsigned short random_short = getRandomNumber();
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

    /// Ncurses
    initscr();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
    }

    int stdscr_maxx = getmaxx(stdscr);
    int stdscr_maxy = getmaxy(stdscr);

    auto *player = new Player(stdscr_maxx / 2 - 3, stdscr_maxy - 1, 0, stdscr_maxx, 0, stdscr_maxy);
    health = player->getHit_points();
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
            /// Ruch w lewo
            playerMutex.lock();
            player->move(-1, 0);
            playerMutex.unlock();
        }

        if (key == 'd') {
            /// Ruch w prawo
            playerMutex.lock();
            player->move(1, 0);
            playerMutex.unlock();
        }

    }

    refresh_thread.join();
    endwin();
    return 0;
}
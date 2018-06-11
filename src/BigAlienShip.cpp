//
// Created by piotrek on 04.06.17.
//

#include <cassert>
#include "BigAlienShip.h"
#include "BigBullet.h"

BigAlienShip::BigAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                           std::mutex &_newBigBulletsQueueConditionVarMutex,
                           std::condition_variable &_newBigBulletsQueueConditionVariable,
                           std::atomic_bool &_game_over, std::queue<BigBullet *> &_newBigBulletsQueue,
                           std::vector<BigBullet *> &_bigActiveBulletsVector) :
        GameObject(_pos_x, _pos_y, 9, 3, _min_x, _max_x, _min_y, _max_y),
        newBigBulletsQueueConditionVariable(_newBigBulletsQueueConditionVariable),
        newBigBulletsQueueConditionVarMutex(_newBigBulletsQueueConditionVarMutex),
        game_over(_game_over), newBigBulletsQueue(_newBigBulletsQueue),
        bigActiveBulletsVector(_bigActiveBulletsVector){
    hit_points = 10;
}

void BigAlienShip::drawObject() {
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x + 1, "_");
    mvprintw(pos_y, pos_x + 2, "_");
    mvprintw(pos_y, pos_x + 3, "_");
    mvprintw(pos_y, pos_x + 4, "_");
    mvprintw(pos_y, pos_x + 5, "_");
    mvprintw(pos_y, pos_x + 6, "_");
    mvprintw(pos_y, pos_x + 7, "_");
    mvprintw(pos_y, pos_x + 8, "|");

    mvprintw(pos_y + 1, pos_x, "|");
    mvprintw(pos_y + 1, pos_x + 1, "_");
    mvprintw(pos_y + 1, pos_x + 2, "_");
    mvprintw(pos_y + 1, pos_x + 3, "_");
    mvprintw(pos_y + 1, pos_x + 4, "_");
    mvprintw(pos_y + 1, pos_x + 5, "_");
    mvprintw(pos_y + 1, pos_x + 6, "_");
    mvprintw(pos_y + 1, pos_x + 7, "_");
    mvprintw(pos_y + 1, pos_x + 8, "|");

    mvprintw(pos_y + 2, pos_x, "|");
    mvprintw(pos_y + 2, pos_x + 8, "|");
}

void BigAlienShip::addBigBulletToActiveGame() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(newBigBulletsQueueConditionVarMutex);
        newBigBulletsQueueConditionVariable.wait(locker, [this] { return !newBigBulletsQueue.empty(); });
        assert(!newBigBulletsQueue.empty());

        BigBullet *&bigBullet = newBigBulletsQueue.front();
        bigBullet->setParameters(short(getPos_x() + getWidth() / 2 - 1), short(getPos_y() + 1), 3, 3, 0,
                                 getmaxx(stdscr), 0,
                                 getmaxy(stdscr) + 3);

        newBigBulletsQueue.pop();
        bigActiveBulletsVector.push_back(bigBullet);
        locker.unlock();
    }
}

std::thread BigAlienShip::startInterceptionOfBulletsThreads() {
    return std::thread(&BigAlienShip::addBigBulletToActiveGame, this);
}





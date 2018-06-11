
#include <cassert>
#include "SmallAlienShip.h"


void SmallAlienShip::drawObject() {
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x + 1, "=");
    mvprintw(pos_y, pos_x + 2, "=");
    mvprintw(pos_y, pos_x + 3, "=");
    mvprintw(pos_y, pos_x + 4, "|");
}

SmallAlienShip::SmallAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                               std::mutex &_newSmallBulletsQueueConditionVarMutex,
                               std::condition_variable &_newSmallBulletConditionVariable,
                               std::atomic_bool &_game_over, std::queue<SmallBullet *> &_newSmallBulletsQueue,
                               std::vector<SmallBullet *> &_big_bullets_vector) :
        GameObject(_pos_x, _pos_y, 5, 1, _min_x, _max_x, _min_y, _max_y),
        newSmallBulletsQueueConditionVariable(_newSmallBulletConditionVariable),
        newSmallBulletsQueueConditionVarMutex(_newSmallBulletsQueueConditionVarMutex),
        game_over(_game_over), newSmallBulletsQueue(_newSmallBulletsQueue),
        smallActiveBulletsVector(_big_bullets_vector) {
    hit_points = 1;
}

void SmallAlienShip::addSmallBulletToActiveGame() {
    while (!game_over) {
        std::unique_lock<std::mutex> locker(newSmallBulletsQueueConditionVarMutex);
        newSmallBulletsQueueConditionVariable.wait(locker, [this] { return !newSmallBulletsQueue.empty(); });
        assert(!newSmallBulletsQueue.empty());

        SmallBullet *&smallBullet = newSmallBulletsQueue.front();
        smallBullet->setParameters(short(getPos_x() + getWidth() / 2 - 1), short(getPos_y() + 1), 3, 3, 0,
                                   getmaxx(stdscr), 0,
                                   getmaxy(stdscr) + 3);

        newSmallBulletsQueue.pop();
        smallActiveBulletsVector.push_back(smallBullet);
        locker.unlock();
    }
}


std::thread SmallAlienShip::startInterceptionOfBulletsThreads() {
    return std::thread(&SmallAlienShip::addSmallBulletToActiveGame, this);
}

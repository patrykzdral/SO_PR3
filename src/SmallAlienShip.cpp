
#include <cassert>
#include "SmallAlienShip.h"


SmallAlienShip::SmallAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                               std::mutex &_newSmallBulletsQueueConditionVarMutex,
                               std::condition_variable &_newSmallBulletConditionVariable,
                               std::atomic_bool &_game_over, std::queue<SmallBullet *> &_newSmallBulletsQueue,
                               std::vector<SmallBullet *> &_big_bullets_vector) : GameObject(_pos_x, _pos_y, 5, 1,
                                                                                             _min_x, _max_x, _min_y,
                                                                                             _max_y),
                                                                                  new_small_bullet_condition_variable(
                                                                                          _newSmallBulletConditionVariable),
                                                                                  new_small_adder_bullet_mutex(
                                                                                          _newSmallBulletsQueueConditionVarMutex),
                                                                                  game_over(_game_over),
                                                                                  new_small_bullets_queue(
                                                                                          _newSmallBulletsQueue),
                                                                                  small_bullets_vector(
                                                                                          _big_bullets_vector),
                                                                                  isBlue(false), died(false) {
    hit_points = 1;
}

void SmallAlienShip::drawObject() {
    if (isBlue) {
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(5));
    }

    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x + 1, "=");
    mvprintw(pos_y, pos_x + 2, "=");
    mvprintw(pos_y, pos_x + 3, "=");
    mvprintw(pos_y, pos_x + 4, "|");

    if (isBlue)
        attroff(COLOR_PAIR(5));

}

void SmallAlienShip::addSmallBulletToActiveGame() {
    while (!died) {
        std::unique_lock<std::mutex> locker(new_small_adder_bullet_mutex);
        new_small_bullet_condition_variable.wait(locker, [this] { return (!new_small_bullets_queue.empty()); });
        assert((!new_small_bullets_queue.empty()));

        SmallBullet *&smallBullet = new_small_bullets_queue.front();
        smallBullet->setParameters(short(getPos_x() + getWidth() / 2), short(getPos_y()), 1, 1, 0,
                                   getmaxx(stdscr), 0,
                                   getmaxy(stdscr));

        new_small_bullets_queue.pop();
        small_bullets_vector.push_back(smallBullet);
        locker.unlock();
    }

    delete &new_small_adder_bullet_mutex;
    delete &new_small_bullet_condition_variable;
    delete &game_over;
    delete[] &new_small_bullets_queue;
    delete[] &small_bullets_vector;
}

std::thread SmallAlienShip::startThread() {
    return std::thread(&SmallAlienShip::addSmallBulletToActiveGame, this);
}

void SmallAlienShip::setIsGreen(bool isBlue) {
    SmallAlienShip::isBlue = isBlue;
}

void SmallAlienShip::setDied(bool died) {
    SmallAlienShip::died = died;
}

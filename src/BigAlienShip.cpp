#include <cassert>
#include "BigAlienShip.h"
#include "BigBullet.h"

BigAlienShip::BigAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                           std::mutex &_newBigBulletsQueueConditionVarMutex,
                           std::condition_variable &_newBigBulletsQueueConditionVariable,
                           std::atomic_bool &_game_over, std::queue<BigBullet *> &_newBigBulletsQueue,
                           std::vector<BigBullet *> &_bigActiveBulletsVector) :
        GameObject(_pos_x, _pos_y, 9, 3, _min_x, _max_x, _min_y, _max_y),
        new_big_bullet_condition_variable(_newBigBulletsQueueConditionVariable),
        new_big_adder_bullet_mutex(_newBigBulletsQueueConditionVarMutex),
        game_over(_game_over), new_big_bullets_queue(_newBigBulletsQueue), big_bullets_vector(_bigActiveBulletsVector),
        isBlue(false), died(false) {
    hit_points = 10;
}

void BigAlienShip::drawObject() {
    if (isBlue) {
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        attron(COLOR_PAIR(4));
    }

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

    if (isBlue) {
        attroff(COLOR_PAIR(4));
    }

}

void BigAlienShip::addBigBulletToActiveGame() {
    while (!died) {
        std::unique_lock<std::mutex> locker(new_big_adder_bullet_mutex);
        new_big_bullet_condition_variable.wait(locker, [this] { return (!new_big_bullets_queue.empty()); });
        assert((!new_big_bullets_queue.empty()));

        BigBullet *&bigBullet = new_big_bullets_queue.front();
        bigBullet->setParameters(short(getPos_x() + getWidth() / 2 - 1), short(getPos_y() + 1), 3, 3, 0,
                                 getmaxx(stdscr), 0,
                                 getmaxy(stdscr) + 3);

        new_big_bullets_queue.pop();
        big_bullets_vector.push_back(bigBullet);
        locker.unlock();
    }

    delete &new_big_adder_bullet_mutex;
    delete &new_big_bullet_condition_variable;
    delete &game_over;
    delete[] &new_big_bullets_queue;
    delete[] &big_bullets_vector;
}

std::thread BigAlienShip::startInterceptionOfBulletsThreads() {
    return std::thread(&BigAlienShip::addBigBulletToActiveGame, this);
}

void BigAlienShip::setIsBlue(bool isBlue) {
    BigAlienShip::isBlue = isBlue;
}

void BigAlienShip::setDied(bool died) {
    BigAlienShip::died = died;
}

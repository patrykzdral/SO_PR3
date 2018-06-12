
#include <cassert>
#include "EnemySmall.h"




EnemySmall::EnemySmall(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                       std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
                       std::atomic_bool &game_over, std::queue<SmallBullet *> &new_small_bullets_queue,
                       std::vector<SmallBullet *> &small_bullets_vector):GameActor(_pos_x, _pos_y, 5, 1, _min_x, _max_x, _min_y, _max_y ),
                                                                     new_small_bullet_condition_variable(conditionVariable), new_small_adder_bullet_mutex(conditionVarMutex),
                                                                     game_over(game_over), new_small_bullets_queue(new_small_bullets_queue), small_bullets_vector(small_bullets_vector),isBlue(false),died(false)
{
hit_points = 1;
}

void EnemySmall::drawActor() {
    if(isBlue){
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        attron(COLOR_PAIR(5));
    }
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x+1, "=");
    mvprintw(pos_y, pos_x+2, "=");
    mvprintw(pos_y, pos_x+3, "=");
    mvprintw(pos_y, pos_x+4, "|");
    if(isBlue)
        attroff(COLOR_PAIR(5));

}

void EnemySmall::add_small_bullet_to_active_game() {
    while(!died) {
        std::unique_lock<std::mutex> locker(new_small_adder_bullet_mutex);
        new_small_bullet_condition_variable.wait(locker, [this] { return (!new_small_bullets_queue.empty()); });
        assert((!new_small_bullets_queue.empty()));

        SmallBullet *&smallBullet = new_small_bullets_queue.front();
        smallBullet->setParameters(short(getPos_x() + getWidth() / 2), short(getPos_y()),1,1, 0,
                                   getmaxx(stdscr), 0,
                                   getmaxy(stdscr));

        new_small_bullets_queue.pop();
        small_bullets_vector.push_back(smallBullet);
        locker.unlock();
    }
}

std::thread EnemySmall::startThread() {
    return std::thread(&EnemySmall::add_small_bullet_to_active_game, this);
}

bool EnemySmall::isIsBlue() const {
    return isBlue;
}

void EnemySmall::setIsGreen(bool isBlue) {
    EnemySmall::isBlue = isBlue;
}

bool EnemySmall::isDied() const {
    return died;
}

void EnemySmall::setDied(bool died) {
    EnemySmall::died = died;
}

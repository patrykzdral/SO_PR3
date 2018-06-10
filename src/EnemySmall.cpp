
#include <cassert>
#include "EnemySmall.h"


void EnemySmall::drawActor() {
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x+1, "=");
    mvprintw(pos_y, pos_x+2, "=");
    mvprintw(pos_y, pos_x+3, "=");
    mvprintw(pos_y, pos_x+4, "|");
}

EnemySmall::EnemySmall(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                       std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
                       std::atomic_bool &game_over, std::queue<SmallBullet *> &new_small_bullets_queue,
                       std::vector<SmallBullet *> &big_bullets_vector) :
        GameActor(_pos_x, _pos_y, 5, 1, _min_x, _max_x, _min_y, _max_y),
        new_small_bullet_condition_variable(conditionVariable), new_small_adder_bullet_mutex(conditionVarMutex),
        game_over(game_over), new_small_bullets_queue(new_small_bullets_queue), small_bullets_vector(big_bullets_vector) {
    hit_points = 1;



}

// TODO: TU BYLA ZMIANA
void EnemySmall::add_big_bullet_to_active_game() {

    while(!game_over) {
        std::unique_lock<std::mutex> locker(new_small_adder_bullet_mutex);
        new_small_bullet_condition_variable.wait(locker, [this] { return !new_small_bullets_queue.empty(); });
        assert(!new_small_bullets_queue.empty());

        SmallBullet *&smallBullet = new_small_bullets_queue.front();
        smallBullet->setParameters(short(getPos_x() + getWidth() / 2 - 1), short(getPos_y() + 1), 3, 3, 0,
                                 getmaxx(stdscr), 0,
                                 getmaxy(stdscr) + 3);

        new_small_bullets_queue.pop();
        small_bullets_vector.push_back(smallBullet);
        locker.unlock();
    }
}

// TODO: TU BYLA ZMIANA
std::thread EnemySmall::startThread() {
    return std::thread(&EnemySmall::add_big_bullet_to_active_game, this);
}

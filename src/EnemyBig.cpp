//
// Created by piotrek on 04.06.17.
//

#include <cassert>
#include "EnemyBig.h"
#include "BigBullet.h"

// TODO: TU BYLA ZMIANA
EnemyBig::EnemyBig(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                   std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
                   std::atomic_bool &game_over, std::queue<BigBullet *> &new_big_bullets_queue,
                   std::vector<BigBullet *> &big_bullets_vector) :
        GameActor(_pos_x, _pos_y, 9, 3, _min_x, _max_x, _min_y, _max_y),
        new_big_bullet_condition_variable(conditionVariable), new_big_adder_bullet_mutex(conditionVarMutex),
        game_over(game_over), new_big_bullets_queue(new_big_bullets_queue), big_bullets_vector(big_bullets_vector),isBlue(false) {
    hit_points = 10;
}

void EnemyBig::drawActor() {
    if(isBlue){
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        attron(COLOR_PAIR(4));
    }

    //first pos_y
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x + 1, "_");
    mvprintw(pos_y, pos_x + 2, "_");
    mvprintw(pos_y, pos_x + 3, "_");
    mvprintw(pos_y, pos_x + 4, "_");
    mvprintw(pos_y, pos_x + 5, "_");
    mvprintw(pos_y, pos_x + 6, "_");
    mvprintw(pos_y, pos_x + 7, "_");
    mvprintw(pos_y, pos_x + 8, "|");

    //second pos_y
    mvprintw(pos_y + 1, pos_x, "|");
    mvprintw(pos_y + 1, pos_x + 1, "_");
    mvprintw(pos_y + 1, pos_x + 2, "_");
    mvprintw(pos_y + 1, pos_x + 3, "_");
    mvprintw(pos_y + 1, pos_x + 4, "_");
    mvprintw(pos_y + 1, pos_x + 5, "_");
    mvprintw(pos_y + 1, pos_x + 6, "_");
    mvprintw(pos_y + 1, pos_x + 7, "_");
    mvprintw(pos_y + 1, pos_x + 8, "|");
    // third pos_y
    mvprintw(pos_y + 2, pos_x, "|");
    mvprintw(pos_y + 2, pos_x + 8, "|");
    if(isBlue) {
        attroff(COLOR_PAIR(4));
    }

}

// TODO: TU BYLA ZMIANA
void EnemyBig::add_big_bullet_to_active_game() {

    while(!game_over) {
        std::unique_lock<std::mutex> locker(new_big_adder_bullet_mutex);
        new_big_bullet_condition_variable.wait(locker, [this] { return !new_big_bullets_queue.empty(); });
        assert(!new_big_bullets_queue.empty());

        BigBullet *&bigBullet = new_big_bullets_queue.front();
        bigBullet->setParameters(short(getPos_x() + getWidth() / 2 - 1), short(getPos_y() + 1), 3, 3, 0,
                                 getmaxx(stdscr), 0,
                                 getmaxy(stdscr) + 3);

        new_big_bullets_queue.pop();
        big_bullets_vector.push_back(bigBullet);
        locker.unlock();
    }
}

// TODO: TU BYLA ZMIANA
std::thread EnemyBig::startThread() {
    return std::thread(&EnemyBig::add_big_bullet_to_active_game, this);
}

bool EnemyBig::isIsBlue() const {
    return isBlue;
}

void EnemyBig::setIsBlue(bool isBlue) {
    EnemyBig::isBlue = isBlue;
}

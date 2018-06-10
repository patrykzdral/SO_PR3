//
// Created by patrykzdral on 10.06.18.
//

#include <assert.h>
#include "EnemyBigObserver.h"

std::mutex EnemyBigObserver::mutex;

// TODO: TU BYLA ZMIANA
void EnemyBigObserver::add_big_bullet_to_active_game() {

        while(!game_over) {
            std::unique_lock<std::mutex> uniqueLock(mutex);
            new_big_bullet_condition_variable.wait(uniqueLock, [this] { return !new_big_bullets_queue.empty(); });

            BigBullet *&bigBullet = new_big_bullets_queue.front();
            bigBullet->setParameters(short(this->gameActor->getPos_x() + this->gameActor->getWidth() / 2 - 1),
                                     short(this->gameActor->getPos_y() + 1), 3, 3, 0,
                                     getmaxx(stdscr), 0,
                                     getmaxy(stdscr) + 3);

            new_big_bullets_queue.pop();
            big_bullets_vector.push_back(bigBullet);
            uniqueLock.unlock();
        }
}

// TODO: TU BYLA ZMIANA
std::thread EnemyBigObserver::startThread() {
    return std::thread(&EnemyBigObserver::add_big_bullet_to_active_game, this);
}

EnemyBigObserver::EnemyBigObserver(GameActor *&enemyBig, std::mutex &conditionVarMutex,
                                   std::condition_variable &conditionVariable, std::atomic_bool &game_over,
                                   std::queue<BigBullet *> &new_big_bullets_queue,
                                   std::vector<BigBullet *> &big_bullets_vector) : new_big_bullet_condition_variable(
        conditionVariable), new_big_adder_bullet_mutex(conditionVarMutex),
                                                                                   game_over(game_over),
                                                                                   new_big_bullets_queue(
                                                                                           new_big_bullets_queue),
                                                                                   big_bullets_vector(
                                                                                           big_bullets_vector) {
    this->gameActor = enemyBig;

}
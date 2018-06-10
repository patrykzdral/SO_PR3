//
// Created by patrykzdral on 10.06.18.
//

#ifndef SO_PR3_ENEMYBIGOBSERVER_H
#define SO_PR3_ENEMYBIGOBSERVER_H

#include <ncurses.h>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <thread>
#include "Direction.h"
#include "GameActor.h"
#include "BigBullet.h"
#include "EnemyBig.h"

class EnemyBigObserver {

private:
    // TODO: TU BYLA ZMIANA
    std::mutex &new_big_adder_bullet_mutex;
    static std::mutex mutex;

    std::condition_variable &new_big_bullet_condition_variable;
    std::atomic_bool &game_over;
    std::queue<BigBullet *> &new_big_bullets_queue;
    std::vector<BigBullet *> &big_bullets_vector;
    GameActor * gameActor;

public:
    EnemyBigObserver(
             GameActor*& enemyBig, std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
             std::atomic_bool &game_over, std::queue<BigBullet *> &new_big_bullets_queue,
             std::vector<BigBullet *> &big_bullets_vector);
    // TODO: TU BYLA ZMIANA
    void add_big_bullet_to_active_game();

    // TODO: TU BYLA ZMIANA
    std::thread startThread();};


#endif //SO_PR3_ENEMYBIGOBSERVER_H

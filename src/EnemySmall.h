

#ifndef SPACE_INVADERS_ENEMY_SMALL_FAST_H
#define SPACE_INVADERS_ENEMY_SMALL_FAST_H

#include <ncurses.h>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <thread>
#include "Direction.h"
#include "GameActor.h"
#include "SmallBullet.h"


class EnemySmall : public GameActor {
private:
    // TODO: TU BYLA ZMIANA
    std::mutex &new_small_adder_bullet_mutex;
    std::condition_variable &new_small_bullet_condition_variable;
    std::atomic_bool &game_over;
    std::queue<SmallBullet *> &new_small_bullets_queue;
    std::vector<SmallBullet *> &small_bullets_vector;
public:
    EnemySmall(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
               std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
               std::atomic_bool &game_over, std::queue<SmallBullet *> &new_big_bullets_queue,
               std::vector<SmallBullet *> &big_bullets_vector);
    void drawActor();

    // TODO: TU BYLA ZMIANA
    void add_small_bullet_to_active_game();

    // TODO: TU BYLA ZMIANA
    std::thread startThread();
};


#endif //SPACE_INVADERS_ENEMY_SMALL_FAST_H

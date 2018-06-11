

#ifndef SPACE_INVADERS_ENEMY_BIG_SLOW_H
#define SPACE_INVADERS_ENEMY_BIG_SLOW_H

#include <ncurses.h>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <thread>
#include "Direction.h"
#include "GameActor.h"
#include "BigBullet.h"

class EnemyBig : public GameActor {
private:
    // TODO: TU BYLA ZMIANA
    std::mutex &new_big_adder_bullet_mutex;
    std::condition_variable &new_big_bullet_condition_variable;
    std::atomic_bool &game_over;
    std::queue<BigBullet *> &new_big_bullets_queue;
    std::vector<BigBullet *> &big_bullets_vector;
    bool isBlue;
public:
    EnemyBig(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
             std::mutex &conditionVarMutex, std::condition_variable &conditionVariable,
             std::atomic_bool &game_over, std::queue<BigBullet *> &new_big_bullets_queue,
             std::vector<BigBullet *> &big_bullets_vector);
    void drawActor();

    // TODO: TU BYLA ZMIANA
    void add_big_bullet_to_active_game();

    // TODO: TU BYLA ZMIANA
    std::thread startThread();

    bool isIsBlue() const;

    void setIsBlue(bool isBlue);
};


#endif //SPACE_INVADERS_ENEMY_BIG_SLOW_H

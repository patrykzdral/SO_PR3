

#ifndef SPACE_INVADERS_ENEMY_SMALL_FAST_H
#define SPACE_INVADERS_ENEMY_SMALL_FAST_H

#include <ncurses.h>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <thread>
#include "Direction.h"
#include "GameObject.h"
#include "SmallBullet.h"

class SmallAlienShip : public GameObject {
private:
    std::mutex &newSmallBulletsQueueConditionVarMutex;
    std::condition_variable &newSmallBulletsQueueConditionVariable;
    std::atomic_bool &game_over;
    std::queue<SmallBullet *> &newSmallBulletsQueue;
    std::vector<SmallBullet *> &smallActiveBulletsVector;
public:
    SmallAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                   std::mutex &_newSmallBulletsQueueConditionVarMutex,
                   std::condition_variable &_newSmallBulletConditionVariable,
                   std::atomic_bool &_game_over, std::queue<SmallBullet *> &_newSmallBulletsQueue,
                   std::vector<SmallBullet *> &_big_bullets_vector);

    void drawObject() override;

    void addSmallBulletToActiveGame();

    std::thread startInterceptionOfBulletsThreads();
};


#endif //SPACE_INVADERS_ENEMY_SMALL_FAST_H

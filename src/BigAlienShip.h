

#ifndef SPACE_INVADERS_ENEMY_BIG_SLOW_H
#define SPACE_INVADERS_ENEMY_BIG_SLOW_H

#include <ncurses.h>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <thread>
#include "Direction.h"
#include "GameObject.h"
#include "BigBullet.h"

class BigAlienShip : public GameObject {
private:
    std::mutex &newBigBulletsQueueConditionVarMutex;
    std::condition_variable &newBigBulletsQueueConditionVariable;
    std::atomic_bool &game_over;
    std::queue<BigBullet *> &newBigBulletsQueue;
    std::vector<BigBullet *> &bigActiveBulletsVector;
    public:
    BigAlienShip(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y,
                 std::mutex &_newBigBulletsQueueConditionVarMutex,
                 std::condition_variable &_newBigBulletsQueueConditionVariable,
                 std::atomic_bool &_game_over, std::queue<BigBullet *> &_newBigBulletsQueue,
                 std::vector<BigBullet *> &_bigActiveBulletsVector);

    void drawObject() override;

    void addBigBulletToActiveGame();

    std::thread startInterceptionOfBulletsThreads();
};


#endif //SPACE_INVADERS_ENEMY_BIG_SLOW_H



#ifndef SPACE_INVADERS_ENEMY_BIG_SLOW_H
#define SPACE_INVADERS_ENEMY_BIG_SLOW_H

#include <ncurses.h>
#include "Direction.h"
#include "GameActor.h"

class EnemyBig : public GameActor {
public:
    EnemyBig(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_ENEMY_BIG_SLOW_H



#ifndef SPACE_INVADERS_ENEMY_SMALL_FAST_H
#define SPACE_INVADERS_ENEMY_SMALL_FAST_H

#include <ncurses.h>
#include "GameActor.h"

class EnemySmall : public GameActor {
public:
    EnemySmall(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_ENEMY_SMALL_FAST_H

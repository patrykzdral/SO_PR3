

#ifndef SPACE_INVADERS_BULLET_H
#define SPACE_INVADERS_BULLET_H

#include <ncurses.h>
#include "GameActor.h"

class SmallBullet : public GameActor{

public:
    SmallBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    SmallBullet();

    void drawActor();
};



#endif //SPACE_INVADERS_BULLET_H

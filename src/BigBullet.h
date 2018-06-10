

#ifndef SPACE_INVADERS_BIGBULLET_H
#define SPACE_INVADERS_BIGBULLET_H

#include <ncurses.h>
#include "GameActor.h"

class BigBullet : public GameActor{
public:
    BigBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);
    void drawActor();
};


#endif //SPACE_INVADERS_BIGBULLET_H

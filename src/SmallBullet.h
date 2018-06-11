

#ifndef SPACE_INVADERS_BULLET_H
#define SPACE_INVADERS_BULLET_H

#include <ncurses.h>
#include "GameObject.h"

class SmallBullet : public GameObject {

public:
    SmallBullet(short _pos_x, short _pos_y, int _min_x, int _max_x, int _min_y, int _max_y);

    SmallBullet();

    void drawObject() override;
};


#endif //SPACE_INVADERS_BULLET_H

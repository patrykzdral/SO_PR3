

#ifndef SPACE_INVADERS_BIGBULLET_H
#define SPACE_INVADERS_BIGBULLET_H

#include <ncurses.h>
#include "GameObject.h"

class BigBullet : public GameObject{
public:
    BigBullet();

    void drawObject();
};


#endif //SPACE_INVADERS_BIGBULLET_H

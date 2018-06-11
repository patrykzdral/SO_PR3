
#include "BigBullet.h"

void BigBullet::drawObject() {
    mvprintw(pos_y, pos_x+1, "x");
    mvprintw(pos_y+1, pos_x, "x");
    mvprintw(pos_y+1, pos_x+1, "x");
    mvprintw(pos_y+1, pos_x+2, "x");
    mvprintw(pos_y+2, pos_x+1, "x");
}

BigBullet::BigBullet(): GameObject(){

}

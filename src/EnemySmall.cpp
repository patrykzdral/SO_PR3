
#include "EnemySmall.h"

EnemySmall::EnemySmall(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y) :
    GameActor(_pos_x, _pos_y, 5, 1, _min_x, _max_x, _min_y, _max_y ){
    hit_points = 1;
}


void EnemySmall::drawActor() {
    mvprintw(pos_y, pos_x, "|");
    mvprintw(pos_y, pos_x+1, "=");
    mvprintw(pos_y, pos_x+2, "=");
    mvprintw(pos_y, pos_x+3, "=");
    mvprintw(pos_y, pos_x+4, "|");
}


#include "Player.h"

Player::Player(int _pos_x, int _pos_y, int _min_x, int _max_x, int _min_y, int _max_y)
    : GameActor(_pos_x, _pos_y, 7, 1, _min_x, _max_x, _min_y, _max_y){
    hit_points = 100;
}


void Player::drawActor() {
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    attron(COLOR_PAIR(3));
    mvprintw(pos_y, pos_x,"|");
    mvprintw(pos_y, pos_x+1, "_");
    mvprintw(pos_y, pos_x+2, "_");
    mvprintw(pos_y, pos_x+3, "&");
    mvprintw(pos_y, pos_x+4, "_");
    mvprintw(pos_y, pos_x+5, "_");
    mvprintw(pos_y, pos_x+6, "|");
    attroff(COLOR_PAIR(3));

}


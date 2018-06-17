
#include "GameObject.h"

GameObject::GameObject(int _pos_x, int _pos_y, int _width, int _height, int _min_x, int _max_x, int _min_y,
                       int _max_y) {
    pos_x = _pos_x;
    pos_y = _pos_y;
    width = _width;
    height = _height;
    min_x = _min_x;
    max_x = _max_x;
    min_y = _min_y;
    max_y = _max_y;
}


GameObject::GameObject() = default;


void GameObject::move(int move_x, int move_y) {

    if (!done) {
        if (move_x < 0) {

            if (pos_x + move_x >= min_x) {
                pos_x += move_x;
            }
        } else {
            if (pos_x + width + move_x <= max_x) {
                pos_x += move_x;
            }
        }
        if (move_y < 0) {
            if (pos_y + move_y < min_y) {
                done = true;
            }
            pos_y += move_y;
        } else {
            if (pos_y + height + move_y > max_y) {
                done = true;
            }
            pos_y += move_y;
        }
    }
}

int GameObject::getMax_x() const {
    return max_x;
}

int GameObject::getMin_x() const {
    return min_x;
}

int GameObject::getMax_y() const {
    return max_y;
}

int GameObject::getMin_y() const {
    return min_y;
}

int GameObject::getPos_x() const {
    return pos_x;
}

int GameObject::getPos_y() const {
    return pos_y;
}

int GameObject::getHit_points() const {
    return hit_points;
}

void GameObject::setDamage(int hp) {
    hit_points -= hp;
    if (hit_points <= 0) {
        done = true;
        hit_points = 0;
    }
}

void GameObject::setParameters(int _pos_x, int _pos_y, int _width, int _height, int _min_x, int _max_x, int _min_y,
                               int _max_y) {
    pos_x = _pos_x;
    pos_y = _pos_y;
    width = _width;
    height = _height;
    min_x = _min_x;
    max_x = _max_x;
    min_y = _min_y;
    max_y = _max_y;
}




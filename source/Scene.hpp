//
// Created by ljh on 25. 9. 12..
//

#ifndef MYPROJECT_SCENE_H
#define MYPROJECT_SCENE_H
#include <memory>
#include <vector>

#include "geometry.h"


class Scene{
public:
void add(Hittable& object);
void remove(Hittable& object);



private:
std::vector<std::unique_ptr<Hittable>> objects;

};


#endif //MYPROJECT_SCENE_H
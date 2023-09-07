#pragma once
#include <vector>
#include "Mesh.h"

class Scene
{
public:
    
    std::vector<Mesh*> meshs;

    void add(Mesh* mesh);
    void render();
};
#pragma once
#include <vector>
#include "Mesh.h"

class PSO;

class Scene
{
public:
    PSO* currPSO{nullptr};
    std::vector<Mesh*> meshs;

    void add(Mesh* mesh);
    void render();
    void reset();
};
#pragma once
#include "Geometry.h"
#include "Material.h"
#include "Transform.h"

class Mesh
{
public:
    Geometry* geom{nullptr};
    Material* mat{nullptr};
    Transform* trans{nullptr};

    Mesh();
    void prepare();
    void update();
    void draw();
};
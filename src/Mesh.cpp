#include "Mesh.h"

Mesh::Mesh()
{
    geom = new Geometry();
    mat = new Material();
    trans = new Transform();
}

void Mesh::prepare()
{
    geom->prepare();
}

void Mesh::draw()
{
    geom->draw();
}


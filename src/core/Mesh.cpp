#include "Mesh.h"

Mesh::Mesh()
{
    geom = new Geometry();
    trans = new Transform();
}

void Mesh::prepare()
{
    geom->prepare();
}

void Mesh::update()
{
    trans->update();
}

void Mesh::draw()
{
    mat->apply();
    geom->draw();
}


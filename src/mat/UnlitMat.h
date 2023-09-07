#pragma once
#include "Material.h"
#include "UnlitPso.h"
#include "Texture.h"

class UnlitMat : public Material
{
public:
    static UnlitPso* pso;

    Texture* diffuse;
    UnlitMat(LPCWSTR filename);

    virtual void apply();
};
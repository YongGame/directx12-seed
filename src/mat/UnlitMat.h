#pragma once
#include "Material.h"
#include "Texture.h"

class UnlitMat : public Material
{
public:
    Texture* diffuse;
    UnlitMat(LPCWSTR filename);

    void initPSO();
    virtual void apply();
};
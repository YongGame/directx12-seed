#pragma once
#include <string>
#include <d3dx12.h>
#include "core/PSO.h"

class Material
{
public:
    std::string name;
    
    PSO* pso{nullptr};

    virtual void apply()=0;
};
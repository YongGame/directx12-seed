#pragma once
#include <string>
#include <d3dx12.h>

class Material
{
public:
    std::string name;
    ID3D12PipelineState* pipelineStateObject;

    virtual void apply()=0;
};
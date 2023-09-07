#pragma once
#include "Material.h"
#include "Texture.h"

class UnlitMat : public Material
{
public:
    static ID3D12RootSignature* rootSignature;
    static ID3D12PipelineState* pipelineStateObject;

    Texture* diffuse;
    UnlitMat(LPCWSTR filename);

    void initPSO();
    virtual void apply();
};
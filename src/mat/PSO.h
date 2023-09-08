#pragma once
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include "d3dx12.h"

class Shader;

class PSO
{
public:
    static std::unordered_map<std::string, PSO*> psos;

    ID3D12RootSignature* rootSignature{nullptr};
    ID3D12PipelineState* pipelineStateObject{nullptr};
    
    PSO();
    void build(Shader* shader, CD3DX12_ROOT_SIGNATURE_DESC& rootSignatureDesc);

    static PSO* get(std::string name);
    
    DXGI_SAMPLE_DESC sampleDesc{};
    D3D12_INPUT_ELEMENT_DESC inputLayout[3];
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
};
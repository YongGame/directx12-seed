#pragma once
#include <d3dx12.h>

class UnlitPso
{
public:
    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineStateObject;
    
    UnlitPso();
};
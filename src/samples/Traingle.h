#pragma once
#include "Sample.h"
#include "DX.h"

using namespace DirectX;

struct Vertex {
    XMFLOAT3 pos;
};

class Traingle : public Sample
{
public:
    ID3D12PipelineState* pipelineStateObject;
    ID3D12RootSignature* rootSignature;

    

    ID3D12Resource* vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    Traingle(DX &dx);
    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();
};
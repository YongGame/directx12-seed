#pragma once
#include "Sample.h"
#include "DX.h"

using namespace DirectX;

struct Vertex {
    Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x,y,z), color(r,g,b,a) {}
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

class Traingle : public Sample
{
public:
    ID3D12PipelineState* pipelineStateObject;
    ID3D12RootSignature* rootSignature;
    
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView2;
    D3D12_INDEX_BUFFER_VIEW indexBufferView2; // a structure holding information about the index buffer

    Traingle(DX &dx);
    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();
    
    void initPSO();
    void initMesh();
};
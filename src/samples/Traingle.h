#pragma once
#include "Sample.h"
#include "DX.h"

using namespace DirectX;

// this is the structure of our constant buffer.
struct ConstantBuffer {
    XMFLOAT4 colorMultiplier;
};

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

    // frameBufferCount = 3
    ID3D12DescriptorHeap* mainDescriptorHeap[3]; // this heap will store the descripor to our constant buffer
    ID3D12Resource* constantBufferUploadHeap[3]; // this is the memory on the gpu where our constant buffer will be placed.
    UINT8* cbColorMultiplierGPUAddress[3]; // this is a pointer to the memory location we get when we map our constant buffer
    ConstantBuffer cbColorMultiplierData; // this is the constant buffer data we will send to the gpu 
                                        // (which will be placed in the resource we created above)

    Traingle(DX &dx);
    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();

    void initRootSignature();
    void initPSO();
    void initMesh();
    void initCBV();
};
#pragma once
#include "Sample.h"
#include "DX.h"

class Texture;

using namespace DirectX;

// this is the structure of our constant buffer.
struct ConstantBuffer {
    XMFLOAT4 colorMultiplier;
};

// this is the structure of our constant buffer.
struct ConstantBufferPerObject {
    XMFLOAT4X4 wvpMat;
};

struct Vertex {
    Vertex(float x, float y, float z, float u, float v, float r, float g, float b, float a) : pos(x,y,z), uv(u,v), color(r,g,b,a) {}
    XMFLOAT3 pos;
    XMFLOAT2 uv;
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

    
    int CBV_SRV_UAV_DescriptorSize;
    ID3D12DescriptorHeap* CBV_SRV_UAV_DescriptorHeap; 

    // frameBufferCount = 3
    ID3D12Resource* constantBufferUploadHeap[3]; // this is the memory on the gpu where our constant buffer will be placed.
    UINT8* cbColorMultiplierGPUAddress[3]; // this is a pointer to the memory location we get when we map our constant buffer
    ConstantBuffer cbColorMultiplierData; // this is the constant buffer data we will send to the gpu 
                                        // (which will be placed in the resource we created above)


    // Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
    // We are only able to read at 256 byte intervals from the start of a resource heap, so we will
    // make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
    // Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
    // we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
    // The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
    // buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
    // were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
    // being copied.
    int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

    ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu 
                                            // (which will be placed in the resource we created above)

    ID3D12Resource* constantBufferUploadHeaps[3]; // this is the memory on the gpu where constant buffers for each frame will be placed

    UINT8* cbvGPUAddress[3]; // this is a pointer to each of the constant buffer resource heaps

    XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
    XMFLOAT4X4 cameraViewMat; // this will store our view matrix

    XMFLOAT4 cameraPosition; // this is our cameras position vector
    XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
    XMFLOAT4 cameraUp; // the worlds up vector

    XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
    XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
    XMFLOAT4 cube1Position; // our first cubes position in space

    XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
    XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
    XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

    int numCubeIndices; // the number of indices to draw the cube

    Texture* tex;

    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();

    void initRootSignature();
    void initPSO();
    void initMesh();
    void initCBV();
};
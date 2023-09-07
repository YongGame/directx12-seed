#pragma once
#include "Sample.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Shader.h"

#include "DX.h"

class Texture;

using namespace DirectX;

// this is the structure of our constant buffer.
struct ConstantBufferPerObject {
    XMFLOAT4X4 modelMat;
};

struct CameraConstantBuffer{
    XMFLOAT4X4 projMat;
    XMFLOAT4X4 viewMat;
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
    Camera* camera;

    Mesh* quad;
    Mesh* tri;
    Shader* shader;

    // frameBufferCount = 3
    ID3D12Resource* cameraBufferRes[3]; // this is the memory on the gpu where our constant buffer will be placed.
    UINT8* cameraBufferResAddress[3]; // this is a pointer to the memory location we get when we map our constant buffer
    CameraConstantBuffer cameraBufferData;


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

    ID3D12Resource* objBufferRes[3]; // this is the memory on the gpu where constant buffers for each frame will be placed

    UINT8* objBufferResAddress[3]; // this is a pointer to each of the constant buffer resource heaps

    virtual void init();
    virtual void Update();
    virtual void UpdatePipeline();
    virtual void resize();
    
    void initMesh();
    void initCBV();
};
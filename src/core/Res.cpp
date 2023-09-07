#include "Res.h"
#include "../DX.h"

void Res::CreateCommittedResource_UPLOAD(LPTSTR name, int size)
{
    DX::dx->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(size), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
        D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
        nullptr, // we do not have use an optimized clear value for constant buffers
        IID_PPV_ARGS(&res));

    res->SetName(name);

    CD3DX12_RANGE readRange(0, 0);
    res->Map(0, &readRange, reinterpret_cast<void**>(&resAddress));

    gpuAddress = res->GetGPUVirtualAddress();
}
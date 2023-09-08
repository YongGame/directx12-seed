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

void Res::CreateCommittedResource_DEFAULT(LPTSTR name, int size, const void *data)
{
    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    DX::dx->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
        D3D12_HEAP_FLAG_NONE, 
        &CD3DX12_RESOURCE_DESC::Buffer(size), 
        D3D12_RESOURCE_STATE_COPY_DEST, 
        nullptr,
        IID_PPV_ARGS(&res));
    res->SetName(name);

    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    ID3D12Resource* uploadRes;
    DX::dx->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, 
        &CD3DX12_RESOURCE_DESC::Buffer(size), 
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&uploadRes));
    uploadRes->SetName(L"UpLoadBufferRes");

    D3D12_SUBRESOURCE_DATA sub = {};
    sub.pData = data; // pointer to our vertex array
    sub.RowPitch = size; // size of all our triangle vertex data
    sub.SlicePitch = size; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    UpdateSubresources(DX::dx->commandList, res, uploadRes, 0, 0, 1, &sub);

    gpuAddress = res->GetGPUVirtualAddress();
}
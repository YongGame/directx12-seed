#pragma once
#include <d3d12.h>

class Res
{
public:
    ID3D12Resource* res;
    UINT8* resAddress;
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;

    void CreateCommittedResource_UPLOAD(LPTSTR name, int size);
};
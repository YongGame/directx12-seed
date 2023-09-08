#pragma once
#include <d3d12.h>

class Res
{
public:
    ID3D12Resource* res;
    UINT8* resAddress; // 这个貌似是 cpuAddress ，用来做资源更新
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;

    // 针对每帧改变的数据，放在 UPLOAD
    void CreateCommittedResource_UPLOAD(LPTSTR name, int size);
    // 针对不变数据，比如 顶点数据 静态纹理，放在 DEFAULT
    void CreateCommittedResource_DEFAULT(LPTSTR name, int size, const void *data);
};
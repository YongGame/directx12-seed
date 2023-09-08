#pragma once
#include "Res.h"

class TextureRes : public Res
{
public:
    // 纹理需要创建描述符
    // g_CPU_CBV_SRV_UAV_DescriptorHeap 描述符堆的描述符句柄， 句柄是对资源的描述
    int handleIndex;

    void create(LPTSTR name, D3D12_RESOURCE_DESC &textureDesc, BYTE *imageData, int imageBytesPerRow);
};
#pragma once
#include "Res.h"

class TextureRes : public Res
{
public:
    // g_CPU_CBV_SRV_UAV_DescriptorHeap 描述符堆的描述符句柄
    int handleIndex;
    
    void create(LPTSTR name, D3D12_RESOURCE_DESC &textureDesc, BYTE *imageData, int imageBytesPerRow);
};
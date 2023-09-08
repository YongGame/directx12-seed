#pragma once
#include "Res.h"

class TextureRes : public Res
{
public:
    // 纹理需要通过 描述符表 使用， 描述符表需要指定对应的描述符句柄
    // g_CPU_CBV_SRV_UAV_DescriptorHeap 这个是GPU不可见的描述符堆中分配的句柄索引，在绘制之前会将句柄拷贝到GPU可见的描述符堆
    int handleIndex;

    void create(LPTSTR name, D3D12_RESOURCE_DESC &textureDesc, BYTE *imageData, int imageBytesPerRow);
};
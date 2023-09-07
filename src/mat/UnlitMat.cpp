#include "UnlitMat.h"
#include "DX.h"

UnlitPso* UnlitMat::pso = nullptr;

UnlitMat::UnlitMat(LPCWSTR filename)
{
    if(!pso) pso = new UnlitPso();
    diffuse = new Texture(L"D://cc/directx12-seed/assets/braynzar.jpg");
}

void UnlitMat::apply()
{
    //DX::dx->device->CopyDescriptorsSimple(1, DX::dx->getGpuDescHandle(), DX::dx->getDescHandle(diffuse->handleIndex), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //CD3DX12_GPU_DESCRIPTOR_HANDLE handle(CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 3, CBV_SRV_UAV_DescriptorSize);
    // getGpuHandle 这个方法会将cpu描述符堆的句柄 拷贝到 gpu描述符堆，顺便返回gpu描述符的GPU描述符句柄
    DX::dx->commandList->SetGraphicsRootDescriptorTable(2, DX::dx->getGpuHandle(diffuse->handleIndex));
}
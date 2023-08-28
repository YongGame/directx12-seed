#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

class DX{
public:
    int frameIndex;
    const static int frameBufferCount = 3;
    int rtvDescriptorSize;
    
    ID3D12Device* device;
    IDXGISwapChain3* swapChain;
    ID3D12CommandQueue* commandQueue;
    ID3D12GraphicsCommandList* commandList;

    ID3D12DescriptorHeap* rtvDescriptorHeap;
    ID3D12Resource* renderTargets[frameBufferCount];
    ID3D12CommandAllocator* commandAllocator[frameBufferCount];
    ID3D12Fence* fence[frameBufferCount];
    HANDLE fenceEvent;
    UINT64 fenceValue[frameBufferCount];


    

    bool init(HWND hwnd, int w, int h, bool fullScene);

};
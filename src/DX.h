#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include "Help.h"

class Sample;

class DX{
public:
    Sample* sample;
    
    int frameIndex;
    const static int frameBufferCount = 3;
    int rtvDescriptorSize;

    IDXGIFactory4* dxgiFactory;
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

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;
    DXGI_SAMPLE_DESC sampleDesc{};
    

    void init(HWND hwnd, int w, int h, bool fullScene);
    void Update();
    void Render();
    void UpdatePipeline();
    void WaitForPreviousFrame();
    void destory();

private:
    void createDevice();
    void createQueue();
    void createSwapChain(HWND hwnd, int w, int h, bool fullScene);
	void createRTV();
	void createCmdList();
	void createFence();
};
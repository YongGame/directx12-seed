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
    int width;
    int height;

    Sample* sample;
    
    IDXGIFactory4* dxgiFactory;
    ID3D12Device* device;
    IDXGISwapChain3* swapChain;
    ID3D12CommandQueue* commandQueue;
    ID3D12GraphicsCommandList* commandList;

    int rtvDescriptorSize;
    ID3D12DescriptorHeap* rtvDescriptorHeap;

    ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
    ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

    int frameIndex;
    const static int frameBufferCount = 3;
    ID3D12Resource* renderTargets[frameBufferCount];
    ID3D12CommandAllocator* commandAllocator[frameBufferCount];
    ID3D12Fence* fence[frameBufferCount];
    UINT64 fenceValue[frameBufferCount];
    HANDLE fenceEvent;

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;
    DXGI_SAMPLE_DESC sampleDesc{};

    ID3D12PipelineState* pipelineStateObject;
    

    void init(HWND hwnd, int w, int h, bool fullScene);
    void Update();
    void Render();
    void UpdatePipeline();
    void WaitForPreviousFrame();
    void destory();

    D3D12_SHADER_BYTECODE createShader(LPCWSTR pFileName, LPCSTR pTarget);
    D3D12_VERTEX_BUFFER_VIEW createVertexBuffer(int vBufferSize, int strideInBytes, const void * pData);
    D3D12_INDEX_BUFFER_VIEW createIndexBuffer(int iBufferSize, const void * pData);
    void uploadBuffer();
private:
    void initDevice();
    void initQueue();
    void initSwapChain(HWND hwnd, bool fullScene);
	void initRTV();
    void initDSV();
	void initCmdList();
	void initFence();
};
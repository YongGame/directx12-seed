#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include "Help.h"

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

class Sample;

class DX{
public:
    static DX* dx;
    int width{0};
    int height{0};

    int frameIndex;
    const static int frameBufferCount = 3;

    Sample* sample;
    
    IDXGIFactory4* dxgiFactory;
    ID3D12Device* device;
    IDXGISwapChain3* swapChain;
    ID3D12CommandQueue* commandQueue;
    ID3D12GraphicsCommandList* commandList;

    // rtv
    ID3D12DescriptorHeap* g_rtv_desc_heap; // 描述符堆
    D3D12_CPU_DESCRIPTOR_HANDLE g_rtv_desc_handle[frameBufferCount]{}; // 描述符句柄
    ID3D12Resource* g_rtv_res[frameBufferCount]{}; // 资源

    // dsv
    ID3D12Resource* depthStencilBuffer{nullptr}; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
    ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

    ID3D12CommandAllocator* commandAllocator[frameBufferCount];
    ID3D12Fence* fence[frameBufferCount];
    UINT64 fenceValue[frameBufferCount];
    HANDLE fenceEvent;

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;

    ID3D12PipelineState* pipelineStateObject;
    

    void init(HWND hwnd, int w, int h, bool fullScene);
    void Update();
    void Render();
    void UpdatePipeline();
    void WaitForPreviousFrame();
    void WaitForLastSubmittedFrame(int index);
    void resize(int w, int h);
    void destory();

    
    D3D12_SHADER_BYTECODE createShader(LPCWSTR pFileName, LPCSTR pTarget);
    D3D12_VERTEX_BUFFER_VIEW createVertexBuffer(int vBufferSize, int strideInBytes, const void * pData);
    D3D12_INDEX_BUFFER_VIEW createIndexBuffer(int iBufferSize, const void * pData);
    void uploadRes();

    void init_RTV_DESC_HEAP();
    void init_DSV_DESC_HEAP();
    void create_RTV_RES();
    void create_DSV_RES();
    
private:
    void initDevice();
    void initQueue();
    void initSwapChain(HWND hwnd, bool fullScene);
	void initCmdList();
	void initFence();
};
#include "dx.h"
#include "samples/Sample.h"

DX* DX::dx = nullptr;

void DX::init(HWND hwnd, int w, int h, bool fullScene)
{
	dx = this;
	width = w;
	height = h;

	// [DEBUG] Enable debug interface
	#ifdef DX12_ENABLE_DEBUG_LAYER
		ID3D12Debug* pdx12Debug = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
			pdx12Debug->EnableDebugLayer();
	#endif

	initDevice();

	// [DEBUG] Setup debug interface to break on any warnings/errors
	#ifdef DX12_ENABLE_DEBUG_LAYER
		if (pdx12Debug != nullptr)
		{
			ID3D12InfoQueue* pInfoQueue = nullptr;
			device->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			pInfoQueue->Release();
			pdx12Debug->Release();
		}
	#endif

	initQueue();
	initSwapChain(hwnd, fullScene);
	init_RTV_DESC_HEAP(); 
	init_DSV_DESC_HEAP();
	initCmdList();
	initFence();

	initDescHeap();

	sample->init();

	// Fill out the Viewport
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = float(w);
    viewport.Height = float(h);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // Fill out a scissor rect
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = w;
    scissorRect.bottom = h;
}

void DX::initDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC cpuHeapDesc = {};
    cpuHeapDesc.NumDescriptors = 1000;
    cpuHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    cpuHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // GPU不可见
    dx->device->CreateDescriptorHeap(&cpuHeapDesc, IID_PPV_ARGS(&g_CPU_CBV_SRV_UAV_DescriptorHeap));
    CBV_SRV_UAV_DescriptorSize = dx->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for(int i=999; i>=0; i--) g_CPU_CBV_SRV_UAV_DescriptorPool.push_back(i);

	D3D12_DESCRIPTOR_HEAP_DESC gpuHeapDesc = {};
    gpuHeapDesc.NumDescriptors = 1000;
    gpuHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // GPU可见
    gpuHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    dx->device->CreateDescriptorHeap(&gpuHeapDesc, IID_PPV_ARGS(&g_GPU_CBV_SRV_UAV_DescriptorHeap));
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DX::getGpuHandle(int cpuIndex)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(g_CPU_CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), cpuIndex, CBV_SRV_UAV_DescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE gpuHandle(g_GPU_CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), gpuHandleIndex, CBV_SRV_UAV_DescriptorSize);
	device->CopyDescriptorsSimple(1, gpuHandle, cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(g_GPU_CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), gpuHandleIndex, CBV_SRV_UAV_DescriptorSize);
	gpuHandleIndex++;
	return handle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX::getDescHandle(int idx)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(g_CPU_CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), idx, CBV_SRV_UAV_DescriptorSize);
	return handle;
}

int DX::getDescHandleIndex()
{
	int index = g_CPU_CBV_SRV_UAV_DescriptorPool[g_CPU_CBV_SRV_UAV_DescriptorPool.size()-1];

	g_CPU_CBV_SRV_UAV_DescriptorUsing.push_back(index);
	g_CPU_CBV_SRV_UAV_DescriptorPool.pop_back();
	return index;
}

void DX::Update()
{
	sample->Update();
}

void DX::Render()
{
	//UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
	// warning if present is called on the render target when it's not in the present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_rtv_res[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	commandList->Close();

					  // create an array of command lists (only one command list here)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// present the current backbuffer
	swapChain->Present(0, 0);

	// this command goes in at the end of our command queue. we will know when our command queue 
	// has finished because the fence value will be set to "fenceValue" from the GPU since the command
	// queue is being executed on the GPU
	commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
}

void DX::UpdatePipeline()
{
	gpuHandleIndex = 0;

	// We have to wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	commandAllocator[frameIndex]->Reset();
	commandList->Reset(commandAllocator[frameIndex], NULL);

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_rtv_res[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->RSSetViewports(1, &viewport); // set the viewports
    commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &g_rtv_desc_handle[frameIndex], FALSE, &g_dsv_desc_handle);
	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(g_rtv_desc_handle[frameIndex], clearColor, 0, nullptr);
	// clear the depth/stencil buffer
    commandList->ClearDepthStencilView(g_dsv_desc_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	sample->UpdatePipeline();
}

void DX::resize(int w, int h)
{
	if(width == w && height == h) return;

	width = w;
	height = h;

    WaitForLastSubmittedFrame(0);
    WaitForLastSubmittedFrame(1);
    WaitForLastSubmittedFrame(2);

	for (UINT i = 0; i < frameBufferCount; i++)
    {
		if (g_rtv_res[i]) {
			g_rtv_res[i]->Release();
			g_rtv_res[i] = nullptr; 
		}
	}    
	
	if(g_dsv_res)
	{
		g_dsv_res->Release();
		g_dsv_res = nullptr; 
	}

	HRESULT result = swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	assert(SUCCEEDED(result) && "Failed to resize swapchain.");

	create_RTV_RES();
	create_DSV_RES();

    viewport.Width = float(w);
    viewport.Height = float(h);
    scissorRect.right = w;
    scissorRect.bottom = h;

	sample->resize();
}

void DX::WaitForLastSubmittedFrame(int index)
{
	if(fenceValue[index] == 0) return; // No fence was signaled

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[index]->GetCompletedValue() < fenceValue[index])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		fence[index]->SetEventOnCompletion(fenceValue[index], fenceEvent);

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void DX::WaitForPreviousFrame()
{
	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

void DX::initFence()
{
	for (int i = 0; i < frameBufferCount; i++)
	{
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		fenceValue[i] = 0; // set the initial fence value to 0
	}

	// create a handle to a fence event
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}
}

void DX::initCmdList()
{
	for (int i = 0; i < frameBufferCount; i++)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
	}

	// -- Create a Command List -- //

	// create the command list with the first allocator
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));

	// command lists are created in the recording state. our main loop will set it up for recording again so close it now
	//commandList->Close();
}

void DX::init_RTV_DESC_HEAP()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

	// This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
	// otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtv_desc_heap));
	g_rtv_desc_heap->SetName(L"g_rtv_desc_heap");
	// get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	int rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
	// but we cannot literally use it like a c++ pointer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < frameBufferCount; i++)
	{
		g_rtv_desc_handle[i] = rtvHandle;
		rtvHandle.ptr += rtvDescriptorSize;
	}

	create_RTV_RES();
}

void DX::create_RTV_RES()
{
	// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
	for (int i = 0; i < frameBufferCount; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		swapChain->GetBuffer(i, IID_PPV_ARGS(&g_rtv_res[i]));

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		device->CreateRenderTargetView(g_rtv_res[i], nullptr, g_rtv_desc_handle[i]);

		g_rtv_res[i]->SetName(L"g_rtv_res");
	}
}

void DX::init_DSV_DESC_HEAP()
{
	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_dsv_desc_heap));
	g_dsv_desc_heap->SetName(L"g_dsv_desc_heap");

	g_dsv_desc_handle = g_dsv_desc_heap->GetCPUDescriptorHandleForHeapStart();

	create_DSV_RES();
}

void DX::create_DSV_RES()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&g_dsv_res)
        );

	g_dsv_res->SetName(L"g_dsv_res");

    device->CreateDepthStencilView(g_dsv_res, &depthStencilDesc, g_dsv_desc_handle);
}

void DX::initSwapChain(HWND hwnd, bool fullScene)
{
	DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = frameBufferCount;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }
	
	IDXGIFactory4* dxgiFactory = nullptr;
	IDXGISwapChain1* swapChain1 = nullptr;
	if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
		return;
	if (dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
		return;
	if (swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain)) != S_OK)
		return;
	swapChain1->Release();
	dxgiFactory->Release();
	swapChain->SetMaximumFrameLatency(frameBufferCount);
	//g_hSwapChainWaitableObject = swapChain->GetFrameLatencyWaitableObject();
    

	// 假如窗口尺寸为800*600，那么实际的客户区域大概为784*561，要去掉标题栏和边框，swapChain和DSV的尺寸要等于客户区域，并非窗口size。
	// viewport 和 scissorRect 的尺寸貌似也要和客户区域相同哇。
	// 文档说，假如 swapchain的size和窗口size不同，默认会使用 DXGI_SCALING_STRETCH 拉伸模式，拉伸swapchian的size和窗口保持一致，拉伸会导致画面变形，imgui鼠标偏移哇。
	// 文档说，可以设置swapchain的Width和Height为0，然后如果采用 CreateSwapChainForHwnd 此方法创建交换链，会自动将Width和Height设置为实际的窗口客户size。
	// 实验表明如果设置为0，即使不使用CreateSwapChainForHwnd创建交换链，也会设置为正确的size。

	// 奇怪的地方？ 为什么没有全速率执行？ 即使非垂直同步，fps也受限了 ？

	// 解答： 这里有相关的描述  https://developer.nvidia.com/dx12-dos-and-donts
	// 只有全屏 并且 Present(0,0) 时 才允许无限帧速率和撕裂 。 任何其他模式都不允许无限帧速率和撕裂。
	// 如果不是全屏状态（真正的立即独立翻转模式），请仔细控制交换链中的延迟和缓冲区计数，以获得所需的 FPS 和延迟
	// 使用 IDXGISwapChain2::SetMaximumFrameLatency(MaxLatency) 设置所需的延迟，
	// 为此，您需要创建带有 DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT 标志集的交换链。
	// 在您呈现 MaxLatency-1 次后，DXGI 将开始在 Present() 中阻塞
	// 默认延迟为 3 时，这意味着 FPS 不能高于 2 * RefershRate。因此，对于 60Hz 显示器，FPS 不能超过 120 FPS。
	// 确实如此： UHD630中为60fps， RTX3070为120fps。

/*	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = 0; // buffer width
	backBufferDesc.Height = 0; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

	// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = !fullScene; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue, // the queue will be flushed once the swap chain is created
		&swapChainDesc, // give it the swap chain description we created above
		&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
*/
	frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void DX::initQueue()
{
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
}

void DX::initDevice()
{
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0
	bool adapterFound = false; // set this to true when a good one was found
	// find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		if(SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if(adapterFound)
	{
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	}
}

void DX::uploadRes()
{
	// Now we execute the command list to upload the initial assets (triangle data)
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
    fenceValue[frameIndex]++;
    commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
}

void DX::destory()
{
    SAFE_RELEASE(device);
    SAFE_RELEASE(swapChain);
    SAFE_RELEASE(commandQueue);
    SAFE_RELEASE(g_rtv_desc_heap);
    SAFE_RELEASE(commandList);

    for (int i = 0; i < frameBufferCount; ++i)
    {
        SAFE_RELEASE(g_rtv_res[i]);
        SAFE_RELEASE(commandAllocator[i]);
        SAFE_RELEASE(fence[i]);
    };
/*
	#ifdef DX12_ENABLE_DEBUG_LAYER
		IDXGIDebug1* pDebug = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
		{
			pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
			pDebug->Release();
		}
	#endif*/
}
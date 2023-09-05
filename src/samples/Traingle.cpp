#include "Traingle.h"
#include "Texture.h"

#include <filesystem>

void Traingle::init()
{
    dx = DX::dx;
    camera = new Camera(dx->width, dx->height);
    trans = new Transform();

    // 同一时刻， commandList->SetDescriptorHeaps 某种类型的描述符堆只能有一个被使用。
    // 所以，每种类型的描述符最好只创建一个。也可以创建多个，只要绘制时不同时使用即可。
    // 创建4个 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV 类型的描述符
    // 前面三个指向动态颜色值，每个后备缓冲对应一个
    // 第四个指向纹理
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 4;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    dx->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&CBV_SRV_UAV_DescriptorHeap));
    CBV_SRV_UAV_DescriptorSize = dx->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    initRootSignature();
    initPSO();
    initMesh(); // 顶点 索引缓冲，对于静态网格数据，最好从Upload上传至Default，性能最佳，相关上传指令需要记录到cmdList中
    initCBV(); // 对于每帧都需要更新的，仅使用Upload，不需要上传指令

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 3, CBV_SRV_UAV_DescriptorSize);
    tex = new Texture(L"D://cc/directx12-seed/assets/braynzar.jpg", cbvHandle);

    // 执行 buffer tex 初始化涉及的相关指令，从而保证相关资源在使用时处于可访问状态
    dx->uploadRes();
}

void Traingle::initRootSignature()
{
    // create root signature
    // create a root parameter and fill it out
    D3D12_ROOT_PARAMETER  rootParameters[3];

    // colorMultiplier   每帧更新，frameBufferCount个描述符堆和资源  【b0】
    {
        D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
        descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
        descriptorTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
        descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range   【b0】
        descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
        descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

        // create a descriptor table 
        D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
        descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
        descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
        rootParameters[0].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now
    }

    // mvp   每帧更新， frameBufferCount个资源，不使用描述符 【b1】
    {
        // create a root descriptor, which explains where to find the data for this root parameter
        D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
        rootCBVDescriptor.RegisterSpace = 0;
        rootCBVDescriptor.ShaderRegister = 1; // 【b1】

        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
        rootParameters[1].Descriptor = rootCBVDescriptor; // this is the root descriptor for this root parameter
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now
    }

    // 纹理  静态，就一个  【t0】 
    {
        D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1];
        descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
        descriptorTableRanges[0].NumDescriptors = 1; // we only have one texture right now, so the range is only 1
        descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range 【t0】
        descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
        descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

        // create a descriptor table 
        D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
        descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
        descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

        // fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
        // buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
        rootParameters[2].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // our pixel shader will be the only shader accessing this parameter for now
    }

    // 一个静态采样器  【s0】
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), // we have 1 root parameter
        rootParameters, // a pointer to the beginning of our root parameters array
        1,
        &sampler, //【s0】
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    ID3DBlob* errorBuff; // a buffer holding the error data if any
    ID3DBlob* signature;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
    dx->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
}

void Traingle::initPSO()
{
    // create vertex and pixel shaders
    D3D12_SHADER_BYTECODE vertexShaderBytecode = dx->createShader(L"D://cc/directx12-seed/assets/VertexShader.hlsl", "vs_5_0");
    D3D12_SHADER_BYTECODE pixelShaderBytecode = dx->createShader(L"D://cc/directx12-seed/assets/PixelShader.hlsl", "ps_5_0");

    // create input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // fill out an input layout description structure
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
    inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
    inputLayoutDesc.pInputElementDescs = inputLayout;

    // create a pipeline state object (PSO)

    // In a real application, you will have many pso's. for each different shader
    // or different combinations of shaders, different blend states or different rasterizer states,
    // different topology types (point, line, triangle, patch), or a different number
    // of render targets you will need a pso

    // VS is the only required shader for a pso. You might be wondering when a case would be where
    // you only set the VS. It's possible that you have a pso that only outputs data with the stream
    // output, and not on a render target, which means you would not need anything after the stream
    // output.

    DXGI_SAMPLE_DESC sampleDesc{};
    sampleDesc.Count = 1; // 需要和 swapChain的 SampleDesc.Count 保持一致。

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
    psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    // create the pso
    dx->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
}

void Traingle::initCBV()
{
    // create the constant buffer resource heap
    // We will update the constant buffer one or more times per frame, so we will use only an upload heap
    // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
    // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
    // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
    // will be modified and uploaded at least once per frame, so we only use an upload heap

    // create a resource heap, descriptor heap, and pointer to cbv for each frame
    
    for (int i = 0; i < dx->frameBufferCount; ++i)
    {
        dx->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&constantBufferUploadHeap[i]));
        constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

        // 描述符句柄
        CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, CBV_SRV_UAV_DescriptorSize);
        // 资源的类型描述
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        // 将资源关联到描述符句柄
        dx->device->CreateConstantBufferView(&cbvDesc, cbvHandle);
        //cbvHandle.Offset(1, CBV_SRV_UAV_DescriptorSize);

        ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
        constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultiplierGPUAddress[i]));
        memcpy(cbColorMultiplierGPUAddress[i], &cbColorMultiplierData, sizeof(cbColorMultiplierData));
    }


    for (int i = 0; i < dx->frameBufferCount; ++i)
    {
        // create resource for cube 1
        dx->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&constantBufferUploadHeaps[i]));
        constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

        ZeroMemory(&cbPerObject, sizeof(cbPerObject));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)
        
        // map the resource heap to get a gpu virtual address to the beginning of the heap
        constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));

        // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
        // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
        memcpy(cbvGPUAddress[i], &cbPerObject, sizeof(cbPerObject)); // cube1's constant buffer data
        memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube2's constant buffer data
    }
}

void Traingle::resize()
{
    camera->resize(dx->width, dx->height);
}

void Traingle::initMesh()
{
    std::vector<float> triVertexs = {
        0.0f, 0.5f, 0.0f,     0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f ,
        0.5f, -0.5f, 0.0f,    0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f ,
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f 
    };

    tri = new Mesh();
    tri->geom->setVertexs(triVertexs);
    tri->prepare();

    std::vector<float> quadVertexs = {
        -1.0f, 1.0f, 1.0f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f ,
        1.0f, 1.0f, 1.0f,     1.0f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f ,
        1.0f, -1.0f, 1.0f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f, 1.0f , 
        -1.0f, -1.0f, 1.0f,   0.0f, 1.0f,     1.0f, 0.0f, 0.0f, 1.0f  
    };
    std::vector<int> quadIndices = {
        0, 1, 2, // first triangle
        0, 2, 3 // second triangle
    };

    quad = new Mesh();
    quad->geom->setVertexs(quadVertexs);
    quad->geom->setIndices(quadIndices);
    quad->prepare();
}

void Traingle::Update()
{
    // update app logic, such as moving the camera or figuring out what objects are in view
    static float rIncrement = 0.002f;
    static float gIncrement = 0.006f;
    static float bIncrement = 0.009f;

    cbColorMultiplierData.colorMultiplier.x += rIncrement;
    cbColorMultiplierData.colorMultiplier.y += gIncrement;
    cbColorMultiplierData.colorMultiplier.z += bIncrement;

    if (cbColorMultiplierData.colorMultiplier.x >= 1.0f || cbColorMultiplierData.colorMultiplier.x <= 0.0f)
    {
        cbColorMultiplierData.colorMultiplier.x = cbColorMultiplierData.colorMultiplier.x >= 1.0f ? 1.0f : 0.0f;
        rIncrement = -rIncrement;
    }
    if (cbColorMultiplierData.colorMultiplier.y >= 1.0f || cbColorMultiplierData.colorMultiplier.y <= 0.0f)
    {
        cbColorMultiplierData.colorMultiplier.y = cbColorMultiplierData.colorMultiplier.y >= 1.0f ? 1.0f : 0.0f;
        gIncrement = -gIncrement;
    }
    if (cbColorMultiplierData.colorMultiplier.z >= 1.0f || cbColorMultiplierData.colorMultiplier.z <= 0.0f)
    {
        cbColorMultiplierData.colorMultiplier.z = cbColorMultiplierData.colorMultiplier.z >= 1.0f ? 1.0f : 0.0f;
        bIncrement = -bIncrement;
    }

    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cbColorMultiplierGPUAddress[dx->frameIndex], &cbColorMultiplierData, sizeof(cbColorMultiplierData));


    trans->rotateAxis(0.0f, 0.0f, 0.001f);
    trans->update();

    // update constant buffer for cube1
    // create the wvp matrix and store in constant buffer
    XMMATRIX viewMat = XMLoadFloat4x4(&camera->cameraViewMat); // load view matrix
    XMMATRIX projMat = XMLoadFloat4x4(&camera->cameraProjMat); // load projection matrix
    XMMATRIX wvpMat = trans->worldMat * viewMat * projMat; // create wvp matrix
    XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
    XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cbvGPUAddress[dx->frameIndex], &cbPerObject, sizeof(cbPerObject));


    wvpMat = trans->worldMat * viewMat * projMat; // create wvp matrix
    transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
    XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cbvGPUAddress[dx->frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
}

void Traingle::UpdatePipeline()
{
    auto commandList = dx->commandList;
    commandList->SetPipelineState(pipelineStateObject);

    // 设定相关的描述符堆
    ID3D12DescriptorHeap* descriptorHeaps[] = { CBV_SRV_UAV_DescriptorHeap };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // 设置根描述符
    commandList->SetGraphicsRootSignature(rootSignature); 
    // 根描述符参数0
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), dx->frameIndex, CBV_SRV_UAV_DescriptorSize);
    commandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
    // 根描述符参数2
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle2(CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 3, CBV_SRV_UAV_DescriptorSize);
    commandList->SetGraphicsRootDescriptorTable(2, cbvHandle2);
    // 根描述符参数1，没有创建描述符堆， mvp
    commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[dx->frameIndex]->GetGPUVirtualAddress());

    // draw
    tri->draw();

    // mvp
    commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[dx->frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

    // draw
    quad->draw();
}
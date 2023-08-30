#include "Traingle.h"

#include <filesystem>

Traingle::Traingle(DX &dx_ref)
{
    dx = &dx_ref;
}

void Traingle::init()
{
    initRootSignature();
    initPSO();
    initMesh();
    initCBV();
}

void Traingle::initRootSignature()
{
    // create root signature
    // create a descriptor range (descriptor table) and fill it out
    // this is a range of descriptors inside a descriptor heap
    D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
    descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
    descriptorTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
    descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range   [b0]
    descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
    descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables
    
    // create a descriptor table
    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
    descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

    // create a root descriptor, which explains where to find the data for this root parameter
    D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
    rootCBVDescriptor.RegisterSpace = 0;
    rootCBVDescriptor.ShaderRegister = 1; // [b1]

    // create a root parameter and fill it out
    D3D12_ROOT_PARAMETER  rootParameters[2]; // only one parameter right now
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
    rootParameters[0].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
    rootParameters[1].Descriptor = rootCBVDescriptor; // this is the root descriptor for this root parameter
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), // we have 1 root parameter
        rootParameters, // a pointer to the beginning of our root parameters array
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

    ID3DBlob* signature;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr));
    ThrowIfFailed(dx->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
    psoDesc.SampleDesc = dx->sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

    // create the pso
    ThrowIfFailed(dx->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));
}

void Traingle::initCBV()
{
    // Create a constant buffer descriptor heap for each frame
    // this is the descriptor heap that will store our constant buffer descriptor
    for (int i = 0; i < dx->frameBufferCount; ++i)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ThrowIfFailed(dx->device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap[i])));
    }

    // create the constant buffer resource heap
    // We will update the constant buffer one or more times per frame, so we will use only an upload heap
    // unlike previously we used an upload heap to upload the vertex and index data, and then copied over
    // to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
    // efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
    // will be modified and uploaded at least once per frame, so we only use an upload heap

    // create a resource heap, descriptor heap, and pointer to cbv for each frame
    for (int i = 0; i < dx->frameBufferCount; ++i)
    {
        ThrowIfFailed(dx->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&constantBufferUploadHeap[i])));
        constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        dx->device->CreateConstantBufferView(&cbvDesc, mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

        ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
        ThrowIfFailed(constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultiplierGPUAddress[i])));
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

    // build projection and view matrix
    XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f/180.0f), dx->width / dx->height, 0.1f, 1000.0f);
    XMStoreFloat4x4(&cameraProjMat, tmpMat);

    // set starting camera state
    cameraPosition = XMFLOAT4(0.0f, 0.0f, -4.0f, 0.0f);
    cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

    // build view matrix
    XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
    XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
    XMVECTOR cUp = XMLoadFloat4(&cameraUp);
    tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
    XMStoreFloat4x4(&cameraViewMat, tmpMat);

    // set starting cubes position
    // first cube
    cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // set cube 1's position
    XMVECTOR posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

    tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
    XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
    XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix

    // second cube
    cube2PositionOffset = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // create xmvector for cube2's position
                                                                                // we are rotating around cube1 here, so add cube2's position to cube1

    tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube2's position offset vector
    XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); // initialize cube2's rotation matrix to identity matrix
    XMStoreFloat4x4(&cube2WorldMat, tmpMat); // store cube2's world matrix
}

void Traingle::initMesh()
{
    // Create vertex buffer
    Vertex vList[] = {
        { 0.0f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f },
        { 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f }
    };
    Vertex vList2[] = {
        { -1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f,     1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, -1.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f }, 
        { -1.0f, -1.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f } 
    };
    DWORD iList2[] = {
        0, 1, 2, // first triangle
        0, 2, 3 // second triangle
    };
    // triangle
    vertexBufferView = dx->createVertexBuffer(sizeof(vList), sizeof(Vertex), reinterpret_cast<BYTE*>(vList));
    // quad
    vertexBufferView2 = dx->createVertexBuffer(sizeof(vList2), sizeof(Vertex), reinterpret_cast<BYTE*>(vList2));
    indexBufferView2 = dx->createIndexBuffer(sizeof(iList2), reinterpret_cast<BYTE*>(iList2));

    dx->uploadBuffer();
}

void Traingle::Update()
{
    // update app logic, such as moving the camera or figuring out what objects are in view
    static float rIncrement = 0.00002f;
    static float gIncrement = 0.00006f;
    static float bIncrement = 0.00009f;

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


    // create rotation matrices
    XMMATRIX rotXMat = XMMatrixRotationX(0.0000f);
    XMMATRIX rotYMat = XMMatrixRotationY(0.0000f);
    XMMATRIX rotZMat = XMMatrixRotationZ(0.0001f);

    // add rotation to cube1's rotation matrix and store it
    XMMATRIX rotMat = XMLoadFloat4x4(&cube1RotMat) * rotXMat * rotYMat * rotZMat;
    XMStoreFloat4x4(&cube1RotMat, rotMat);

    // create translation matrix for cube 1 from cube 1's position vector
    XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube1Position));

    // create cube1's world matrix by first rotating the cube, then positioning the rotated cube
    XMMATRIX worldMat = rotMat * translationMat;

    // store cube1's world matrix
    XMStoreFloat4x4(&cube1WorldMat, worldMat);

    // update constant buffer for cube1
    // create the wvp matrix and store in constant buffer
    XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat); // load view matrix
    XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat); // load projection matrix
    XMMATRIX wvpMat = XMLoadFloat4x4(&cube1WorldMat) * viewMat * projMat; // create wvp matrix
    XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
    XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cbvGPUAddress[dx->frameIndex], &cbPerObject, sizeof(cbPerObject));

    // now do cube2's world matrix
    // create rotation matrices for cube2
    rotXMat = XMMatrixRotationX(0.0000f);
    rotYMat = XMMatrixRotationY(0.0000f);
    rotZMat = XMMatrixRotationZ(0.0000f);

    // add rotation to cube2's rotation matrix and store it
    rotMat = rotZMat * (XMLoadFloat4x4(&cube2RotMat) * (rotXMat * rotYMat));
    XMStoreFloat4x4(&cube2RotMat, rotMat);

    // create translation matrix for cube 2 to offset it from cube 1 (its position relative to cube1
    XMMATRIX translationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));

    // we want cube 2 to be half the size of cube 1, so we scale it by .5 in all dimensions
    XMMATRIX scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);

    // reuse worldMat. 
    // first we scale cube2. scaling happens relative to point 0,0,0, so you will almost always want to scale first
    // then we translate it. 
    // then we rotate it. rotation always rotates around point 0,0,0
    // finally we move it to cube 1's position, which will cause it to rotate around cube 1
    worldMat = scaleMat * translationOffsetMat * rotMat * translationMat;

    wvpMat = XMLoadFloat4x4(&cube2WorldMat) * viewMat * projMat; // create wvp matrix
    transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
    XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cbvGPUAddress[dx->frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));

    // store cube2's world matrix
    XMStoreFloat4x4(&cube2WorldMat, worldMat);
}

void Traingle::UpdatePipeline()
{
    auto commandList = dx->commandList;
    commandList->SetPipelineState(pipelineStateObject);
    commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

    // set constant buffer descriptor heap
    ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap[dx->frameIndex] };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    // set the root descriptor table 0 to the constant buffer descriptor heap
    commandList->SetGraphicsRootDescriptorTable(0, mainDescriptorHeap[dx->frameIndex]->GetGPUDescriptorHandleForHeapStart());

    // mvp
    commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[dx->frameIndex]->GetGPUVirtualAddress());

    // draw
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    commandList->DrawInstanced(3, 1, 0, 0); // finally draw 3 vertices (draw the triangle)

    // mvp
    commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[dx->frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

    // draw
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView2); // set the vertex buffer (using the vertex buffer view)
    commandList->IASetIndexBuffer(&indexBufferView2);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0); // finally draw 3 vertices (draw the triangle)
}
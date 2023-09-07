#include "Traingle.h"
#include "Texture.h"
#include "mat/UnlitMat.h"

#include <filesystem>

void Traingle::init()
{
    dx = DX::dx;
    scene = new Scene();
    camera = new Camera(dx->width, dx->height);

    initMesh(); // 顶点 索引缓冲，对于静态网格数据，最好从Upload上传至Default，性能最佳，相关上传指令需要记录到cmdList中
    initCBV(); // 对于每帧都需要更新的，仅使用Upload，不需要上传指令

    // 执行 buffer tex 初始化涉及的相关指令，从而保证相关资源在使用时处于可访问状态
    dx->uploadRes();
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
            IID_PPV_ARGS(&cameraBufferRes[i]));
        cameraBufferRes[i]->SetName(L"Constant Buffer Upload Resource Heap");

        // 描述符句柄
        /*CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, CBV_SRV_UAV_DescriptorSize);
        // 资源的类型描述
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(CameraConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
        // 将资源关联到描述符句柄
        dx->device->CreateConstantBufferView(&cbvDesc, cbvHandle);
        //cbvHandle.Offset(1, CBV_SRV_UAV_DescriptorSize);

        ZeroMemory(&cameraBufferData, sizeof(cameraBufferData));*/

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
        cameraBufferRes[i]->Map(0, &readRange, reinterpret_cast<void**>(&cameraBufferResAddress[i]));
        memcpy(cameraBufferResAddress[i], &cameraBufferData, sizeof(cameraBufferData));
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
            IID_PPV_ARGS(&objBufferRes[i]));
        objBufferRes[i]->SetName(L"Constant Buffer Upload Resource Heap");

        ZeroMemory(&cbPerObject, sizeof(cbPerObject));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)
        
        // map the resource heap to get a gpu virtual address to the beginning of the heap
        objBufferRes[i]->Map(0, &readRange, reinterpret_cast<void**>(&objBufferResAddress[i]));

        // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
        // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
        memcpy(objBufferResAddress[i], &cbPerObject, sizeof(cbPerObject)); // cube1's constant buffer data
        memcpy(objBufferResAddress[i] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube2's constant buffer data
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
    tri->mat = new UnlitMat(L"D://cc/directx12-seed/assets/braynzar.jpg");

    scene->add(tri);

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
    quad->mat = new UnlitMat(L"D://cc/directx12-seed/assets/braynzar.jpg");

    scene->add(quad);
}

void Traingle::Update()
{
    cameraBufferData.projMat = camera->cameraProjMat;
    cameraBufferData.viewMat = camera->cameraViewMat;
    // copy our ConstantBuffer instance to the mapped constant buffer resource
    memcpy(cameraBufferResAddress[dx->frameIndex], &cameraBufferData, sizeof(cameraBufferData));

    tri->trans->rotateAxis(0.0f, 0.0f, 0.01f);
    tri->trans->update();

    // update constant buffer for cube1
    // create the wvp matrix and store in constant buffer
    //XMMATRIX viewMat = XMLoadFloat4x4(&camera->cameraViewMat); // load view matrix
    //XMMATRIX projMat = XMLoadFloat4x4(&camera->cameraProjMat); // load projection matrix
    //XMMATRIX wvpMat = tri->trans->worldMat * viewMat * projMat; // create wvp matrix
    //XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
    //XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
    // copy our ConstantBuffer instance to the mapped constant buffer resource
    XMStoreFloat4x4(&cbPerObject.modelMat, tri->trans->worldMat);
    memcpy(objBufferResAddress[dx->frameIndex], &cbPerObject, sizeof(cbPerObject));

    quad->trans->update();
    XMStoreFloat4x4(&cbPerObject.modelMat, quad->trans->worldMat);
    memcpy(objBufferResAddress[dx->frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
}

void Traingle::UpdatePipeline()
{
    scene->render(); 

    auto commandList = dx->commandList;
    // 设定相关的描述符堆  同一时刻，同种类型的描述符堆只能有一个被使用
    ID3D12DescriptorHeap* descriptorHeaps[] = { DX::dx->g_GPU_CBV_SRV_UAV_DescriptorHeap };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // 设置根描述符
    //commandList->SetPipelineState(tri->mat->pso->pipelineStateObject);
    //commandList->SetGraphicsRootSignature(tri->mat->pso->rootSignature); 
    // 根描述符 参数0，不使用句柄
    commandList->SetGraphicsRootConstantBufferView(0, cameraBufferRes[dx->frameIndex]->GetGPUVirtualAddress());
    // 根描述符 参数1，不使用句柄
    commandList->SetGraphicsRootConstantBufferView(1, objBufferRes[dx->frameIndex]->GetGPUVirtualAddress());

    // draw
    tri->draw();

    // mvp
    commandList->SetGraphicsRootConstantBufferView(1, objBufferRes[dx->frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

    // draw
    quad->draw();
}
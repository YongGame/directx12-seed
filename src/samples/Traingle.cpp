#include "Traingle.h"
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
    // 在确定 根签名的时候， 可以确定传入的数据类型
    cameraRes[0].CreateCommittedResource_UPLOAD(L"cameraCBV", 1024*64);
    cameraRes[1].CreateCommittedResource_UPLOAD(L"cameraCBV", 1024*64);
    cameraRes[2].CreateCommittedResource_UPLOAD(L"cameraCBV", 1024*64);

    objsRes.CreateCommittedResource_UPLOAD(L"objsCBV", 1024*64);
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
    memcpy(cameraRes[dx->frameIndex].resAddress, &cameraBufferData, sizeof(cameraBufferData));

    
    tri->trans->rotateAxis(0.0f, 0.0f, 0.01f);
    for(int i=0; i<scene->meshs.size(); i++)
    {
        scene->meshs[i]->trans->update();
        XMStoreFloat4x4(&cbPerObject.modelMat, scene->meshs[i]->trans->worldMat);
        memcpy(objsRes.resAddress + ConstantBufferPerObjectAlignedSize * i, &cbPerObject, sizeof(cbPerObject));
    }
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
    commandList->SetGraphicsRootConstantBufferView(0, cameraRes[dx->frameIndex].gpuAddress);
    // 根描述符 参数1，不使用句柄
    commandList->SetGraphicsRootConstantBufferView(1, objsRes.gpuAddress);

    // draw
    tri->draw();

    // mvp
    commandList->SetGraphicsRootConstantBufferView(1, objsRes.gpuAddress + ConstantBufferPerObjectAlignedSize);

    // draw
    quad->draw();
}
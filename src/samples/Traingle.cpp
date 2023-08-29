#include "Traingle.h"

#include <filesystem>

Traingle::Traingle(DX &dx_ref)
{
    dx = &dx_ref;
}

void Traingle::init()
{
    // create root signature
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ID3DBlob* signature;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr));
    ThrowIfFailed(dx->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

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
    // create the pso
    ThrowIfFailed(dx->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));

    // Create vertex buffer
    Vertex vList[] = {
        { 0.0f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f, 1.0f },
        { 0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f }
    };
    Vertex vList2[] = {
        { -0.7f, 0.2f, 0.6f,    1.0f, 0.0f, 0.0f, 1.0f },
        { 0.7f, 0.2f, 0.6f,     1.0f, 0.0f, 0.0f, 1.0f },
        { 0.7f, -0.2f, 0.6f,    1.0f, 0.0f, 0.0f, 1.0f }, 
        { -0.7f, -0.2f, 0.6f,   1.0f, 0.0f, 0.0f, 1.0f } 
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

}

void Traingle::UpdatePipeline()
{
    auto commandList = dx->commandList;
    commandList->SetPipelineState(pipelineStateObject);
    commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
    commandList->DrawInstanced(3, 1, 0, 0); // finally draw 3 vertices (draw the triangle)

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView2); // set the vertex buffer (using the vertex buffer view)
    commandList->IASetIndexBuffer(&indexBufferView2);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0); // finally draw 3 vertices (draw the triangle)
}
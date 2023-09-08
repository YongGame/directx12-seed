#include "UnlitMat.h"
#include "Shader.h"
#include "DX.h"

UnlitMat::UnlitMat(LPCWSTR filename)
{
    name = "UnlitMat";

    diffuse = new Texture(L"D://cc/directx12-seed/assets/braynzar.jpg");

    initPSO();
}

void UnlitMat::apply()
{
    //DX::dx->device->CopyDescriptorsSimple(1, DX::dx->getGpuDescHandle(), DX::dx->getDescHandle(diffuse->handleIndex), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //CD3DX12_GPU_DESCRIPTOR_HANDLE handle(CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 3, CBV_SRV_UAV_DescriptorSize);
    // getGpuHandle 这个方法会将cpu描述符堆的句柄 拷贝到 gpu描述符堆，顺便返回gpu描述符的GPU描述符句柄
    DX::dx->commandList->SetGraphicsRootDescriptorTable(2, DX::dx->getGpuHandle(diffuse->textureRes.handleIndex));
}

void UnlitMat::initPSO()
{
    pso = PSO::get(name);

    if(pso->pipelineStateObject) return;
    
    // create RootSignature
    // 根参数
    D3D12_ROOT_PARAMETER  rootParameters[3];

    // 根描述符，CBV，b0，camera数据
    D3D12_ROOT_DESCRIPTOR cameraDesc;
    cameraDesc.RegisterSpace = 0;
    cameraDesc.ShaderRegister = 0;
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; 
    rootParameters[0].Descriptor = cameraDesc; 
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // 根描述符，CBV，b1，每个模型的worldMat
    D3D12_ROOT_DESCRIPTOR modelDesc;
    modelDesc.RegisterSpace = 0;
    modelDesc.ShaderRegister = 1;
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; 
    rootParameters[1].Descriptor = modelDesc; 
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // 根描述符表，SRV，t0，每个模型的diffuseMap
    D3D12_DESCRIPTOR_RANGE  texRanges[1];
    texRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; 
    texRanges[0].NumDescriptors = 1; 
    texRanges[0].BaseShaderRegister = 0; 
    texRanges[0].RegisterSpace = 0; 
    texRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    D3D12_ROOT_DESCRIPTOR_TABLE texDescTable;
    texDescTable.NumDescriptorRanges = _countof(texRanges); 
    texDescTable.pDescriptorRanges = &texRanges[0]; 
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; 
    rootParameters[2].DescriptorTable = texDescTable; 
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; 

    // 一个静态采样器  s0
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
    rootSignatureDesc.Init(_countof(rootParameters), 
        rootParameters, 
        1,
        &sampler, 
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Shader* shader = new Shader();
    shader->setVertexShader(L"D://cc/directx12-seed/assets/VertexShader.hlsl");
    shader->setPixelShader(L"D://cc/directx12-seed/assets/PixelShader.hlsl");

    pso->build(shader, rootSignatureDesc);
}

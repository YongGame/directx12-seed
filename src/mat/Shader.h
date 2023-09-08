#pragma once
#include <d3dx12.h>

class Shader
{
public:
    // create vertex and pixel shaders
    D3D12_SHADER_BYTECODE vertexShaderBytecode;
    D3D12_SHADER_BYTECODE pixelShaderBytecode;

    void setVertexShader(LPCWSTR pFileName);
    void setPixelShader(LPCWSTR pFileName);
    D3D12_SHADER_BYTECODE createShader(LPCWSTR pFileName, LPCSTR pTarget);
};
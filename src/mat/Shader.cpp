#include "Shader.h"
#include <D3Dcompiler.h>

void Shader::setVertexShader(LPCWSTR pFileName)
{
    vertexShaderBytecode = createShader(pFileName, "vs_5_0");
}

void Shader::setPixelShader(LPCWSTR pFileName)
{
    pixelShaderBytecode = createShader(pFileName, "ps_5_0");
}

D3D12_SHADER_BYTECODE Shader::createShader(LPCWSTR pFileName, LPCSTR pTarget)
{
    // when debugging, we can compile the shader files at runtime.
    // but for release versions, we can compile the hlsl shaders
    // with fxc.exe to create .cso files, which contain the shader
    // bytecode. We can load the .cso files at runtime to get the
    // shader bytecode, which of course is faster than compiling
    // them at runtime
	
	// compile vertex shader
    ID3DBlob* shader; // d3d blob for holding vertex shader bytecode
    ID3DBlob* errorBuff; // a buffer holding the error data if any
    D3DCompileFromFile(pFileName,
        nullptr,
        nullptr,
        "main",
        pTarget,
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &shader,
        &errorBuff);

    // fill out a shader bytecode structure, which is basically just a pointer
    // to the shader bytecode and the size of the shader bytecode
    D3D12_SHADER_BYTECODE shaderBytecode = {};
    shaderBytecode.BytecodeLength = shader->GetBufferSize();
    shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

    return shaderBytecode;
}


#include "VertexBufferRes.h"
#include "../DX.h"

void VertexBufferRes::create(LPTSTR name, int size, int strideInBytes, const void *data)
{
    CreateCommittedResource_DEFAULT(name, size, data);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    DX::dx->commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            res, 
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        )
    );

    view.BufferLocation = gpuAddress;
    view.StrideInBytes = strideInBytes;
    view.SizeInBytes = size;
}
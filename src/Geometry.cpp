#include "Geometry.h"
#include "DX.h"

void Geometry::setVertexs(std::vector<float> datas)
{
    vertexs = datas;
}

void Geometry::setIndices(std::vector<int> datas)
{
    indices = datas;
}

void Geometry::prepare()
{
    createVertexBuffer();

    if(indices.size()>0) 
    {
        createIndexBuffer();
    }
}

void Geometry::draw()
{
    auto commandList = DX::dx->commandList;

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)

    if(indices.size() >0)
    {
        commandList->IASetIndexBuffer(&indexBufferView);
        commandList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0); // finally draw 3 vertices (draw the triangle)
    }
    else
    {
        commandList->DrawInstanced(vertexs.size()/9, 1, 0, 0); // finally draw 3 vertices (draw the triangle)
    }
    
}

void Geometry::createIndexBuffer()
{
    UINT iBufferSize = indices.size() * 4;

    indexBufferRes.CreateCommittedResource_DEFAULT(L"IndexBufferRes", iBufferSize, static_cast<void*>(indices.data()));

    DX::dx->commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(indexBufferRes.res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
    );

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	indexBufferView.BufferLocation = indexBufferRes.gpuAddress;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	indexBufferView.SizeInBytes = iBufferSize;
}

void Geometry::createVertexBuffer()
{
    UINT vBufferSize = vertexs.size() * 4;
    auto strideInBytes = stride * 4;

    vertexBufferRes.CreateCommittedResource_DEFAULT(L"VertexBufferRes", vBufferSize, static_cast<void*>(vertexs.data()));

    // transition the vertex buffer data from copy destination state to vertex buffer state
    DX::dx->commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(vertexBufferRes.res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
    );

    vertexBufferView.BufferLocation = vertexBufferRes.gpuAddress;
    vertexBufferView.StrideInBytes = strideInBytes;
    vertexBufferView.SizeInBytes = vBufferSize;
}
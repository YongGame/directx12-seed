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

    commandList->IASetVertexBuffers(0, 1, &vertexBufferRes.view); // set the vertex buffer (using the vertex buffer view)

    if(indices.size() >0)
    {
        commandList->IASetIndexBuffer(&indexBufferRes.view);
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

    indexBufferRes.create(L"IndexBufferRes", iBufferSize, static_cast<void*>(indices.data()));
}

void Geometry::createVertexBuffer()
{
    UINT vBufferSize = vertexs.size() * 4;
    auto strideInBytes = stride * 4;

    vertexBufferRes.create(L"VertexBufferRes", vBufferSize, strideInBytes, static_cast<void*>(vertexs.data()));
}
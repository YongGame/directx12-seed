#pragma once
#include <vector>
#include <d3d12.h>

class Geometry
{
public:
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    int stride = 9; // pos+uv+color
    std::vector<float> vertexs;
    std::vector<int> indices;

    void setVertexs(std::vector<float> datas);
    void setIndices(std::vector<int> datas);

    void prepare();
    void createVertexBuffer();
    void createIndexBuffer();
    void draw();
};
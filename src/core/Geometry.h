#pragma once
#include <vector>
#include <d3d12.h>
#include "res/VertexBufferRes.h"
#include "res/IndexBufferRes.h"

class Geometry
{
public:
    VertexBufferRes vertexBufferRes;

    IndexBufferRes indexBufferRes;
    

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
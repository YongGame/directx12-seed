#pragma once
#include "Res.h"

class VertexBufferRes : public Res
{
public:
    D3D12_VERTEX_BUFFER_VIEW view;
    void create(LPTSTR name, int size, int strideInBytes, const void *data);
};
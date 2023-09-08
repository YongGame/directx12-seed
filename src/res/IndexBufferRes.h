#pragma once
#include "Res.h"

class IndexBufferRes : public Res
{
public:
    D3D12_INDEX_BUFFER_VIEW view;
    void create(LPTSTR name, int size, const void *data);
};
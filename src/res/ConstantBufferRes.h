#pragma once
#include "Res.h"

class ConstantBufferRes : public Res
{
public:
    void create(LPTSTR name, int size);
};
#include "ConstantBufferRes.h"

void ConstantBufferRes::create(LPTSTR name, int size)
{
    CreateCommittedResource_UPLOAD(name, size);
}
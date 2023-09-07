#include "UnlitMat.h"

UnlitPso* UnlitMat::pso = nullptr;

UnlitMat::UnlitMat(LPCWSTR filename)
{
    if(!pso) pso = new UnlitPso();
    //diffuse = new Texture(L"D://cc/directx12-seed/assets/braynzar.jpg", cbvHandle);
}
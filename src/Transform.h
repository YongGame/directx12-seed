#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Transform
{
public:
    XMFLOAT4 translate{0.0f, 0.0f, 0.0f, 0.0f}; 
    XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
    XMFLOAT3 rotate{0.0f, 0.0f, 0.0f};
    XMMATRIX worldMat;

    Transform();
    void rotateAxis(float x, float y, float z);
    void update();
};
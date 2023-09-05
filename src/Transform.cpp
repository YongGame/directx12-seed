#include "Transform.h"

Transform::Transform()
{
    
}

void Transform::rotateAxis(float x, float y, float z)
{
    rotate.x += x;
    rotate.y += y;
    rotate.z += z;
}

void Transform::update()
{
    // 平移矩阵
    XMVECTOR translateVec = XMLoadFloat4(&translate);
    XMMATRIX translateMat = XMMatrixTranslationFromVector(translateVec);
    // 旋转矩阵
    XMMATRIX rotXMat = XMMatrixRotationX(rotate.x);
    XMMATRIX rotYMat = XMMatrixRotationY(rotate.y);
    XMMATRIX rotZMat = XMMatrixRotationZ(rotate.z);
    XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
    // 缩放矩阵
    XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);

    worldMat = scaleMat * rotMat * translateMat;
    //XMStoreFloat4x4(&worldMat, tmpMat);
}
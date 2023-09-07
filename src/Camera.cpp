#include "Camera.h"

Camera::Camera(int w, int h)
{
    resize(w, h);

    // set starting camera state
    cameraPosition = XMFLOAT4(0.0f, 0.0f, -4.0f, 0.0f);
    cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

    // build view matrix
    XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
    XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
    XMVECTOR cUp = XMLoadFloat4(&cameraUp);
    XMMATRIX tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp); // 直接获取视图矩阵
    XMStoreFloat4x4(&cameraViewMat, tmpMat);
}

void Camera::resize(int w, int h)
{
    // build projection and view matrix
    XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f/180.0f), float(w) / float(h), 0.1f, 1000.0f);
    XMStoreFloat4x4(&cameraProjMat, tmpMat);
}
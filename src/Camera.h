#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
    XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
    XMFLOAT4X4 cameraViewMat; // this will store our view matrix

    XMFLOAT4 cameraPosition; // this is our cameras position vector
    XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
    XMFLOAT4 cameraUp; // the worlds up vector

    Camera(int w, int h);
    void resize(int w, int h);
};
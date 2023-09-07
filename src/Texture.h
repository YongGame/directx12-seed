#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <wincodec.h>

class Texture
{
public:
    // 描述符句柄
    int handleIndex;


    BYTE* imageData;
    ID3D12Resource* textureBuffer; // the resource heap containing our texture
    ID3D12Resource* textureBufferUploadHeap;

    Texture(LPCWSTR filename);
    int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);
    DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
    WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
    int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
};
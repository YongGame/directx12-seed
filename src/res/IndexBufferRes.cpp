#include "IndexBufferRes.h"
#include "core/DX.h"

void IndexBufferRes::create(LPTSTR name, int size, const void *data)
{
    CreateCommittedResource_DEFAULT(name, size, data);

    DX::dx->commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            res, 
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_INDEX_BUFFER
        )
    );

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	view.BufferLocation = gpuAddress;
	view.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	view.SizeInBytes = size;
}

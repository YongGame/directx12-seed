struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv: TEXCOORD;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float2 uv: TEXCOORD;
    float4 color: COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 projMat;
    float4x4 viewMat;
};

cbuffer ConstantBuffer : register(b1)
{
    float4x4 modelMat;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // DirectXMath 是行矩阵格式。  HLSL默认使用列矩阵。 所以在传递 XMFLOAT4X4 格式的矩阵给GPU时，需要进行转置。
    float4x4 mvp = mul(transpose(viewMat),transpose(projMat));
    mvp = mul(transpose(modelMat), mvp);

    output.pos = mul(float4(input.pos, 1.0f), mvp);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}
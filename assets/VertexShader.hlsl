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
    float4 colorMultiplier;
};

cbuffer ConstantBuffer : register(b1)
{
    float4x4 wvpMat;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), wvpMat);
    output.color = input.color * colorMultiplier;
    output.uv = input.uv;
    return output;
}
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 적절한 행렬 계산을 위해 벡터를 4 단위로 변경합니다
    input.position.w = 1.0f;

    // 월드(world), 뷰(view) 및 투영(projection) 행렬에 대한 정점의 위치를 계산합니다
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // 픽셀 쉐이더가 사용할 입력 색상을 저장합니다
    output.color = input.color;

    return output;
}

cbuffer PerFrame
{
	float4x4 world_matrix;      //����任
	float4x4 normal_matrix;     //���߱任
	float4x4 final_matrix;      //�ܱ任
};
struct VertexIn//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosV       : POSITION;
	float3 NormalV    : NORMAL;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.NormalV = mul(float4(vin.normal,0.0f), normal_matrix).xyz;
	vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV, pin.PosV.z);
}
technique11 NormalDepth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
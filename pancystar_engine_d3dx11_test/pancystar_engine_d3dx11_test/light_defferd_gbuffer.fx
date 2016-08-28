#include"light_define.fx"
cbuffer perobject
{
	pancy_material   material_need;    //����
	float4x4         world_matrix;     //����任
	float4x4         normal_matrix;    //���߱任
	float4x4         final_matrix;     //�ܱ任
};
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
Texture2D        texture_diffuse;  //��������ͼ
Texture2D        texture_normal;   //������ͼ
struct Vertex_IN//��������ͼ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //�任��Ķ�������
	float3 normal        : NORMAL;         //�任��ķ�����
	float3 tangent       : TANGENT;        //����������
	float2 tex           : TEXCOORD;       //��������
	float3 position_bef  : POSITION;       //�任ǰ�Ķ�������
};
struct PixelOut_high
{
	float4 normal        : SV_TARGET0;
	float4 diffuse       : SV_TARGET1;
	float4 specular      : SV_TARGET2;
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal   = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent  = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.tex      = vin.tex1;
	vout.position_bef = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	return vout;
}
PixelOut_high PS(VertexOut pin)
{
	PixelOut_high pout;
	pin.normal = normalize(pin.normal);
	//���Gbuffer
	pout.normal = float4(pin.normal, pin.position_bef.z);
	pout.diffuse = texture_diffuse.Sample(samTex_liner, pin.tex) * material_need.diffuse;
	pout.specular = material_need.specular;
	return pout;
}
PixelOut_high PS_wthnormal(VertexOut pin)
{
	PixelOut_high pout;
	pin.normal = normalize(pin.normal);
	//���ͼƬ�����Կռ�->ģ������ͳһ����ռ�ı任����
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//�ӷ�����ͼ�л�÷��߲���
	normal_map = 2 * normal_map - 1;                               //��������ͼƬ����[0,1]ת������ʵ����[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //���߿ռ�������ռ�
	pin.normal = normal_map;
	//���Gbuffer
	pout.normal = float4(pin.normal, pin.position_bef.z);
	pout.diffuse = texture_diffuse.Sample(samTex_liner, pin.tex) * material_need.diffuse;
	pout.specular = material_need.specular;
	return pout;
}
technique11 draw_common
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 draw_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_wthnormal()));
	}
}
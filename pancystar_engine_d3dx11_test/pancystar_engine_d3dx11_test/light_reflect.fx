cbuffer PerFrame
{
	float4x4 world_matrix;      //����任
	float4x4 normal_matrix;     //���߱任
	float4x4 final_matrix;      //�ܱ任
	float3   position_view;     //�ӵ�λ��
};
TextureCube texture_cube;
SamplerState samTex
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
struct Vertex_IN//��������ͼ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //�任��Ķ�������
	float3 normal        : TEXCOORD0;      //�任��ķ�����
	float3 position_bef	 : TEXCOORD2;      //�任ǰ�Ķ�������
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.position_bef = normalize(mul(float4(vin.pos, 1.0f), world_matrix)).xyz;
	vout.normal = normalize(mul(float4(vin.normal, 0.0f), normal_matrix)).xyz;
	return vout;
}
float4 PS_reflect(VertexOut pin) :SV_TARGET
{
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	float4 color_fog = float4(0.75f, 0.75f, 0.75f, 1.0f);
	float3 view_direct = normalize(position_view - pin.position_bef);
	float3 map_direct = view_direct.xyz;//��������

	tex_color = texture_cube.Sample(samTex, map_direct);
	return tex_color;
}
float4 PS_inside(VertexOut pin) :SV_TARGET
{
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	float4 color_fog = float4(0.75f, 0.75f, 0.75f, 1.0f);

	float3 view_direct = normalize(pin.position_bef - position_view);

	float3 map_direct = -reflect(view_direct, pin.normal);//���߷�������
	tex_color = texture_cube.Sample(samTex, map_direct);
	return tex_color;
}
technique11 draw_reflect
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_reflect()));
	}
}
technique11 draw_inside
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_inside()));
	}
}

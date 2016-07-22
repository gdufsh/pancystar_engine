/*ǰ�����Ч��*/
#include"light_define.fx"

cbuffer perframe
{
	pancy_light_dir    dir_light_need[10];    //�����Դ
	pancy_light_point  point_light_need[5];   //���Դ
	pancy_light_spot   spot_light_need[15];   //�۹�ƹ�Դ
	float3             position_view;         //�ӵ�λ��
	
	float4x4           ssao_matrix;           //ssao�任
	int num_dir;
	int num_point;
	int num_spot;
};
cbuffer perobject 
{
	pancy_material   material_need;    //����
	float4x4         world_matrix;     //����任
	float4x4         normal_matrix;    //���߱任
	float4x4         final_matrix;     //�ܱ任
	float4x4         shadowmap_matrix; //��Ӱ��ͼ�任
};
Texture2D        texture_diffuse;  //��������ͼ
Texture2D        texture_normal;   //������ͼ
Texture2D        texture_shadow;   //��Ӱ��ͼ
Texture2D        texture_ssao;     //ssao��ͼ
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
SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparisonFunc = LESS_EQUAL;
};
float CalcShadowFactor(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	//��һ��
	shadowPosH.xyz /= shadowPosH.w;

	//�ɼ���ԴͶӰ������
	float depth = shadowPosH.z;

	//��Ӱ��ͼ�Ĳ���
	const float dx = 1.0f / 1024.0f;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		float2 rec_pos = shadowPosH.xy + offsets[i];
		if (rec_pos.x > 1.0f || rec_pos.x < 0.0f || rec_pos.y > 1.0f || rec_pos.y < 0.0f) 
		{
			percentLit += 1.0f;
		}
		else 
		{
			percentLit += shadowMap.SampleCmpLevelZero(samShadow, rec_pos, depth).r;
		}
	}
	return percentLit /= 9.0f;
}

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
	float4 pos_shadow    : POSITION1;       //��Ӱ��������
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal   = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent  = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.tex      = vin.tex1;
	vout.position_bef = mul(float4(vin.pos, 0.0f), world_matrix).xyz;
	vout.pos_shadow = mul(float4(vin.pos, 1.0f), shadowmap_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	pin.normal       = normalize(pin.normal);
	pin.tangent      = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[0], pin.normal, eye_direct, A1, D1, S1);

	compute_pointlight(material_need, point_light_need[0], pin.position_bef, pin.normal, position_view, A1, D1, S1);
	compute_spotlight(material_need, spot_light_need[0], pin.position_bef, pin.normal, eye_direct, A, D, S);
	ambient += A + A1;
	diffuse += D + D1;
	spec += S + S1;
	float4 final_color = ambient + diffuse + spec;
	return final_color;
}
float4 PS_withtex(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[0], pin.normal, eye_direct, A, D, S);

	compute_pointlight(material_need, point_light_need[0], pin.position_bef, pin.normal,position_view, A1, D1, S1);
	compute_spotlight(material_need, spot_light_need[0], pin.position_bef, pin.normal, eye_direct, A, D, S);
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	ambient += A + A1;
	diffuse += D + D1;
	spec += S+ S1;
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	return final_color;
}
float4 PS_withshadow(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[0], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	compute_pointlight(material_need, point_light_need[0], pin.position_bef, pin.normal,position_view, A1, D1, S1);
	compute_spotlight(material_need, spot_light_need[0], pin.position_bef, pin.normal, eye_direct, A, D, S);
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	ambient += A + A1;
	diffuse += (0.2 + 0.8*rec_shadow)*D + D1;
	spec += (0.2 + 0.8*rec_shadow)*S + S1;
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	return final_color;
}
technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 draw_withtexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withtex()));
	}
}
technique11 draw_withshadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadow()));
	}
}
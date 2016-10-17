/*�ӳٹ���Ч��*/
#include"light_define.fx"
cbuffer perobject
{
	pancy_material   material_need;    //����
	float4x4         final_matrix;     //�ܱ任
	float4x4         ssao_matrix;      //ssao�任
	float4x4         gBoneTransforms[100];//�����任����
};
Texture2D        texture_light_diffuse;      //�����������ͼ
Texture2D        texture_light_specular;     //���淴�������ͼ
Texture2D        texture_diffuse;            //��������ͼ
Texture2D        texture_ssao;               //ssao��ͼ
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
struct Vertex_IN//��������ͼ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //������������
};
struct Vertex_IN_bone//��������ͼ����
{
	float3	pos 	    : POSITION;     //����λ��
	float3	normal 	    : NORMAL;       //���㷨����
	float3	tangent     : TANGENT;      //����������
	uint4   bone_id     : BONEINDICES;  //����ID��
	float4  bone_weight : WEIGHTS;      //����Ȩ��
	uint4   texid       : TEXINDICES;   //��������
	float2  tex1        : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //�任��Ķ�������
	float2 tex           : TEXCOORD;       //��������
	float4 pos_ssao      : POSITION1;      //��Ӱ��������
};
VertexOut VS_bone(Vertex_IN_bone vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.bone_weight.x;
	weights[1] = vin.bone_weight.y;
	weights[2] = vin.bone_weight.z;
	weights[3] = vin.bone_weight.w;
	for (int i = 0; i < 4; ++i)
	{
		// �����任һ�㲻���ڲ������ŵ���������Կ��Բ������ߵ���ת�ò���
		posL += weights[i] * mul(float4(vin.pos, 1.0f), gBoneTransforms[vin.bone_id[i]]).xyz;
		normalL += weights[i] * mul(vin.normal, (float3x3)gBoneTransforms[vin.bone_id[i]]);
		tangentL += weights[i] * mul(vin.tangent.xyz, (float3x3)gBoneTransforms[vin.bone_id[i]]);
	}
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(posL, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(vin.pos, 1.0f), ssao_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.6f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	
	final_color.rgb *= final_color.a;
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
technique11 LightWithBone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
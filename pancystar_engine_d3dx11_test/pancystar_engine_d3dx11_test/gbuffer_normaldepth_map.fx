cbuffer PerFrame
{
	float4x4 world_matrix;      //����任
	float4x4 normal_matrix;     //���߱任
	float4x4 final_matrix;      //�ܱ任
	float4x4 gBoneTransforms[100];//�����任����
};
Texture2D        texture_diffuse;  //��������ͼ
Texture2D        texture_normal;   //������ͼ
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosV       : POSITION;
	float3 NormalV    : NORMAL;
	float3 tangent    : TANGENT;
	float2 tex        : TEXCOORD;       //��������
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
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.NormalV = mul(float4(vin.normal,0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	return vout;
}
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
	//posL = vin.pos;
	//normalL = vin.normal;
	//tangentL = vin.tangent;
	vout.PosV = mul(float4(posL, 1.0f), world_matrix).xyz;
	vout.NormalV = mul(float4(normalL, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(tangentL, 0.0f), normal_matrix).xyz;
	vout.PosH = mul(float4(posL, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV,10.0);
}
float4 PS_withalpha(VertexOut pin) : SV_Target
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.9f);
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV, 1);
}
float4 PS_withnormal(VertexOut pin) : SV_Target
{
	pin.NormalV = normalize(pin.NormalV);
	pin.tangent = normalize(pin.tangent);
	//���ͼƬ�����Կռ�->ģ������ͳһ����ռ�ı任����
	float3 N = pin.NormalV;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex_liner, pin.tex).rgb;//�ӷ�����ͼ�л�÷��߲���
	normal_map = 2 * normal_map - 1;                               //��������ͼƬ����[0,1]ת������ʵ����[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //���߿ռ�������ռ�
	pin.NormalV = normal_map;

	//pin.NormalV = normalize(pin.normal_map);
	//pin.NormalV = normalize(normal_map);
	//return float4(pin.NormalV,10.0);
	return float4(normal_map, 10.0);
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
technique11 NormalDepth_withalpha
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
	}
}
technique11 NormalDepth_skin
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 NormalDepth_skin_withalpha
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
	}
}
technique11 NormalDepth_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal()));
	}
}
technique11 NormalDepth_skin_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withnormal()));
	}
}
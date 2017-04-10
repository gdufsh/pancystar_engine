#include"terrain.hlsli"
cbuffer PerFrame
{
	float4x4 world_matrix;      //����任
	float4x4 normal_matrix;     //���߱任
	float4x4 final_matrix;      //�ܱ任
	float4x4 gBoneTransforms[100];//�����任����
	float4x4 world_matrix_array[300];
	float4x4 view_proj_matrix;
};
Texture2D        texture_diffuse;  //��������ͼ
Texture2D        texture_normal;   //������ͼ
Texture2DArray   texture_terrain_bump;       //���θ߶�ͼ
Texture2DArray   texture_terrain_diffuse;    //��������ͼ
struct patch_tess
{
	float edge_tess[4]: SV_TessFactor;
	float inside_tess[2] : SV_InsideTessFactor;
};
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
struct Vertex_IN_instance
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //������������
	uint InstanceId : SV_InstanceID;//instace������
};

struct Vertex_IN_terrain
{
	float3	pos 	: POSITION;     //����λ��
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //���ͼ��������
	float2  tex2    : TEXCOORD1;    //��������������
};
struct VertexOut_terrain
{
	float3 position        : POSITION;
	uint4  texid           : TEXINDICES;   //��������
	float2 tex1            : TEXCOORD;     //��������
	float2 tex2            : TEXCOORD1;    //��������
};
struct hull_out_terrain
{
	float3 position        : POSITION;
	uint4  texid           : TEXINDICES;     //��������
	float2 tex1            : TEXCOORD;       //�����������
	float2 tex2            : TEXCOORD1;      //��ͨ��������
};
struct domin_out_terrain
{
	float4 position        : SV_POSITION;    //�任��Ķ�������
	float3 NormalV         : NORMAL;
	uint4  texid           : TEXINDICES;     //��������
	float2 tex             : TEXCOORD;       //��������
};
VertexOut_terrain VS_terrain(Vertex_IN_terrain vin)
{
	VertexOut_terrain vout;
	vout.position = vin.pos;
	vout.texid = vin.texid;
	vout.tex1 = vin.tex1;
	vout.tex2 = vin.tex2;
	return vout;
}
patch_tess ConstantHS(InputPatch<VertexOut_terrain, 4> patch, uint PatchID:SV_PrimitiveID)
{
	patch_tess pt;
	pt.edge_tess[0] = 64;
	pt.edge_tess[1] = 64;
	pt.edge_tess[2] = 64;
	pt.edge_tess[3] = 64;

	pt.inside_tess[0] = 64;
	pt.inside_tess[1] = 64;

	return pt;
}
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
hull_out_terrain HS(
	InputPatch<VertexOut_terrain, 4> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	hull_out_terrain hout;
	hout.position = patch[i].position;
	hout.texid = patch[i].texid;
	hout.tex1 = patch[i].tex1;
	hout.tex2 = patch[i].tex2;
	return hout;
}
[domain("quad")]
domin_out_terrain DS(
	patch_tess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<hull_out_terrain, 4> quard
	)
{
	domin_out_terrain rec;
	float3 v1_pos = lerp(quard[0].position, quard[1].position, uv.x);
	float3 v2_pos = lerp(quard[2].position, quard[3].position, uv.x);
	float3 position_before = lerp(v1_pos, v2_pos, uv.y);
	float3 normal_before = float3(0.0f, 1.0f, 0.0f);
	float3 tangent_before = float3(1.0f, 0.0f, 0.0f);
	//������ͼ�����ֵ
	float2 v1_tex1 = lerp(quard[0].tex1, quard[1].tex1, uv.x);
	float2 v2_tex1 = lerp(quard[2].tex1, quard[3].tex1, uv.x);
	float2 tex1_need = lerp(v1_tex1, v2_tex1, uv.y);
	//��ȡ����ͼ
	float4 sample_normal = texture_terrain_bump.SampleLevel(samTex_liner, float3(tex1_need, quard[0].texid[0]), 0);
	float3 normal = sample_normal.xyz;

	//float height = (2.0f*sample_normal.w - 1.0f) * 30.0f;
	float height = count_terrain_height(sample_normal.w);
	//���ͼƬ�����Կռ�->ģ������ͳһ����ռ�ı任����
	float3 N = normal_before;
	float3 T = normalize(tangent_before - N * tangent_before * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	normal = 2 * normal - 1;               //��������ͼƬ����[0,1]ת������ʵ����[-1,1]
	normal = normalize(mul(normal, T2W));  //���߿ռ�������ռ�
	/*
	//todo����õķ�����ͼz�����Ƿ��ģ�������
	//solve���ӳ���Ⱦ��¼�ķ���������ռ�Ķ�����ȡ���ռ��
	normal.z = -normal.z;
	normal = normalize(mul(normal, normal_matrix));
*/
	position_before.y = height;
	//�������������ֵ
	float2 v1_tex2 = lerp(quard[0].tex2, quard[1].tex2, uv.x);
	float2 v2_tex2 = lerp(quard[2].tex2, quard[3].tex2, uv.x);
	float2 tex2_need = lerp(v1_tex2, v2_tex2, uv.y);
	//�����µĶ���
	rec.position = mul(float4(position_before, 1.0f), final_matrix);
	rec.texid = quard[0].texid;
	rec.tex = tex2_need;
	rec.NormalV = normal;
	return rec;
};
float4 PS_terrain(domin_out_terrain pin) :SV_TARGET
{
	pin.NormalV = normalize(pin.NormalV);
	return float4(pin.NormalV,10.0);
}


VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	//vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.PosV = mul(float4(vin.pos, 1.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.NormalV = mul(float4(vin.normal, 0.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), world_matrix_array[vin.InstanceId]).xyz;
	vout.PosH = mul(float4(vout.PosV, 1.0f), view_proj_matrix);
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
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	//clip(tex_color.a - 0.2f);
	pin.NormalV = normalize(pin.NormalV);
	float rec_need = pin.PosV.x + pin.PosV.y + pin.PosV.z;
	return float4(pin.NormalV, rec_need);
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
float4 PS_plant_normal(VertexOut pin) : SV_Target
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.2f);
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
technique11 NormalDepth_withinstance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
	}
}
technique11 NormalDepth_withinstance_normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_plant_normal()));
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
technique11 NormalDepth_terrain
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_terrain()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_terrain()));
	}
}
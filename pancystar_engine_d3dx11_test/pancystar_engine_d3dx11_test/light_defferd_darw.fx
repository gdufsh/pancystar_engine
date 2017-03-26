/*�ӳٹ���Ч��*/
#include"light_define.fx"
#include"terrain.hlsli"
cbuffer perobject
{
	pancy_material   material_need;    //����
	float4x4         world_matrix;     //����任
	float3           position_view;         //�ӵ�λ��
	float4x4         final_matrix;     //�ܱ任
	float4x4         ssao_matrix;      //ssao�任
	float4x4         gBoneTransforms[100];//�����任����
	float4x4         world_matrix_array[300];
	float4x4         view_proj_matrix;
};
Texture2D        texture_light_diffuse;      //�����������ͼ
Texture2D        texture_light_specular;     //���淴�������ͼ
Texture2D        texture_diffuse;            //��������ͼ
Texture2D        texture_ssao;               //ssao��ͼ
TextureCube      texture_cube;
Texture2DArray   texture_terrain_bump;       //���θ߶�ͼ
Texture2DArray   texture_terrain_diffuse;    //��������ͼ
StructuredBuffer<float4x4> mat_buffer;
struct patch_tess
{
	float edge_tess[4]: SV_TessFactor;
	float inside_tess[2] : SV_InsideTessFactor;
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
struct Vertex_IN//��������ͼ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex1    : TEXCOORD;     //������������
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
	float3 position_before : POSITION;
	float4 position        : SV_POSITION;    //�任��Ķ�������
	float2 tex             : TEXCOORD;       //��������
	float4 pos_ssao        : POSITION1;      //��Ӱ��������
};
struct PixelOut
{
	float4 final_color;
	float4 reflect_message;
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
	uint4  texid           : TEXINDICES;     //��������
	float2 tex             : TEXCOORD;       //��������
	float4 pos_ssao        : POSITION;      //��Ӱ��������
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
	//������ͼ�����ֵ
	float2 v1_tex1 = lerp(quard[0].tex1, quard[1].tex1, uv.x);
	float2 v2_tex1 = lerp(quard[2].tex1, quard[3].tex1, uv.x);
	float2 tex1_need = lerp(v1_tex1, v2_tex1, uv.y);
	//��ȡ����ͼ
	float4 sample_normal = texture_terrain_bump.SampleLevel(samTex_liner, float3(tex1_need, quard[0].texid[0]),0);
	float3 normal = sample_normal.xyz;
	float height = count_terrain_height(sample_normal.w);
	position_before.y = height;
	//�������������ֵ
	float2 v1_tex2 = lerp(quard[0].tex2, quard[1].tex2, uv.x);
	float2 v2_tex2 = lerp(quard[2].tex2, quard[3].tex2, uv.x);
	float2 tex2_need = lerp(v1_tex2, v2_tex2, uv.y);
	float4 aopos_before = mul(float4(position_before, 1.0f), ssao_matrix);
	//�����µĶ���
	rec.position = mul(float4(position_before, 1.0f), final_matrix);
	rec.texid = quard[0].texid;
	rec.tex = tex2_need;
	rec.pos_ssao = aopos_before;
	return rec;
};
PixelOut PS_terrain(domin_out_terrain pin) :SV_TARGET
{
	//pin.pos_ssao /= pin.pos_ssao.w + (pin.position / pin.position.w) *0.001f;
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 tex_color = texture_terrain_diffuse.Sample(samTex, float3(pin.tex, pin.texid[1]));
	//clip(tex_color.a - 0.6f);
	float4 ambient = 0.5f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;

	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}


VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position_before = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS_instance(Vertex_IN_instance vin)
{
	VertexOut vout;
	//vout.position_before = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.position_before = mul(float4(vin.pos, 1.0f), world_matrix_array[vin.InstanceId]).xyz;
	//float4x4 mat_check = mat_buffer[vin.InstanceId];
	//vout.position_before = mul(float4(vin.pos, 1.0f), mat_buffer[vin.InstanceId]).xyz;

	vout.position = mul(float4(vout.position_before,1.0f), view_proj_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(vout.position_before, 1.0f), ssao_matrix);
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
	vout.position_before = mul(float4(posL, 1.0f), world_matrix);
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(vout.position_before,1.0f), ssao_matrix);
	return vout;
}
PixelOut PS(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.5f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	
	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
	//final_color.rgb *= tex_color.a;
	//return final_color;
}
PixelOut PS_plant(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	float4 ambient = 0.5f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;

	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
}
PixelOut PS_without_ao(VertexOut pin) :SV_TARGET
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.6f*float4(1.0f, 1.0f, 1.0f, 0.0f);
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //�������
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //���淴���
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	//return final_color;
	PixelOut ans_pix;
	ans_pix.final_color = final_color;
	ans_pix.reflect_message = material_need.reflect;
	return ans_pix;
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
technique11 LightTech_instance
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_instance()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_plant()));
	}
}
technique11 LightTech_without_ao
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_without_ao()));
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
technique11 Lightterrain
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
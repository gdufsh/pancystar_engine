cbuffer PerFrame
{
	float4x4 gViewToTexSpace;      //����3D�ؽ��������任
	float4x4 invview_matrix;       //ȡ���任����
	float4x4 view_matrix;          //ȡ���任
	float3   view_position;        //�ӵ�λ��
	float4   gFrustumCorners[4];   //3D�ؽ����ĸ��ǣ����ڽ�����դ����ֵ
	float4x4 view_matrix_cube[6];
};
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
Texture2D gcolorMap;
TextureCube texture_cube;
TextureCube depth_cube;
SamplerState samp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};
struct VertexIn//��ͨ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //��Ⱦ���߱�Ҫ����
	float3 ToFarPlane : TEXCOORD0;   //����3D�ؽ�
	float2 Tex        : TEXCOORD1;   //��������
};
struct pixelOut
{
	float4 color_need;
	float4 mask_need;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//��Ϊ��ǰһ��shader��դ����ϵ����ص㣬����Ҫ���κα仯
	vout.PosH = float4(vin.pos, 1.0f);
	//���ĸ�����������ϣ��ĸ��ǵ�Ĵ���洢�ڷ��ߵ�x�������棩
	vout.ToFarPlane = gFrustumCorners[vin.normal.x].xyz;
	//��¼����������
	vout.Tex = vin.tex1;
	return vout;
}
//���������ľ�����ڵ��������Ȩֵ
[earlydepthstencil]
pixelOut PS(VertexOut pin) : SV_Target
{
	pixelOut out_color;
	out_color.color_need = float4(0.0f, 0.0f, 0.0f, 0.0f);
	out_color.mask_need = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//��ԭ�����������
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	//pz = 0.1f / (1.0f - pz);
	float3 position = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float step = 1.0f / 20;
	float st_find = 0.0f, end_find = 1000.0f;
	float2 answer_sampleloc;
	//���䷴�����
	//position = mul(float4(position.xyz,1.0f),invview_matrix);
	//float3 ray_dir = reflect(normalize(position), n);
	//float3 cube_ray = reflect(mul(float4(normalize(position), 0.0f), invview_matrix).xyz, mul(float4(n, 0.0f), invview_matrix).xyz);
	//float4 view_pos_need = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), invview_matrix);
	//float3 cube_ray = normalize(mul(float4(position, 1.0f), invview_matrix).xyz - float3(0.0f,5.0f,0.0f));
	float3 ray_dir = reflect(normalize(position), n);
	//float4 cube_color = texture_cube.SampleLevel(samTex_liner, ray_dir,0);
	//return cube_color;
	//return gcolorMap.Sample(samTex_liner, pin.Tex);
	/*
	float rec_step_alpha = dot(-normalize(position), n);
	if (rec_step_alpha > 0.9f)
	{
		return gcolorMap.Sample(samTex_liner, pin.Tex);
	}
	*/
	float ray_st = 0.0f,ray_end = 1000.0f;
	//����̽�����߶εľ�ȷ����
	[unroll]
	for (int i = 0; i < 15; ++i)
	{
		float length_final_rec = (ray_st + ray_end) / 2.0f;
		float3 now_3D_point = position + ray_dir * length_final_rec;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		if (rz > 99999.0f || rz < now_3D_point.z)
		{
			//�߶���
			ray_end = length_final_rec;
		}
		else
		{
			//�߶���
			ray_st = length_final_rec;
		}
	}
	// *(1 - rec_step_alpha);
	//������Ѱ�ҵ�һ���������ڵ�����
	[unroll]
	for (int i = 1; i <= 20; i++)
	{
		//�����ƽ�һλ
		float now_distance = ray_end * step * i;
		float3 now_3D_point = position + ray_dir * now_distance;
		//float3 now_3D_point = mul(float4(position + ray_dir * now_distance,1.0f), view_matrix).xyz;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		if (rz > 99999.0f)
		{
			step /= 2.0f;
		}
		if (rz < now_3D_point.z)
		{
			end_find = now_distance;
			break;
		}
	}
	//���־�ȷѰ�ҵ�һ���������ϸλ��
	float delta_save;
	float now_distance;
	float3 now_3D_point;
	[unroll]
	for (int i = 0; i < 25; ++i)
	{
		now_distance = (st_find + end_find) / 2.0f;
		//float3 now_3D_point = mul(float4(position + ray_dir * now_distance, 1.0f), view_matrix).xyz;
		now_3D_point = position + ray_dir * now_distance;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		answer_sampleloc = now_2D_position.xy;
		if (rz < now_3D_point.z)
		{
			end_find = now_distance;
		}
		else
		{
			st_find = now_distance;
		}
		delta_save = abs(rz - now_3D_point.z);
	}
	float alpha_fade = 0.7f * saturate(pow(((5.0f - now_distance) / 5.0f),3));
	float3 normal_test_sample = gNormalDepthMap.Sample(samNormalDepth, answer_sampleloc).xyz;
	float test_dot = dot(normalize(normal_test_sample), ray_dir);
	out_color.color_need = gcolorMap.Sample(samTex_liner, pin.Tex);
	if (delta_save < 0.00005f)
	{
		//float3 cube_ray = normalize(mul(float4(now_3D_point, 1.0f), invview_matrix).xyz - float3(0.0f, 5.0f, 0.0f));
		//float4 cube_color = depth_cube.SampleLevel(samTex_liner, cube_ray, 0) + texture_cube.SampleLevel(samTex_liner, cube_ray,0);
		//return cube_color;
		//float4 outputColor = (1.0f - alpha_fade) * gcolorMap.Sample(samTex_liner, pin.Tex) + alpha_fade * gcolorMap.Sample(samTex_liner, answer_sampleloc);
		float4 outputColor = gcolorMap.Sample(samTex_liner, answer_sampleloc);
		out_color.color_need = outputColor;
		out_color.mask_need = float4(1.0f, 0.0f, 0.0f, 0.0f);
		//return outputColor;
	}
	
	return out_color;
	//return gcolorMap.Sample(samTex_liner, pin.Tex);
}
float4 PS_cube(VertexOut pin) : SV_Target
{
	//��ԭ�����������
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	float3 position = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float step = 1.0f / 10;
	float st_find = 0.0f, end_find = 1000.0f;
	float2 answer_sampleloc;
	position = mul(float4(position, 1.0f), invview_matrix).xyz;
	n = mul(float4(n, 0.0f), invview_matrix).xyz;
	float3 ray_dir = reflect(normalize(position - view_position), n);
	float ray_st = 0.0f,ray_end = 1000.0f;
	//����̽�����߶εľ�ȷ����
	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		float length_final_rec = (ray_st + ray_end) / 2.0f;
		float3 now_3D_point = position + ray_dir * length_final_rec;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0).r;
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz > 99999.0f || rz < depth_3D_point)
		{
		//�߶���
			ray_end = length_final_rec;
		}
		else
		{
			//�߶���
			ray_st = length_final_rec;
		}
	}
	//������Ѱ�ҵ�һ���������ڵ�����
	[unroll]
	for (int i = 1; i <= 10; i++)
	{
		//�����ƽ�һλ
		float now_distance = ray_end * step * i;
		float3 now_3D_point = position + ray_dir * now_distance;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0);
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz > 99999.0f)
		{
			step /= 2.0f;
		}
		if (rz < depth_3D_point)
		{
			end_find = now_distance;
			break;
		}
	}
	//���־�ȷѰ�ҵ�һ���������ϸλ��
	float delta_save;
	float now_distance;
	float3 now_3D_point;
	[unroll]
	for (int i = 0; i < 15; ++i)
	{
		now_distance = (st_find + end_find) / 2.0f;
		now_3D_point = position + ray_dir * now_distance;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0);
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz < depth_3D_point)
		{
			end_find = now_distance;
		}
		else
		{
			st_find = now_distance;
		}
		delta_save = abs(rz - now_3D_point.z);
	}
	float3 cube_ray2 = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
	//float4 cube_color = texture_cube.SampleLevel(samTex_liner, cube_ray2,0);

	float alpha_fade = 0.7f * saturate(pow(((5.0f - now_distance) / 5.0f),3));
	return texture_cube.SampleLevel(samTex_liner, cube_ray2, 0);
}
technique11 draw_ssrmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_cube()));
	}
}

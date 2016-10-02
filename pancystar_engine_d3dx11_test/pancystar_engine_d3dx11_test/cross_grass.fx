cbuffer PerFrame
{
	float4x4 final_matrix;      //�ܱ任
	float4 position_view;          //���߷���
};
cbuffer cbFixed
{
	//������ĸ��������������
	float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};
};
Texture2D texture_first;
Texture2D texture_normal;
Texture2D texture_specular;
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn
{
	float3	position 	: POSITION;//�����λ��
	float2  size    : SIZE;       //������С
};
struct VertexOut
{
	float3	position 	: POSITION;     //�任��Ĺ����λ��
	float2  size    : SIZE;             //������С
};
struct GeoOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 normal  : NORMAL;
	float3 tangent : TANGENT;
	float2 Tex     : TEXCOORD;
	uint   PrimID  : SV_PrimitiveID;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.position = vin.position;
	vout.size = vin.size;
	return vout;
}
[maxvertexcount(12)]
void GS
(
	point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream
	)
{
	float3 up_camera = float3(0.0f, 1.0f, 0.0f);
	
	float theta = (gin[0].position.x + gin[0].position.z) * (3.1415926f / 5.0f);
	float3 right_camera1 = float3(cos(theta), 0.0f, sin(theta));
	//�����泯������Ϣ��ȡ�������ĸ�����
	float half_width = 0.5f * gin[0].size.x;
	float half_height = 0.5f* gin[0].size.y;
	float4 g_vout[4];
	g_vout[0] = float4(gin[0].position + half_width*right_camera1 - half_height*up_camera, 1.0f);
	g_vout[1] = float4(gin[0].position + half_width*right_camera1 + half_height*up_camera, 1.0f);
	g_vout[2] = float4(gin[0].position - half_width*right_camera1 - half_height*up_camera, 1.0f);
	g_vout[3] = float4(gin[0].position - half_width*right_camera1 + half_height*up_camera, 1.0f);
	GeoOut gout;
	
	[unroll]
	for (int i = 0; i < 4; ++i)//������������ɵ����Ǵ�
	{
		gout.PosH = mul(g_vout[i], final_matrix);
		gout.PosW = g_vout[i].xyz;
		//gout.PosW = gin[0].position;
		gout.Tex = gTexC[i];
		gout.PrimID = primID;
		gout.normal = cross(normalize(up_camera), normalize(right_camera1));
		gout.tangent = right_camera1;
		triStream.Append(gout);
	}
	triStream.RestartStrip();
	//�ڶ���billboard
	theta += 3.1415926 / 2.0f;
	float3 right_camera2 = float3(cos(theta), 0.0f, sin(theta));
	g_vout[0] = float4(gin[0].position + half_width*right_camera2 - half_height*up_camera, 1.0f);
	g_vout[1] = float4(gin[0].position + half_width*right_camera2 + half_height*up_camera, 1.0f);
	g_vout[2] = float4(gin[0].position - half_width*right_camera2 - half_height*up_camera, 1.0f);
	g_vout[3] = float4(gin[0].position - half_width*right_camera2 + half_height*up_camera, 1.0f);
	[unroll]
	for (int i = 0; i < 4; ++i)//������������ɵ����Ǵ�
	{
		g_vout[i].xyz -= 0.3f * right_camera1 * float3(gin[0].size.x, 0.0f, gin[0].size.x);
		g_vout[i].xyz += 0.3f * right_camera2 * float3(gin[0].size.x, 0.0f, gin[0].size.x);
		gout.PosH = mul(g_vout[i], final_matrix);
		gout.PosW = g_vout[i].xyz;
		//gout.PosW = gin[0].position;
		gout.Tex = gTexC[i];
		gout.PrimID = primID;
		gout.normal = cross(normalize(up_camera), normalize(right_camera2));
		gout.tangent = right_camera2;
		triStream.Append(gout);
	}
	triStream.RestartStrip();
	//������billboard
	theta += 3.1415926*3.0f / 4.0f;
	float3 right_camera3 = float3(cos(theta), 0.0f, sin(theta));
	g_vout[0] = float4(gin[0].position + half_width*right_camera3 - half_height*up_camera, 1.0f);
	g_vout[1] = float4(gin[0].position + half_width*right_camera3 + half_height*up_camera, 1.0f);
	g_vout[2] = float4(gin[0].position - half_width*right_camera3 - half_height*up_camera, 1.0f);
	g_vout[3] = float4(gin[0].position - half_width*right_camera3 + half_height*up_camera, 1.0f);
	[unroll]
	for (int i = 0; i < 4; ++i)//������������ɵ����Ǵ�
	{
		g_vout[i].xyz -= 0.3f * right_camera1 * float3(gin[0].size.x, 0.0f, gin[0].size.x);
		g_vout[i].xyz += 0.3f * right_camera3*float3(gin[0].size.x, 0.0f, gin[0].size.x);
		//g_vout[i].z -= 0.2f * gin[0].size.x;
		//g_vout[i].x -= 0.2f * gin[0].size.x;
		//g_vout[i].z += 0.3f;
		gout.PosH = mul(g_vout[i], final_matrix);
		gout.PosW = g_vout[i].xyz;
		//gout.PosW = gin[0].position;
		gout.Tex = gTexC[i];
		gout.PrimID = primID;
		gout.normal = cross(normalize(up_camera), normalize(right_camera3));
		gout.tangent = right_camera2;
		triStream.Append(gout);
	}
	triStream.RestartStrip();
}
float4 PS_withtex(GeoOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	//���ͼƬ�����Կռ�->ģ������ͳһ����ռ�ı任����

	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.Tex).rgb;//�ӷ�����ͼ�л�÷��߲���
	normal_map = 2 * normal_map - 1;                               //��������ͼƬ����[0,1]ת������ʵ����[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //���߿ռ�������ռ�

	float diffuse_angle = dot(pin.normal, normal_map);//�������

	float4 light = float4(0.5f,0.5f,0.5f,1.0f);
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	tex_color = texture_first.Sample(samTex,pin.Tex);
	float4 spec_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec_color = texture_specular.Sample(samTex, pin.Tex);
	
	float4 finalcolor = light * (tex_color*diffuse_angle + diffuse_angle*spec_color);
	//�ü�
	clip(tex_color.a - 0.05f);
	return finalcolor;
}
technique11 draw_with_tex
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(CompileShader(gs_5_0,GS()));
		SetPixelShader(CompileShader(ps_5_0,PS_withtex()));
	}
}
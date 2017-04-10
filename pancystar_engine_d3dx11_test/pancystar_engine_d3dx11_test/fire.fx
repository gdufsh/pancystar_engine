cbuffer perframe
{
	float3 position_view;    //�ӵ�λ��
	float3 position_start;   //���Ӳ���Դλ��
	float3 direction_start;  //���Ӳ�������
	float game_time;         //��Ϸʱ��(�������������)
	float delta_time;        //֡����ʱ��
	float4x4 final_matrix;   //�ܱ任����
};
struct Particle
{
	float3 position : POSITION;
	float3 speed    : VELOCITY;
	float2 SizeW    : SIZE;
	float Age : AGE;
	uint Type       : TYPE;
};
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
Texture2D texture_first; //������������
Texture1D texture_random;//�����������
cbuffer cbfixed
{
	float3 gAccelW = { 0.0f, 1.3f, 0.0f };//�������Ӽ��ٶ�
										  //�������������
	float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};
#define PT_EMITTER 0
#define PT_FLARE 1
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���Ӳ���shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};
DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};
BlendState AdditiveBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};
Particle StreamOutVS(Particle vin)
{
	return vin;
}
float3 RandUnitVec3(float offset)
{
	//����ʱ�������������
	float u = (game_time + offset);
	//�����������
	float3 v = 2.0f * texture_random.SampleLevel(samLinear, u, 0).xyz - 1.0f;
	//��һ��
	return normalize(v);
}
[maxvertexcount(25)]
void StreamOutGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	gin[0].Age += delta_time;
	//�Ƿ���Դ����
	if (gin[0].Type == PT_EMITTER)
	{
		// �Ƿ�Ӧ�������µ�����
		if (gin[0].Age > 0.005f)
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			if (vRandom.y < 0)
				vRandom.y = -vRandom.y;
			//vRandom.y *= 0.6f;
			Particle p;
			p.position = position_start.xyz;
			//p.position.z -= 0.73*j;
			//p.position.z -= 4.76*i;
			p.speed = 4.0f*vRandom;
			p.SizeW = float2(0.5f, 0.5f);
			p.Age = 0.0f;
			p.Type = PT_FLARE;
			ptStream.Append(p);
			//��ԭԴ���ӵ�ʱ��
			gin[0].Age = 0.0f;
			/*
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					float3 vRandom = RandUnitVec3(0.0f);
					vRandom.x *= 0.5f;
					vRandom.z *= 0.5f;
					if (vRandom.y < 0)
						vRandom.y = -vRandom.y;
					//vRandom.y *= 0.6f;
					Particle p;
					p.position = position_start.xyz;
					p.position.z -= 0.73*j;
					p.position.z -= 4.76*i;
					p.speed = 4.0f*vRandom;
					p.SizeW = float2(0.5f, 0.5f);
					p.Age = 0.0f;
					p.Type = PT_FLARE;

					ptStream.Append(p);

					//��ԭԴ���ӵ�ʱ��
					gin[0].Age = 0.0f;
				}
			}
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					float3 vRandom = RandUnitVec3(0.0f);
					vRandom.x *= 0.5f;
					vRandom.z *= 0.5f;
					if (vRandom.y < 0)
						vRandom.y = -vRandom.y;
					//vRandom.y *= 0.6f;
					Particle p;
					p.position = position_start.xyz;
					p.position.z -= 0.73*j;
					p.position.z -= 4.76*i;
					p.position.x -= 14.6;
					p.speed = 4.0f*vRandom;
					p.SizeW = float2(0.5f, 0.5f);
					p.Age = 0.0f;
					p.Type = PT_FLARE;

					ptStream.Append(p);

					//��ԭԴ���ӵ�ʱ��
					gin[0].Age = 0.0f;
				}
			}
			*/
		}

		// ��Դ�������붥�㻺����
		ptStream.Append(gin[0]);
	}
	else
	{
		//��Դ���ӣ��������ڻ�Ծʱ������붥�㻺��
		if (gin[0].Age <= 0.6f)
			ptStream.Append(gin[0]);
	}
}
GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x");

technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		SetGeometryShader(gsStreamOut);
		// ����������ɫ��
		SetPixelShader(NULL);
		// ������Ȼ���
		SetDepthStencilState(DisableDepth, 0);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������Ⱦshader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct Vout
{
	float3 position  : POSITION; //λ��
	float2 size      : SIZE;     //��С
	float4 alpha     : COLOR;    //��ɫ������
	uint   Type      : TYPE;     //����
};
struct GeoOut
{
	float4 PosH  : SV_Position;
	float3 PosW  : POSITION;
	float4 alpha : COLOR;
	float2 Tex   : TEXCOORD;
};
Vout DrawVS(Particle vin)
{
	Vout vout;
	float t = vin.Age;
	float3 rec_acce = -vin.speed * 0.8f + gAccelW;
	rec_acce.y = gAccelW.y;
	//�������ӵ�ǰ��λ��s[i+1] = a*t^2/2 + v*t + s[i];
	vout.position = 0.5f*t*t*rec_acce + 0.3*t*vin.speed + vin.position;
	//�������ӵ�����������ɫ˥��
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.alpha = float4(1.0f, 1.0f, 1.0f, opacity);
	vout.size = vin.SizeW;
	vout.Type = vin.Type;
	return vout;
}
[maxvertexcount(4)]
void DrawGS(point Vout gin[1],
	inout TriangleStream<GeoOut> triStream)
{
	//�ж������Ƿ���Դ����
	if (gin[0].Type != PT_EMITTER)
	{

		float3 up_camera = float3(0.0f, 1.0f, 0.0f);
		//���㹫�����泯����
		float3 look_camera = position_view - gin[0].position;
		look_camera.y = 0.0f;
		look_camera = normalize(look_camera);
		float3 right_camera = cross(up_camera, look_camera);
		//�����泯������Ϣ��ȡ�������ĸ�����
		float half_width = 0.5f * gin[0].size.x;
		float half_height = 0.5f* gin[0].size.y;
		float4 g_vout[4];
		g_vout[0] = float4(gin[0].position + half_width*right_camera - half_height*up_camera, 1.0f);
		g_vout[1] = float4(gin[0].position + half_width*right_camera + half_height*up_camera, 1.0f);
		g_vout[2] = float4(gin[0].position - half_width*right_camera - half_height*up_camera, 1.0f);
		g_vout[3] = float4(gin[0].position - half_width*right_camera + half_height*up_camera, 1.0f);
		GeoOut gout;
		[unroll]
		for (int i = 0; i < 4; ++i)//������������ɵ����Ǵ�
		{
			gout.PosH = mul(g_vout[i], final_matrix);
			gout.PosW = g_vout[i].xyz;
			gout.Tex = gTexC[i];
			gout.alpha = gin[0].alpha;
			triStream.Append(gout);
		}
	}
}
float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return texture_first.Sample(samLinear,pin.Tex)*(pin.alpha*0.8f);
}
technique11 DrawTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, DrawVS()));
		SetGeometryShader(CompileShader(gs_5_0, DrawGS()));
		SetPixelShader(CompileShader(ps_5_0, DrawPS()));
		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		SetDepthStencilState(NoDepthWrites, 0);
	}
}
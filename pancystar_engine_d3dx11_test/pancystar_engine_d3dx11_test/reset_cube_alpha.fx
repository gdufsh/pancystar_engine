Texture2D tex_input;
Texture2D depth_input;
float3 cube_count;
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
	float2  tex1    : TEXCOORD;     //������������
};
struct VertexOut
{
	float4 PosH     : SV_POSITION; //��Ⱦ���߱�Ҫ����
	float2 tex      : TEXCOORD;     //������������
};
struct pixelout 
{
	float4 color_RGB;
	float2 color_alpha;
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
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//�����ͶӰ����(���ڹ�դ��)
	vout.PosH = float4(vin.pos, 1.0f);
	//�������������(����msaa����)
	vout.tex = vin.tex1;
	return vout;
}
//----------------------------------------------------------------------------------
pixelout PS(VertexOut IN) : SV_TARGET
{
	pixelout final_pixel;
	float4 color_final = tex_input.SampleLevel(samTex_liner, IN.tex, 0);
	final_pixel.color_RGB = color_final;
	final_pixel.color_alpha.r = cube_count.r;
	final_pixel.color_alpha.g = depth_input.SampleLevel(samNormalDepth, IN.tex, 0);
	//color_final.a = cube_count.r;
    return final_pixel;
}
technique11 resolove_alpha
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

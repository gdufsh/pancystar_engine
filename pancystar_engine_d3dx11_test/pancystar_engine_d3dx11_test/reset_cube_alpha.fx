Texture2D tex_input;
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
float4 PS(VertexOut IN) : SV_TARGET
{
	float4 color_final = tex_input.SampleLevel(samTex_liner, IN.tex, 0);
	color_final.a = cube_count.r;
    return color_final;
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

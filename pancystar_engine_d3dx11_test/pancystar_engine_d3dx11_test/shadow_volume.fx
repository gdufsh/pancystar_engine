cbuffer perframe
{
	float3 position_light;   //��Դλ��
	float3 direction_light;  //��Դ����
	float4x4 world_matrix;   //����任����
	float4x4 normal_matrix;  //���߱任����
	float4x4 final_matrix;  //���߱任����
};
struct Vertex_IN//��������ͼ����
{
	float3	pos 	: POSITION;     //����λ��
	float3	normal 	: NORMAL;       //���㷨����
	float3	tangent : TANGENT;      //����������
	uint4   texid   : TEXINDICES;   //��������
	float2  tex     : TEXCOORD;     //������������
};
struct GSShadowIn
{
	float3 pos          : POSITION;
	float3 norm         : NORMAL;
};
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};
DepthStencilState VolumeComplexityStencil
{
	DepthEnable = true;
	DepthWriteMask = ZERO;
	DepthFunc = Less;

	// Setup stencil states
	StencilEnable = true;
	StencilReadMask = 0xFFFFFFFF;
	StencilWriteMask = 0xFFFFFFFF;

	BackFaceStencilFunc = Always;
	BackFaceStencilDepthFail = Decr;
	//BackFaceStencilDepthPass = Incr;
	BackFaceStencilPass = Keep;
	BackFaceStencilFail = Keep;

	FrontFaceStencilFunc = Always;
	FrontFaceStencilDepthFail = Incr;
	//FrontFaceStencilDepthPass = Decr;
	FrontFaceStencilPass = Keep;
	FrontFaceStencilFail = Keep;
};
RasterizerState DisableCulling
{
	CullMode = NONE;
};
struct PSShadowIn
{
	float4 pos			: SV_POSITION;
};
void DetectAndProcessSilhouette(float3 N,//���������η���
	GSShadowIn v1,    // �������������ڽ������εĹ����
	GSShadowIn v2,    // �������������ڽ������εĹ����
	GSShadowIn vAdj,  // �ڽ������εĹ�������
	inout TriangleStream<PSShadowIn> ShadowTriangleStream //�����
	)
{

	float3 NAdj = cross(v2.pos - vAdj.pos, v1.pos - vAdj.pos);
	N = position_light - v1.pos;
	//if (dot(N, NAdj) < 0)
	//{
		//	return;

		float3 outpos[4];
		float3 extrude1 = normalize(v1.pos - position_light);
		float3 extrude2 = normalize(v2.pos - position_light);

		float bias_offset = 0.1f;
		float shadowvolume_range = 20.0f;
		outpos[0] = v1.pos + bias_offset*extrude1;
		outpos[1] = v1.pos + (shadowvolume_range - bias_offset)*extrude1;
		outpos[2] = v2.pos + bias_offset*extrude2;
		outpos[3] = v2.pos + (shadowvolume_range - bias_offset)*extrude2;

		//����������Ӱ�����
		PSShadowIn Out;
		[unroll]
		for (int v = 0; v < 4; v++)
		{
			Out.pos = mul(float4(outpos[v], 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();
	//}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���Ӳ���shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GSShadowIn StreamOutVS(Vertex_IN vin)
{
	GSShadowIn vout;
	vout.pos = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.norm = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	return vout;
}

/*                       5
						/\
					   / d\
					 0/_ _ \4
					 /\ a  /\
					/ b\  / c\
				  1/_ _ \/_ _ \3
						 2
pancy:����������Ϊa�����ڽ�������Ϊb,c,d����gs����ṹ�ж��������� �μ�:https://msdn.microsoft.com/en-us/library/windows/desktop/bb509609(v=vs.85).aspx
*/

[maxvertexcount(18)]
void GSShadowmain(triangleadj GSShadowIn In[6], inout TriangleStream<PSShadowIn> ShadowTriangleStream)
{
	// �������������η��߷���
	float3 N = cross(In[2].pos - In[0].pos, In[4].pos - In[0].pos);
	//�����Դ����(�۹�ƹ�Դ)
	float3 lightDir = position_light - In[0].pos;
	// float3(0, -1, -1);
	//;
	//�����������泯���Դʱ�������Ƿ��б�Ե�ߣ���Ϊ��Ե�����ɼ�����
	if (dot(N, lightDir) > 0.0f)
	{
		float bias_offset = 0.1f;
		float shadowvolume_range = 20.0f;
		// ���������Ե��
		DetectAndProcessSilhouette(lightDir, In[0], In[2], In[1], ShadowTriangleStream);
		DetectAndProcessSilhouette(lightDir, In[2], In[4], In[3], ShadowTriangleStream);
		DetectAndProcessSilhouette(lightDir, In[4], In[0], In[5], ShadowTriangleStream);
		//��������
		PSShadowIn Out;
		for (int i = 0; i < 6; i += 2)
		{
			float3 extrude = normalize(In[i].pos - position_light);

			float3 pos = In[i].pos + bias_offset*extrude;
			Out.pos = mul(float4(pos, 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();

		//�����ײ�
		for (int k = 4; k >= 0; k -= 2)
		{
			float3 extrude = normalize(In[k].pos - position_light);

			float3 pos = In[k].pos + (shadowvolume_range - bias_offset)*extrude;
			Out.pos = mul(float4(pos, 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();
	}
}
//GeometryShader gsStreamOut = ConstructGSWithSO(CompileShader(gs_5_0, GSShadowmain()),"POSITION.xyz");

float4 PS(PSShadowIn pin) :SV_TARGET
{
	return float4(1.0f,1.0f,1.0f,1.0f);
}
technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		//SetGeometryShader(NULL);
		//SetGeometryShader(gsStreamOut);
		SetGeometryShader(CompileShader(gs_5_0, GSShadowmain()));
		// ����������ɫ��
		//SetPixelShader(NULL);
		//SetDepthStencilState(DisableDepth, 0);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
		SetDepthStencilState(VolumeComplexityStencil, 1);
	}
}
#include"shader_pancy.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��������ɫ�����벿��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_basic::shader_basic(LPCWSTR filename,ID3D11Device *device_need,ID3D11DeviceContext *contex_need)
{
	fx_need = NULL;
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_filename = filename;
}
HRESULT shader_basic::shder_create() 
{
	HRESULT hr = combile_shader(shader_filename);
	if (hr != S_OK) 
	{
		MessageBox(0,L"combile shader error",L"tip",MB_OK);
		return hr;
	}
	init_handle();
	return S_OK;
}
void shader_basic::release_basic()
{
	safe_release(fx_need);
}
HRESULT shader_basic::get_technique(ID3DX11EffectTechnique** tech_need,LPCSTR tech_name)
{
	D3D11_INPUT_ELEMENT_DESC member_point[30];
	UINT num_member;
	set_inputpoint_desc(member_point,&num_member);
	*tech_need = fx_need->GetTechniqueByName(tech_name);
	D3DX11_PASS_DESC pass_shade;
	HRESULT hr2;
	hr2 = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	HRESULT hr = device_pancy->CreateInputLayout(member_point,num_member,pass_shade.pIAInputSignature,pass_shade.IAInputSignatureSize,&input_need);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"CreateInputLayout����!", L"����", MB_OK);
		return hr;
	}
	contex_pancy->IASetInputLayout(input_need);
	input_need->Release();
	input_need = NULL;
	return S_OK;
}
HRESULT shader_basic::combile_shader(LPCWSTR filename)
{
	//����shader
	UINT flag_need(0);
	flag_need |= D3D10_SHADER_SKIP_OPTIMIZATION;
#if defined(DEBUG) || defined(_DEBUG)
	flag_need |= D3D10_SHADER_DEBUG;
#endif
	//����ID3D10Blob������ű���õ�shader��������Ϣ
	ID3D10Blob	*shader(NULL);
	ID3D10Blob	*errMsg(NULL);
	//����effect
	std::ifstream fin(filename, std::ios::binary);
	if (fin.fail()) 
	{
		MessageBox(0, L"open shader file error", L"tip", MB_OK);
		return E_FAIL;
	}
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();
	HRESULT hr = D3DX11CreateEffectFromMemory(&compiledShader[0], size,0,device_pancy,&fx_need);
	if(FAILED(hr))
	{
		MessageBox(NULL,L"CreateEffectFromMemory����!",L"����",MB_OK);
		return E_FAIL;
	}
	safe_release(shader);
	//�������붥���ʽ
	return S_OK;
}
HRESULT shader_basic::set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	HRESULT hr;
	hr = mat_handle->SetMatrix(reinterpret_cast<float*>(&rec_mat));
	return hr;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ǰ�����������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_pre::light_pre(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{

}
void light_pre::init_handle()
{
	//������
	texture_diffuse_handle = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();  //shader�е�������Դ���
	texture_normal_handle = fx_need->GetVariableByName("texture_normal")->AsShaderResource();    //������ͼ����
	texture_shadow_handle = fx_need->GetVariableByName("texture_shadow")->AsShaderResource();    //��Ӱ��ͼ���
	texture_ssao_handle = fx_need->GetVariableByName("texture_ssao")->AsShaderResource();        //��������ͼ���
																								 //���α任���
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //ȫ�׼��α任���
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();           //����任���
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();         //���߱任���
																							//texture_matrix_handle = fx_need->GetVariableByName("position_view")->AsMatrix();      //����任���
	shadowmap_matrix_handle = fx_need->GetVariableByName("shadowmap_matrix")->AsMatrix();   //shadowmap����任���
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();             //ssao����任���
																							//�ӵ㼰����
	view_pos_handle = fx_need->GetVariableByName("position_view");
	material_need = fx_need->GetVariableByName("material_need");
	//���վ��
	light_dir = fx_need->GetVariableByName("dir_light_need");                     //�����
	light_point = fx_need->GetVariableByName("point_light_need");                 //���Դ
	light_spot = fx_need->GetVariableByName("spot_light_need");                   //�۹��
}
void light_pre::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
HRESULT light_pre::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_world(XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	XMVECTOR x_delta;
	XMMATRIX check = rec_mat;
	//���߱任
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, check));
	normal_need.r[0].m128_f32[3] = 0.0f;
	normal_need.r[1].m128_f32[3] = 0.0f;
	normal_need.r[2].m128_f32[3] = 0.0f;
	normal_need.r[3].m128_f32[3] = 1.0f;
	HRESULT hr;
	hr = set_matrix(world_matrix_handle, mat_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting world matrix", L"tip", MB_OK);
		return hr;
	}
	hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&normal_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting normal matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_shadow(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(shadowmap_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting shadowmap matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_ssao(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(ssao_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_material(pancy_material material_in)
{
	HRESULT hr = material_need->SetRawValue(&material_in, 0, sizeof(material_in));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting material", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_ssaotex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ssao_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_shadowtex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_shadow_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting shadowmap texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_diffusetex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_normaltex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_normal_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting normal texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_dirlight(pancy_light_dir light_need, int light_num)
{
	HRESULT hr = light_dir->SetRawValue(&light_need, light_num * sizeof(pancy_light_dir), sizeof(pancy_light_dir));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting direct light", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_pointlight(pancy_light_point light_need, int light_num)
{
	HRESULT hr = light_point->SetRawValue(&light_need, light_num * sizeof(light_need), sizeof(light_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting point light", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_spotlight(pancy_light_spot light_need, int light_num)
{
	HRESULT hr = light_spot->SetRawValue(&light_need, light_num * sizeof(light_need), sizeof(light_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting spot light", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_pre::release() 
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow map����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_shadow::light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT light_shadow::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_shadow::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //ȫ�׼��α任���
}
void light_shadow::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void light_shadow::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao ��ȷ��߼�¼����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaodepthnormal_map::shader_ssaodepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaodepthnormal_map::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
}
HRESULT shader_ssaodepthnormal_map::set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view)
{
	XMVECTOR x_delta;
	XMMATRIX world_need = XMLoadFloat4x4(mat_world);
	XMMATRIX view_need = XMLoadFloat4x4(mat_view);
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
	normal_need.r[0].m128_f32[3] = 0;
	normal_need.r[1].m128_f32[3] = 0;
	normal_need.r[2].m128_f32[3] = 0;
	normal_need.r[3].m128_f32[3] = 1;
	HRESULT hr;
	hr = world_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(world_need*view_need)));
	if (FAILED(hr))
	{
		MessageBox(0, L"set world matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
	hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(normal_need*view_need)));
	if (FAILED(hr))
	{
		MessageBox(0, L"set view matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaodepthnormal_map::set_trans_all(XMFLOAT4X4 *mat_final) 
{
	HRESULT hr;
	hr = set_matrix(project_matrix_handle, mat_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"set final matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
}
void shader_ssaodepthnormal_map::release()
{
	release_basic();
}
void shader_ssaodepthnormal_map::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao �ڱ���Ⱦ����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaomap::shader_ssaomap(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaomap::init_handle()
{
	ViewToTexSpace = fx_need->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors = fx_need->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	RandomVecMap = fx_need->GetVariableByName("gRandomVecMap")->AsShaderResource();
}
void shader_ssaomap::release()
{
	release_basic();
}
HRESULT shader_ssaomap::set_ViewToTexSpace(XMFLOAT4X4 *mat)
{
	HRESULT hr = set_matrix(ViewToTexSpace, mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set viewtotex matrix error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		MessageBox(0, L"set FrustumCorners error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_OffsetVectors(const XMFLOAT4 v[14])
{
	HRESULT hr = OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14);
	if (FAILED(hr))
	{
		MessageBox(0, L"set OffsetVectors error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_NormalDepthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = NormalDepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_randomtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = RandomVecMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set randomtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_ssaomap::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao aoͼģ��������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaoblur::shader_ssaoblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaoblur::release()
{
	release_basic();
}
void shader_ssaoblur::init_handle()
{
	TexelWidth = fx_need->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = fx_need->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	InputImage = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
}
HRESULT shader_ssaoblur::set_image_size(float width, float height)
{
	HRESULT hr;
	hr = TexelWidth->SetFloat(width);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	hr = TexelHeight->SetFloat(height);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaoblur::set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap)
{
	HRESULT hr;
	hr = NormalDepthMap->SetResource(tex_normaldepth);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_normaldepth error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	hr = InputImage->SetResource(tex_aomap);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_aomap error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_ssaoblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//���ö�������
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//������    ��������      ���ݸ�ʽ          ����� ��ʼ��ַ     ����۵ĸ�ʽ 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ȫ��shader������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_control::shader_control()
{
	shader_light_pre = NULL;
	shader_light_deferred = NULL;
	shader_shadowmap = NULL;
	shader_cubemap = NULL;
	shader_ssao_depthnormal = NULL;
	shader_ssao_draw = NULL;
	shader_ssao_blur = NULL;
	particle_build = NULL;
	particle_show = NULL;
}
HRESULT shader_control::shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy)
{
	HRESULT hr;
	shader_light_pre = new light_pre(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\light_pre.cso", device_pancy, contex_pancy);
	hr = shader_light_pre->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}

	shader_shadowmap = new light_shadow(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\shadowmap.cso", device_pancy, contex_pancy);
	hr = shader_shadowmap->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_ssao_depthnormal = new shader_ssaodepthnormal_map(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_normaldepth_map.cso", device_pancy, contex_pancy);
	hr = shader_ssao_depthnormal->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_ssao_draw = new shader_ssaomap(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_draw_aomap.cso", device_pancy, contex_pancy);
	hr = shader_ssao_draw->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}

	shader_ssao_blur = new shader_ssaoblur(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_blur_map.cso", device_pancy, contex_pancy);
	hr = shader_ssao_blur->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}
	
	return S_OK;
}
void shader_control::release()
{
	shader_light_pre->release();
	shader_shadowmap->release();
	shader_ssao_depthnormal->release();
	shader_ssao_draw->release();
	shader_ssao_blur->release();
}
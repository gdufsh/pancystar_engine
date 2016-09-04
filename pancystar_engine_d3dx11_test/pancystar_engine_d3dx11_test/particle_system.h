#pragma once
#include"shader_pancy.h"
#include"geometry.h"
using namespace DirectX;
enum particle_use_type 
{
	PARTICLE_TYPE_FIRE = 0,
	PARTICLE_TYPE_SAKURA = 1
};
template<typename T>
class particle_system
{
	ID3D11Device          *device_pancy;  //d3d�豸
	ID3D11DeviceContext   *contex_pancy;  //d3d������
	shader_control        *shader_list;   //shader������
	bool if_firstrun;                   //�ж�����ϵͳ�Ƿ��Ѿ���ʼ����
	float age_need;                     //��������
	float game_time;                    //ȫ����Ϸʱ��
	float time_delta;                   //��֮֡���ʱ����
	XMFLOAT3 start_pos;                 //���Ӳ���λ��
	XMFLOAT3 start_dir;                 //���Ӳ�������
	UINT particle_num;                  //��������
	particle_use_type     particle_type;//��������
	ID3D11Buffer* auto_Vinput_need;     //���Ӷ������뻺����
	ID3D11Buffer* first_Vdraw_need;     //���Ӷ����ʼ������
	ID3D11Buffer* auto_Vstream0ut_need; //���Ӷ������������

	ID3D11ShaderResourceView* particle_tex;//��������
	ID3D11ShaderResourceView* random_tex;  //���������
public:
	particle_system(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, UINT max_particle_num, shader_control *shader_need, particle_use_type type_need);
	void draw_particle();
	void update(float delta_time, float time_world, XMFLOAT4X4 *mat_proj, XMFLOAT3 *view_direct);//�������Ӷ���ʱ��
	void set_particle_direct(XMFLOAT3 *position, XMFLOAT3 *direct);
	HRESULT create(wchar_t *texture_path);
	void reset();
	void release();
private:
	HRESULT init_vertex_buff();
	HRESULT init_texture(wchar_t *texture_path);
	ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
	template<class K>
	void safe_release(K t)
	{
		if (t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
template<typename T>
ID3D11ShaderResourceView* particle_system<T>::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	// �����������.
	XMFLOAT4 randomValues[1024];
	for (int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		randomValues[i].w = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(XMFLOAT4);
	initData.SysMemSlicePitch = 0;
	// ��������.
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;
	ID3D11Texture1D* randomTex = 0;
	HRESULT hr = device->CreateTexture1D(&texDesc, &initData, &randomTex);
	if (FAILED(hr)) 
	{
		MessageBox(0, L"create random tex error in particle system", L"tip", MB_OK);
		return NULL;
	}
	// ������Դ��ͼ.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	ID3D11ShaderResourceView* randomTexSRV = 0;
	hr = device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create random tex error in particle system", L"tip", MB_OK);
		return NULL;
	}
	if (randomTex != NULL)
	{
		randomTex->Release();
	}
	return randomTexSRV;
}
template<typename T>
particle_system<T>::particle_system(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, UINT max_particle_num, shader_control *shader_need,particle_use_type type_need)
{
	auto_Vinput_need = NULL;
	first_Vdraw_need = NULL;
	auto_Vstream0ut_need = NULL;
	random_tex = NULL;
	device_pancy = device_need;
	contex_pancy = contex_need;
	particle_num = max_particle_num;
	shader_list = shader_need;
	particle_type = type_need;
}
template<typename T>
HRESULT particle_system<T>::create(wchar_t *texture_path) 
{
	HRESULT hr_need = init_texture(texture_path);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	reset();
	return init_vertex_buff();
}
template<typename T>
HRESULT particle_system<T>::init_texture(wchar_t *texture_path)
{
	random_tex = CreateRandomTexture1DSRV(device_pancy);
	if (random_tex == NULL)
	{
		MessageBox(0, L"create particle tex error", L"tip", MB_OK);
		return E_FAIL;
	}
	HRESULT hr_need = CreateDDSTextureFromFile(device_pancy, texture_path, 0, &particle_tex, 0, 0);
	if (random_tex == NULL)
	{
		MessageBox(0, L"create random tex error", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
template<typename T>
HRESULT particle_system<T>::init_vertex_buff()
{
	HRESULT hr;
	//��ʼ���㻺��
	D3D11_BUFFER_DESC VB_desc;
	VB_desc.Usage = D3D11_USAGE_DEFAULT;
	VB_desc.ByteWidth = sizeof(T) * 1;
	VB_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VB_desc.CPUAccessFlags = 0;
	VB_desc.MiscFlags = 0;
	VB_desc.StructureByteStride = 0;
	T p;
	ZeroMemory(&p, sizeof(T));
	p.Age = 0.0f;
	p.Type = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;
	hr = device_pancy->CreateBuffer(&VB_desc, &vinitData, &first_Vdraw_need);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"create particle vertex error",L"tip",MB_OK);
		return hr;
	}
	//�����н����Ķ��㻺��
	VB_desc.ByteWidth = sizeof(T) * particle_num;
	VB_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	device_pancy->CreateBuffer(&VB_desc, 0, &auto_Vinput_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"create particle vertex error", L"tip", MB_OK);
		return hr;
	}
	device_pancy->CreateBuffer(&VB_desc, 0, &auto_Vstream0ut_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"create particle vertex error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
template<typename T>
void particle_system<T>::update(float delta_time, float time_world, XMFLOAT4X4 *mat_proj, XMFLOAT3 *view_direct)
{
	shader_particle *particle_fx;
	if (particle_type == PARTICLE_TYPE_FIRE)
	{
		particle_fx = shader_list->get_shader_fireparticle();       //������ɫ��
	}
	else
	{
		particle_fx = NULL;
		return;
	}
	time_delta = delta_time;
	game_time = time_world;
	age_need += delta_time;
	particle_fx->set_trans_all(mat_proj);
	particle_fx->set_viewposition(*view_direct);
	particle_fx->set_frametime(time_world, delta_time);
}
template<typename T>
void particle_system<T>::reset()
{
	if_firstrun = true;
	game_time = 0.0f;
	age_need = 0.0f;
	time_delta = 0.0f;
}
template<typename T>
void particle_system<T>::release()
{
	safe_release(auto_Vinput_need);
	safe_release(first_Vdraw_need);
	safe_release(auto_Vstream0ut_need);
	safe_release(particle_tex);
	safe_release(random_tex);
}
template<typename T>
void particle_system<T>::set_particle_direct(XMFLOAT3 *position, XMFLOAT3 *direct)
{
	start_pos = *position;
	start_dir = *direct;
}
template<typename T>
void particle_system<T>::draw_particle()
{
	shader_particle *particle_fx;
	if (particle_type == PARTICLE_TYPE_FIRE)
	{
		particle_fx = shader_list->get_shader_fireparticle();       //������ɫ��
	}
	else 
	{
		particle_fx = NULL;
		return;
	}
	particle_fx->set_startposition(start_pos);
	particle_fx->set_startdirection(start_dir);
	particle_fx->set_texture(particle_tex);
	particle_fx->set_randomtex(random_tex);
	//���ӵĸ���shader
	ID3DX11EffectTechnique                *teque_need;          //����·��
	particle_fx->get_technique(&teque_need, "StreamOutTech");
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(T);
	UINT offset = 0;

	//���ݷ��������ӵ�λ��ȷ�����㻺��
	if (if_firstrun)
		contex_pancy->IASetVertexBuffers(0, 1, &first_Vdraw_need, &stride, &offset);
	else
		contex_pancy->IASetVertexBuffers(0, 1, &auto_Vinput_need, &stride, &offset);

	//����SO���ߵ�������㻺��
	contex_pancy->SOSetTargets(1, &auto_Vstream0ut_need, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);

		if (if_firstrun)
		{
			contex_pancy->Draw(1, 0);
			if_firstrun = false;
		}
		else
		{
			contex_pancy->DrawAuto();
		}
	}
	//���ӵ���Ⱦshader
	//��󶥵��������
	ID3D11Buffer* bufferArray[1] = { 0 };
	contex_pancy->SOSetTargets(1, bufferArray, &offset);

	// ���������������
	std::swap(auto_Vinput_need, auto_Vstream0ut_need);
	//����������������ɵ�����
	contex_pancy->IASetVertexBuffers(0, 1, &auto_Vinput_need, &stride, &offset);
	particle_fx->get_technique(&teque_need, "DrawTech");
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawAuto();
	}
}

#pragma once
#include<windows.h>
#include<string.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<directxmath.h>
#include<d3dx11effect.h>
#include<DDSTextureLoader.h>
#include"shader_pancy.h"
#include"geometry.h"
using namespace DirectX;
struct BoundingSphere
{
	XMFLOAT3 Center;
	float Radius;
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f)
	{
	}
};

class shadow_basic
{
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	int                      shadowmap_width;
	int                      shadowmap_height;
	ID3D11ShaderResourceView *depthmap_tex;     //���������Ϣ��������Դ
	ID3D11DepthStencilView   *depthmap_target;  //������ȾĿ��Ļ�������Դ(�������������Դָ�빲��һƬtexture��Դ��ʵ��render depth to teture)
	D3D11_VIEWPORT           shadow_map_VP;     //�ӿ���Ⱦ��Ϣ
	XMFLOAT4X4               shadow_build;      //����shadowmap����Ҫ�ľ���
	XMFLOAT4X4               shadow_rebuild;    //����shadowmap����Ҫ�ľ���
	shader_control           *shader_list;
public:
	shadow_basic(ID3D11Device *device_need, ID3D11DeviceContext* contex_need, shader_control *shader_list_need);
	HRESULT set_viewport(int width_need, int height_need);
	HRESULT set_renderstate(XMFLOAT3 light_position, XMFLOAT3 light_dir, BoundingSphere shadow_range, light_type check);
	HRESULT set_renderstate(XMFLOAT4X4 shadow_matrix);
	HRESULT set_shaderresource(XMFLOAT4X4 word_matrix);
	HRESULT set_shaderresource(XMFLOAT4X4 *word_matrix_array,int mat_num);
	HRESULT set_bone_matrix(XMFLOAT4X4 *bone_matrix, int cnt_need);
	virtual HRESULT create(int width_need, int height_need);
	ID3D11ShaderResourceView* get_mapresource();
	ID3DX11EffectTechnique* get_technique();
	ID3DX11EffectTechnique* get_technique_transparent();
	ID3DX11EffectTechnique* get_technique_plant();
	ID3DX11EffectTechnique* get_technique_skin();
	ID3DX11EffectTechnique* get_technique_skin_transparent();
	HRESULT set_transparent_tex(ID3D11ShaderResourceView *tex_in);
	XMFLOAT4X4 get_ViewProjTex_matrix() {return shadow_rebuild;};
	HRESULT init_texture(ID3D11Texture2D* depthMap_array,int index_need);
	void release();
};

class pancy_shadow_volume 
{
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	shader_control           *shader_list;
	ID3D11Buffer             *vertex_need;       //�������������
	ID3DX11EffectTechnique   *teque_need;       //��Ⱦ·��
	ID3DX11EffectTechnique   *teque_transparent;//��Ⱦ·��
public:
	pancy_shadow_volume(ID3D11Device *device_need, ID3D11DeviceContext* contex_need, shader_control *shader_list_need);
	HRESULT set_renderstate(ID3D11DepthStencilView* depth_input, XMFLOAT3 light_position, XMFLOAT3 light_dir, light_type check);
	HRESULT set_view_projmat(XMFLOAT4X4 mat_need);
	HRESULT set_shaderresource(XMFLOAT4X4 word_matrix);
	ID3DX11EffectTechnique* get_technique() { return teque_need; };
	ID3DX11EffectTechnique* get_technique_transparent() { return teque_transparent; };
	HRESULT create(int buffer_num);
	void draw_SOvertex();
	void release();
private:
	HRESULT init_buffer(int buffer_num);
};

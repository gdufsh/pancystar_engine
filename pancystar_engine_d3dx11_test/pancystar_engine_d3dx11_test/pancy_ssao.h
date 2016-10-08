#pragma once
//ssao
#include"shader_pancy.h"
#include"geometry.h"
#include"pancy_DXrenderstate.h"
#include"pancy_model_import.h"
class ssao_pancy
{
	int                      map_width;
	int                      map_height;
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	pancy_renderstate        *renderstate_lib;
	ID3D11Buffer             *AoMap_VB;            //aoͼƬ���㻺����
	ID3D11Buffer             *AoMap_IB;            //aoͼƬ����������
	ID3D11ShaderResourceView *randomtex;           //���������Դ

	ID3D11ShaderResourceView *normaldepth_tex;     //�洢���ߺ���ȵ�������Դ
	ID3D11ShaderResourceView *depth_tex;     //�洢���ߺ���ȵ�������Դ

	ID3D11RenderTargetView   *ambient_target0;     //�洢ssao����ȾĿ��
	ID3D11ShaderResourceView *ambient_tex0;        //�洢ssao��������Դ

	ID3D11RenderTargetView   *ambient_target1;     //�洢���ڽ�����ssao����ȾĿ��
	ID3D11ShaderResourceView *ambient_tex1;        //�洢���ڽ�����ssao��������Դ

	XMFLOAT4                 FrustumFarCorner[4];  //ͶӰ�ӽ����Զ������ĸ��ǵ�
	XMFLOAT4                 random_Offsets[14];   //ʮ�ĸ����ڼ���ao�����������
	D3D11_VIEWPORT           render_viewport;      //�ӿ���Ϣ

	shader_control           *shader_list;         //shader��
	geometry_control         *geometry_lib;        //�������
public:
	ssao_pancy(pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_lib_need, int width, int height, float fovy, float farZ);
	HRESULT basic_create();
	void compute_ssaomap();
	void get_normaldepthmap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	ID3D11ShaderResourceView* get_aomap();
	void blur_ssaomap();
	void check_ssaomap();
	void release();
private:
	void set_size(int width, int height, float fovy, float farZ);
	void build_fullscreen_picturebuff();
	void BuildFrustumFarCorners(float fovy, float farZ);
	HRESULT build_texture();
	void build_offset_vector();
	HRESULT build_randomtex();
	void basic_blur(ID3D11ShaderResourceView *texin, ID3D11RenderTargetView *texout, bool if_row);
	template<class T>
	void safe_release(T t)
	{
		if (t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
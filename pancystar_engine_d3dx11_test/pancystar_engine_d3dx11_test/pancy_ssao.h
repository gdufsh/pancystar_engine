#pragma once
//ssao
#include"shader_pancy.h"
#include"geometry.h"
#include"pancy_DXrenderstate.h"
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

	ID3D11DepthStencilView   *depthmap_target;     //��ʱ��Ȼ�����
	ID3D11RenderTargetView   *normaldepth_target;  //�洢���ߺ���ȵ���ȾĿ��

	ID3D11ShaderResourceView *normaldepth_tex;     //�洢���ߺ���ȵ�������Դ

	ID3D11RenderTargetView   *ambient_target0;     //�洢ssao����ȾĿ��
	ID3D11ShaderResourceView *ambient_tex0;        //�洢ssao��������Դ

	ID3D11RenderTargetView   *ambient_target1;     //�洢���ڽ�����ssao����ȾĿ��
	ID3D11ShaderResourceView *ambient_tex1;        //�洢���ڽ�����ssao��������Դ

	XMFLOAT4                 FrustumFarCorner[4];  //ͶӰ�ӽ����Զ������ĸ��ǵ�
	XMFLOAT4                 random_Offsets[14];   //ʮ�ĸ����ڼ���ao�����������
	D3D11_VIEWPORT           render_viewport;      //�ӿ���Ϣ

	shader_control           *shader_list;         //shader��
	//ID3DX11EffectTechnique   *teque_need;          //ͨ����Ⱦ·��
	//ID3DX11EffectTechnique   *teque_transparent;   //��͸����Ⱦ·��
public:
	ssao_pancy(pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, int width, int height, float fovy, float farZ);
	HRESULT basic_create();
	void set_normaldepth_target(ID3D11DepthStencilView* dsv);
	HRESULT set_normaldepth_mat(XMFLOAT4X4 world_mat, XMFLOAT4X4 view_mat, XMFLOAT4X4 final_mat);
	HRESULT set_bone_matrix(XMFLOAT4X4 *bone_matrix, int cnt_need);
	HRESULT set_transparent_tex(ID3D11ShaderResourceView *tex_in);
	ID3DX11EffectTechnique* get_technique();
	ID3DX11EffectTechnique* get_technique_transparent();
	ID3DX11EffectTechnique* get_technique_skin();
	ID3DX11EffectTechnique* get_technique_skin_transparent();
	void compute_ssaomap();
	ID3D11ShaderResourceView* get_aomap();
	ID3D11DepthStencilView* get_depthstencilmap() { return depthmap_target; };
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
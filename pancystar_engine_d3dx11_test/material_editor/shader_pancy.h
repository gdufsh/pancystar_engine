#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
//#include<d3dx11dbg.h>
#include<directxmath.h>
#include <sstream>
#include <fstream>
#include <vector>
#include<d3dcompiler.h>
using namespace DirectX;
enum light_type 
{
	direction_light = 0,
	point_light     = 1,
	spot_light      = 2
};
enum shadow_type
{
	shadow_none = 0,
	shadow_map = 1,
	shadow_volume = 2
};
struct pancy_light_basic 
{
	XMFLOAT4    ambient;
	XMFLOAT4    diffuse;
	XMFLOAT4    specular;

	XMFLOAT3    dir;
	float       spot;

	XMFLOAT3    position;
	float       theta;

	XMFLOAT3    decay;
	float       range;

	XMUINT4    light_type;
};
struct material_handle//Ϊshader�в�����ص�ȫ�ֱ�����ֵ�ľ������
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
};
struct pancy_material//���ʽṹ
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};
class shader_basic
{
protected:
	ID3D11Device                          *device_pancy;        //d3d�豸
	ID3D11DeviceContext                   *contex_pancy;        //�豸������
	ID3D11InputLayout                     *input_need;          //������shader�����������ĸ�ʽ
	ID3DX11Effect                         *fx_need;             //shader�ӿ�
	LPCWSTR                               shader_filename;      //shader�ļ���
public:
	shader_basic(LPCWSTR filename,ID3D11Device *device_need,ID3D11DeviceContext *contex_need);				 //���캯��������shader�ļ����ļ���
	HRESULT shder_create();
	HRESULT get_technique(ID3DX11EffectTechnique** tech_need,LPCSTR tech_name); //��ȡ��Ⱦ·��
	HRESULT get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //��ȡ������Ⱦ·��
	virtual void release() = 0;
protected:
	HRESULT combile_shader(LPCWSTR filename);		//shader����ӿ�
	virtual void init_handle() = 0;                 //ע��ȫ�ֱ������
	virtual void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member) = 0;
	HRESULT set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need);
	void release_basic();
	template<class T> 
	void safe_release(T t)
	{
		if(t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
class light_pre : public shader_basic 
{
	ID3DX11EffectVariable   *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectVariable   *material_need;              //����
	ID3DX11EffectVariable   *light_list;                 //�ƹ�
	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;     //shader�е�������Դ���
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;      //������ͼ����
	ID3DX11EffectShaderResourceVariable   *texture_shadow_handle;      //��Ӱ��ͼ���
	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;        //��������ͼ���

	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //���߱任���
	//ID3DX11EffectMatrixVariable           *texture_matrix_handle;    //����任���
	ID3DX11EffectMatrixVariable           *shadowmap_matrix_handle;    //shadowmap����任���
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;         //ssao����任���
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //�����任����
public:
	light_pre(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                          //��������任
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
	HRESULT set_trans_shadow(XMFLOAT4X4 *mat_need);                         //������Ӱ�任
	HRESULT set_trans_ssao(XMFLOAT4X4 *mat_need);                           //���û�����任
	virtual HRESULT set_material(pancy_material material_in);				//���ò���
	virtual HRESULT set_ssaotex(ID3D11ShaderResourceView *tex_in);			//����ssaomap
	virtual HRESULT set_shadowtex(ID3D11ShaderResourceView *tex_in);		//����shadowmap
	virtual HRESULT set_diffusetex(ID3D11ShaderResourceView *tex_in);		//��������������
	virtual HRESULT set_normaltex(ID3D11ShaderResourceView *tex_in);		//���÷�����ͼ����
	virtual HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //���ù����任����
	HRESULT set_light(pancy_light_basic light_need, int light_num);          //����һ���۹�ƹ�Դ
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class gui_simple : public shader_basic 
{
	ID3DX11EffectVariable   *move_handle; //�ƶ�
	ID3DX11EffectShaderResourceVariable   *texture_handle;        //��ͼ���
public:
	gui_simple(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_mov_xy(XMFLOAT2 mov_xy);
	HRESULT set_tex(ID3D11ShaderResourceView *tex_in);		//��������
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shader list~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_control
{
	light_pre                  *shader_light_pre;                //ǰ�������ɫ��
	gui_simple                 *shader_GUI;
	shader_basic *shader_light_deferred;
public:
	shader_control();
	HRESULT shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy);
	light_pre*                  get_shader_prelight() { return shader_light_pre; };
	gui_simple*                 get_shader_GUI() { return shader_GUI; };
	void release();
};


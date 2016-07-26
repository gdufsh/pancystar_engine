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
struct light_point_handle//Ϊshader�е��Դ��ص�ȫ�ֱ�����ֵ�ľ������
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
	ID3DX11EffectVariable *position;
	ID3DX11EffectVariable *decay;
};
struct pancy_light_dir//�����ṹ
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT3 dir;

	float    range;
};
struct pancy_light_point//���Դ�ṹ
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT3 position;
	float    range;

	XMFLOAT3 decay;
};
struct pancy_light_spot//�۹�ƽṹ
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
	light_point_handle      pointlight_handle;           //������Ϣ
	ID3DX11EffectVariable   *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectVariable   *material_need;              //����
	ID3DX11EffectVariable   *light_dir;                  //�����
	ID3DX11EffectVariable   *light_point;                //���Դ
	ID3DX11EffectVariable   *light_spot;                 //�۹��
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

	HRESULT set_dirlight(pancy_light_dir light_need,int light_num);         //����һ�������Դ
	HRESULT set_pointlight(pancy_light_point light_need, int light_num);    //����һ�����Դ
	HRESULT set_spotlight(pancy_light_spot light_need, int light_num);      //����һ���۹�ƹ�Դ
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_shadow : public shader_basic 
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //ȫ�׼��α任���
public:
	light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //�����ܱ任
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaodepthnormal_map : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //���߱任���
public:
	shader_ssaodepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_final);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_map_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaomap : public shader_basic
{
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectVectorVariable* OffsetVectors;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* RandomVecMap;
public:
	shader_ssaomap(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);

	HRESULT set_ViewToTexSpace(XMFLOAT4X4 *mat);
	HRESULT set_OffsetVectors(const XMFLOAT4 v[14]);
	HRESULT set_FrustumCorners(const XMFLOAT4 v[4]);
	HRESULT set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	HRESULT set_randomtex(ID3D11ShaderResourceView* srv);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_blur_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaoblur : public shader_basic
{
	ID3DX11EffectScalarVariable* TexelWidth;
	ID3DX11EffectScalarVariable* TexelHeight;

	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* InputImage;
public:
	shader_ssaoblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_image_size(float width, float height);
	HRESULT set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_control
{
	light_pre                  *shader_light_pre;          //ǰ�������ɫ��
	light_shadow               *shader_shadowmap;          //��Ӱ��ɫ��
	shader_ssaodepthnormal_map *shader_ssao_depthnormal;   //ssao���������ɫ��
	shader_ssaomap             *shader_ssao_draw;          //ssao�ڱ�ͼ��Ⱦ��ɫ��
	shader_ssaoblur            *shader_ssao_blur;          //ssaoģ����ɫ��

	shader_basic *shader_light_deferred;
	shader_basic *shader_cubemap;
	shader_basic *particle_build;
	shader_basic *particle_show;
public:
	shader_control();
	HRESULT shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy);
	light_pre* get_shader_prelight() { return shader_light_pre; };
	light_shadow* get_shader_shadowmap() { return shader_shadowmap; };
	shader_ssaodepthnormal_map* get_shader_ssaodepthnormal() {return shader_ssao_depthnormal;};
	shader_ssaomap* get_shader_ssaodraw() { return shader_ssao_draw; };
	shader_ssaoblur* get_shader_ssaoblur() { return shader_ssao_blur; };
	void release();
};


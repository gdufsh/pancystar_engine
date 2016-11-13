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
	ID3DX11EffectVariable                 *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectVariable                 *material_need;              //����
	ID3DX11EffectVariable                 *light_list;                 //�ƹ�
	ID3DX11EffectVariable                 *light_num_handle;           //��Դ����
	ID3DX11EffectVariable                 *shadow_num_handle;           //��Դ����
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
	HRESULT set_shadow_matrix(const XMFLOAT4X4* M, int cnt);		//������Ӱͼ�任����
	HRESULT set_trans_ssao(XMFLOAT4X4 *mat_need);                           //���û�����任
	HRESULT set_light_num(XMUINT3 all_light_num);                           //���ù�Դ(������Ӱ)����
	HRESULT set_shadow_num(XMUINT3 all_light_num);                          //���ù�Դ(������Ӱ)����
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
class light_shadow : public shader_basic 
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //ȫ�׼��α任���
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //�����任����
public:
	light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //�����ܱ任
	HRESULT set_texture(ID3D11ShaderResourceView *tex_in);
	HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //���ù����任����
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_gbufferdepthnormal_map : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //���߱任���
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //�����任����
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectShaderResourceVariable   *texture_normal;
public:
	shader_gbufferdepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_final);
	HRESULT set_texture(ID3D11ShaderResourceView *tex_in);
	HRESULT set_texture_normal(ID3D11ShaderResourceView *tex_in);
	HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //���ù����任����
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_map_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaomap : public shader_basic
{
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectVectorVariable* OffsetVectors;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* RandomVecMap;
public:
	shader_ssaomap(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);

	HRESULT set_ViewToTexSpace(XMFLOAT4X4 *mat);
	HRESULT set_OffsetVectors(const XMFLOAT4 v[14]);
	HRESULT set_FrustumCorners(const XMFLOAT4 v[4]);
	HRESULT set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	HRESULT set_Depthtex(ID3D11ShaderResourceView* srv);
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
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* InputImage;
public:
	shader_ssaoblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_image_size(float width, float height);
	HRESULT set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap);
	HRESULT set_Depthtex(ID3D11ShaderResourceView* srv);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cube mapping~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_reflect : public shader_basic 
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //����任���
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //���߱任���
	ID3DX11EffectVariable                 *view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectShaderResourceVariable   *cubemap_texture;            //������ͼ��Դ
public:
	shader_reflect(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);                                 //�����ӵ�λ��
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                          //��������任
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
	HRESULT set_tex_resource(ID3D11ShaderResourceView* tex_cube);           //����������Դ
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_average_part~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class compute_averagelight : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *texture_input;      //shader�е�������Դ���
	ID3DX11EffectUnorderedAccessViewVariable *buffer_input;       //shader�е�������Դ���
	ID3DX11EffectUnorderedAccessViewVariable *buffer_output;	  //compute_shader�������������Դ
	ID3DX11EffectVariable                    *texture_range;      //���������С
public:
	compute_averagelight(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_compute_tex(ID3D11ShaderResourceView *tex_input);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num,int bytewidth);
	HRESULT set_compute_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need);
	
	void release();
	void dispatch(int width_need, int height_need, int final_need, int map_need);
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_preblur_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRpreblur : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *tex_input;       //shader�е�������Դ���
	ID3DX11EffectShaderResourceVariable      *buffer_input;    //shader�е�������Դ���
	ID3DX11EffectVariable                    *lum_message;     //������Ϣ������
	ID3DX11EffectVariable                    *texture_range;   //���������С
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB;  //YUV2RGB�任���
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV;  //RGB2YUV�任���
public:
	shader_HDRpreblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_buffer_input(ID3D11ShaderResourceView *buffer_need, ID3D11ShaderResourceView *tex_need);
	//������Ϣ(ƽ�����ȣ��߹�ֽ�㣬�߹����ֵ��tonemapping����)
	HRESULT set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_blur_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRblur : public shader_basic
{
	ID3DX11EffectScalarVariable*             TexelWidth;
	ID3DX11EffectScalarVariable*             TexelHeight;
	ID3DX11EffectShaderResourceVariable      *tex_input;      //shader�е�������Դ���
public:
	shader_HDRblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_image_size(float width, float height);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_final_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRfinal : public shader_basic
{
	ID3DX11EffectVariable                    *lum_message;    //������Ϣ������
	ID3DX11EffectShaderResourceVariable      *tex_input;      //ԭʼͼ��
	ID3DX11EffectShaderResourceVariable      *tex_bloom;      //�����ع�ͼ��
	ID3DX11EffectShaderResourceVariable      *buffer_input;   //ƽ������buffer
	ID3DX11EffectVariable                    *texture_range;   //���������С
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB; //YUV2RGB�任���
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV; //RGB2YUV�任���
public:
	shader_HDRfinal(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_resource(ID3D11ShaderResourceView *tex_input, ID3D11ShaderResourceView *tex_bloom,ID3D11ShaderResourceView *buffer_need);
	//������Ϣ(ƽ�����ȣ��߹�ֽ�㣬�߹����ֵ��tonemapping����)
	HRESULT set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~particle_system~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_particle :public shader_basic//������ɫ��
{
	ID3DX11EffectVariable         *view_pos_handle;                //�ӵ�λ��
	ID3DX11EffectVariable         *start_position_handle;          //���Ӳ���Դ��λ��
	ID3DX11EffectVariable         *start_direction_handle;         //���Ӳ����ķ���
	ID3DX11EffectScalarVariable   *time_game_handle;               //���Ӳ���Դ��λ��
	ID3DX11EffectScalarVariable   *time_delta_handle;              //���Ӳ����ķ���
	ID3DX11EffectMatrixVariable   *project_matrix_handle;          //ȫ�׼��α任���
	ID3DX11EffectShaderResourceVariable   *texture_handle;         //������ͼ����
	ID3DX11EffectShaderResourceVariable   *RandomTex_handle;       //�������ͼ����
public:
	shader_particle(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_viewposition(XMFLOAT3 eye_pos);
	HRESULT set_startposition(XMFLOAT3 start_pos);
	HRESULT set_startdirection(XMFLOAT3 start_dir);
	HRESULT set_frametime(float game_time, float delta_time);
	HRESULT set_randomtex(ID3D11ShaderResourceView *tex_in);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);
	HRESULT set_texture(ID3D11ShaderResourceView *tex_in);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	virtual void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member) = 0;
};
class shader_fire :public shader_particle 
{
public:
	shader_fire(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
private:
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow volume~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_shadow_volume : public shader_basic
{
	ID3DX11EffectMatrixVariable *world_matrix_handle; //����任���
	ID3DX11EffectMatrixVariable *normal_matrix_handle; //���߱任���
	ID3DX11EffectMatrixVariable *project_matrix_handle;//ȫ�׼��α任���
	ID3DX11EffectVariable   *position_light_handle;   //��Դλ��
	ID3DX11EffectVariable   *direction_light_handle;  //��Դ����
public:
	shader_shadow_volume(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);      //��������任
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);
	HRESULT set_light_pos(XMFLOAT3 light_pos);
	HRESULT set_light_dir(XMFLOAT3 light_dir);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow volume draw~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_shadow_volume_draw : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //ȫ�׼��α任���
public:
	shader_shadow_volume_draw(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	void release();
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~grass_billboard~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_grass : public shader_basic
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //ȫ�׼��α任���
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectShaderResourceVariable   *texture_normal;
	ID3DX11EffectShaderResourceVariable   *texture_specular;
	ID3DX11EffectVariable   *view_pos_handle;            //�ӵ�λ��
public:
	shader_grass(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //�����ܱ任
	HRESULT set_texture_diffuse(ID3D11ShaderResourceView *tex_in);
	HRESULT set_texture_normal(ID3D11ShaderResourceView *tex_in);
	HRESULT set_texture_specular(ID3D11ShaderResourceView *tex_in);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~resolve_depth stencilview~~~~~~~~~~~~~~~~~~~~~~
class shader_resolvedepth : public shader_basic
{

	ID3DX11EffectShaderResourceVariable   *texture_MSAA;
	ID3DX11EffectVariable   *projmessage_handle;            //�ӵ�λ��
public:
	shader_resolvedepth(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_texture_MSAA(ID3D11ShaderResourceView *tex_in);
	HRESULT set_projmessage(XMFLOAT3 proj_message);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~defered lighting lightbuffer~~~~~~~~~~~~~~~~~~~~~
class light_defered_lightbuffer : public shader_basic
{
	ID3DX11EffectVariable                 *light_list;                 //�ƹ�
	ID3DX11EffectVariable                 *light_num_handle;           //��Դ����
	ID3DX11EffectVariable                 *shadow_num_handle;           //��Դ����
	ID3DX11EffectMatrixVariable           *shadow_matrix_handle;       //��Ӱͼ�任
	ID3DX11EffectMatrixVariable           *view_matrix_handle;         //ȡ���任���
	ID3DX11EffectMatrixVariable           *invview_matrix_handle;      //ȡ���任��任���
	ID3DX11EffectVectorVariable           *FrustumCorners;             //3D��ԭ�ǵ�
	ID3DX11EffectShaderResourceVariable   *NormalspecMap;             //���߾����������Դ���
	ID3DX11EffectShaderResourceVariable   *DepthMap;                   //���������Դ���
	ID3DX11EffectShaderResourceVariable   *texture_shadow;             //��Ӱ������Դ���
public:
	light_defered_lightbuffer(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_light(pancy_light_basic light_need, int light_num); //����һ����Դ
	HRESULT set_light_num(XMUINT3 all_light_num);                   //���ù�Դ����
	HRESULT set_shadow_num(XMUINT3 all_light_num);                  //���ù�Դ����
	HRESULT set_FrustumCorners(const XMFLOAT4 v[4]);                //����3D��ԭ�ǵ�
	HRESULT set_shadow_matrix(const XMFLOAT4X4* M, int cnt);		//������Ӱͼ�任����
	HRESULT set_view_matrix(XMFLOAT4X4 *mat_need);                  //����ȡ���任
	HRESULT set_invview_matrix(XMFLOAT4X4 *mat_need);                  //����ȡ���任

	HRESULT set_Normalspec_tex(ID3D11ShaderResourceView *tex_in);	//���÷��߾��������
	HRESULT set_DepthMap_tex(ID3D11ShaderResourceView *tex_in);		//�����������
	HRESULT set_shadow_tex(ID3D11ShaderResourceView *tex_in);		//������Ӱ����
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~deffered lighting draw~~~~~~~~~~~~~~~~~~~~~~~~~~~
class light_defered_draw : public shader_basic
{
	ID3DX11EffectVariable                 *material_need;            //����
	ID3DX11EffectVariable                 *view_pos_handle;          //�ӵ�λ��
	ID3DX11EffectMatrixVariable           *world_matrix_handle;      //����任���
	ID3DX11EffectMatrixVariable           *final_matrix_handle;      //ȫ�׼��α任���
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;       //ssao����任���
	ID3DX11EffectMatrixVariable           *BoneTransforms;           //�����任����
	ID3DX11EffectShaderResourceVariable   *tex_light_diffuse_handle; //�������������Դ���
	ID3DX11EffectShaderResourceVariable   *tex_light_specular_handle;//�����������Դ���
	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;      //������������Դ���
	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;   //������������Դ���
	ID3DX11EffectShaderResourceVariable   *texture_cube_handle;
public:
	light_defered_draw(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	HRESULT set_trans_ssao(XMFLOAT4X4 *mat_need);                   //���û�����任
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                  //��������任
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                    //�����ܱ任
	HRESULT set_material(pancy_material material_in);				//���ò���
	HRESULT set_ssaotex(ID3D11ShaderResourceView *tex_in);			//����ssaomap
	HRESULT set_diffusetex(ID3D11ShaderResourceView *tex_in);		//��������������
	HRESULT set_diffuse_light_tex(ID3D11ShaderResourceView *tex_in);//���������������
	HRESULT set_specular_light_tex(ID3D11ShaderResourceView *tex_in);//���þ��淴�������
	HRESULT set_enviroment_tex(ID3D11ShaderResourceView* srv);
	virtual HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);	 //���ù����任����
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~real time local reflection~~~~~~~~~~~~~~~~~~~~~~~
class ssr_reflect : public shader_basic 
{
	ID3DX11EffectVariable*       view_pos_handle;            //�ӵ�λ��
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectMatrixVariable* view_matrix_handle;         //ȡ���任���
	ID3DX11EffectMatrixVariable* invview_matrix_handle;      //ȡ���任��任���
	ID3DX11EffectMatrixVariable* cubeview_matrix_handle;     //cubemap������ȡ���任����
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectVectorVariable* camera_positions;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* DepthMap;
	ID3DX11EffectShaderResourceVariable* texture_diffuse_handle;
	ID3DX11EffectShaderResourceVariable* texture_cube_handle;
	//ID3DX11EffectShaderResourceVariable* texture_depthcube_handle;
	ID3DX11EffectShaderResourceVariable* texture_stencilcube_handle;

	ID3DX11EffectShaderResourceVariable* texture_color_mask;
	ID3DX11EffectShaderResourceVariable* texture_color_ssr;
public:
	ssr_reflect(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_ViewToTexSpace(XMFLOAT4X4 *mat);
	HRESULT set_FrustumCorners(const XMFLOAT4 v[4]);
	HRESULT set_camera_positions(XMFLOAT3 v);
	HRESULT set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	HRESULT set_Depthtex(ID3D11ShaderResourceView* srv);
	HRESULT set_diffusetex(ID3D11ShaderResourceView* srv);
	HRESULT set_enviroment_tex(ID3D11ShaderResourceView* srv);
	//HRESULT set_enviroment_depth(ID3D11ShaderResourceView* srv);
	HRESULT set_enviroment_stencil(ID3D11ShaderResourceView* srv);
	HRESULT set_color_mask_tex(ID3D11ShaderResourceView* srv);
	HRESULT set_color_ssr_tex(ID3D11ShaderResourceView* srv);
	HRESULT set_invview_matrix(XMFLOAT4X4 *mat_need);                  //����ȡ����任
	HRESULT set_view_matrix(XMFLOAT4X4 *mat_need);                     //����ȡ���任
	HRESULT set_cubeview_matrix(const XMFLOAT4X4* M, int cnt);	       //��������ȡ������
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~save cube to alpha~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_save_cube : public shader_basic
{
	ID3DX11EffectVariable         *cube_count_handle;
	ID3DX11EffectShaderResourceVariable   *texture_input;
	ID3DX11EffectShaderResourceVariable   *depth_input;
public:
	shader_save_cube(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_texture_input(ID3D11ShaderResourceView *tex_in);
	HRESULT set_depthtex_input(ID3D11ShaderResourceView *tex_in);
	HRESULT set_cube_count(XMFLOAT3 cube_count);
	void release();
private:
	void init_handle();                 //ע��ȫ�ֱ������
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssr reflect blur~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_SSRblur : public shader_basic
{
	ID3DX11EffectVariable*             Texelrange;
	ID3DX11EffectShaderResourceVariable      *tex_input;      //shader�е�������Դ���
	ID3DX11EffectShaderResourceVariable      *tex_normal_input;      //shader�е�������Դ���
	ID3DX11EffectShaderResourceVariable      *tex_depth_input;      //shader�е�������Դ���
public:
	shader_SSRblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_tex_normal_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_tex_depth_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_image_size(XMFLOAT4 texel_range);

	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~reflect final pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_reflectfinal : public shader_basic
{
	ID3DX11EffectVariable                    *Texelrange;
	ID3DX11EffectShaderResourceVariable      *tex_color_input;      //shader�е�������Դ���
	ID3DX11EffectShaderResourceVariable      *tex_reflect_input;      //shader�е�������Դ���
public:
	shader_reflectfinal(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_color_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_tex_reflect_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_image_size(XMFLOAT4 texel_range);
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shader list~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_control
{
	light_pre                  *shader_light_pre;                //ǰ�������ɫ��
	light_defered_lightbuffer  *shader_light_deffered_lbuffer;   //�ӳٹ��չ��ջ�����ɫ��
	light_defered_draw         *shader_light_deffered_draw;      //�ӳٹ�����Ⱦ
	light_shadow               *shader_shadowmap;                //��Ӱͼ��ɫ��
	shader_shadow_volume       *shader_shadowvolume;             //��Ӱ����ɫ��
	shader_shadow_volume_draw  *shader_shadowvolume_draw;        //��Ӱ�����
	shader_resolvedepth        *shader_resolve_depthstencil;     //msaa���ģ�建�����ز���
	shader_gbufferdepthnormal_map *shader_gbuffer_depthnormal;   //ssao���������ɫ��
	shader_ssaomap             *shader_ssao_draw;                //ssao�ڱ�ͼ��Ⱦ��ɫ��
	shader_ssaoblur            *shader_ssao_blur;                //ssaoģ����ɫ��
	shader_reflect             *shader_cubemap;                  //������ͼ��ɫ��
	compute_averagelight       *shader_HDR_average;              //HDR����ƽ��
	shader_HDRpreblur          *shader_HDR_preblur;              //HDR�߹���ȡ
	shader_HDRblur             *shader_HDR_blur;                 //HDR�߹�ģ��
	shader_HDRfinal            *shader_HDR_final;                //HDR���ս��
	shader_particle            *particle_fire;                   //����ϵͳ��ɫ��
	shader_grass               *shader_grass_billboard;          //�ݵع����
	ssr_reflect                *shader_ssreflect;                //��Ļ�ռ䷴��
	shader_save_cube           *shader_reset_alpha;              //�洢cube����alpha
	shader_SSRblur             *shader_reflect_blur;             //������ͼ��˹ģ��
	shader_reflectfinal        *shader_reflect_final;            //���յķ���ϳ�
	//shader_basic *shader_light_deferred;
public:
	shader_control();
	HRESULT shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy);
	light_pre*                  get_shader_prelight() { return shader_light_pre; };
	light_shadow*               get_shader_shadowmap() { return shader_shadowmap; };
	shader_shadow_volume*       get_shader_shadowvolume() { return shader_shadowvolume; };
	shader_shadow_volume_draw*  get_shader_shadowvolume_draw() { return shader_shadowvolume_draw; };
	shader_resolvedepth*        get_shader_resolve_depthstencil() { return shader_resolve_depthstencil; };
	shader_gbufferdepthnormal_map* get_shader_gbufferdepthnormal() {return shader_gbuffer_depthnormal;};
	shader_ssaomap*             get_shader_ssaodraw() { return shader_ssao_draw; };
	shader_ssaoblur*            get_shader_ssaoblur() { return shader_ssao_blur; };
	shader_reflect*             get_shader_reflect() { return shader_cubemap; };
	compute_averagelight*       get_shader_HDRaverage() { return shader_HDR_average; };
	shader_HDRpreblur*          get_shader_HDRpreblur() { return shader_HDR_preblur; };
	shader_HDRblur*             get_shader_HDRblur() { return shader_HDR_blur; };
	shader_HDRfinal*            get_shader_HDRfinal() { return shader_HDR_final; };
	shader_particle*            get_shader_fireparticle() { return particle_fire; };
	shader_grass*               get_shader_grass_billboard() { return shader_grass_billboard; };
	light_defered_lightbuffer*  get_shader_defferedlight_lightbuffer() { return shader_light_deffered_lbuffer; };
	light_defered_draw*         get_shader_light_deffered_draw() { return  shader_light_deffered_draw; };
	ssr_reflect*                get_shader_ssreflect() { return shader_ssreflect; };
	shader_save_cube*           get_shader_cubesave() { return shader_reset_alpha; };
	shader_SSRblur*             get_shader_reflect_blur() { return shader_reflect_blur; };
	shader_reflectfinal*        get_shader_reflect_final() { return shader_reflect_final; };
	void release();
};
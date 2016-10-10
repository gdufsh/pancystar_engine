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
struct material_handle//为shader中材质相关的全局变量赋值的句柄集合
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
};
struct pancy_material//材质结构
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

class shader_basic
{
protected:
	ID3D11Device                          *device_pancy;        //d3d设备
	ID3D11DeviceContext                   *contex_pancy;        //设备描述表
	ID3D11InputLayout                     *input_need;          //定义了shader及其接受输入的格式
	ID3DX11Effect                         *fx_need;             //shader接口
	LPCWSTR                               shader_filename;      //shader文件名
public:
	shader_basic(LPCWSTR filename,ID3D11Device *device_need,ID3D11DeviceContext *contex_need);				 //构造函数，输入shader文件的文件名
	HRESULT shder_create();
	HRESULT get_technique(ID3DX11EffectTechnique** tech_need,LPCSTR tech_name); //获取渲染路径
	HRESULT get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name); //获取特殊渲染路径
	virtual void release() = 0;
protected:
	HRESULT combile_shader(LPCWSTR filename);		//shader编译接口
	virtual void init_handle() = 0;                 //注册全局变量句柄
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
	ID3DX11EffectVariable   *view_pos_handle;            //视点位置
	ID3DX11EffectVariable   *material_need;              //材质
	ID3DX11EffectVariable   *light_list;                 //灯光
	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;     //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;      //法线贴图纹理
	ID3DX11EffectShaderResourceVariable   *texture_shadow_handle;      //阴影贴图句柄
	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;        //环境光贴图句柄

	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	//ID3DX11EffectMatrixVariable           *texture_matrix_handle;    //纹理变换句柄
	ID3DX11EffectMatrixVariable           *shadowmap_matrix_handle;    //shadowmap矩阵变换句柄
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;         //ssao矩阵变换句柄
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //骨骼变换矩阵
public:
	light_pre(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                          //设置世界变换
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	HRESULT set_trans_shadow(XMFLOAT4X4 *mat_need);                         //设置阴影变换
	HRESULT set_trans_ssao(XMFLOAT4X4 *mat_need);                           //设置环境光变换
	virtual HRESULT set_material(pancy_material material_in);				//设置材质
	virtual HRESULT set_ssaotex(ID3D11ShaderResourceView *tex_in);			//设置ssaomap
	virtual HRESULT set_shadowtex(ID3D11ShaderResourceView *tex_in);		//设置shadowmap
	virtual HRESULT set_diffusetex(ID3D11ShaderResourceView *tex_in);		//设置漫反射纹理
	virtual HRESULT set_normaltex(ID3D11ShaderResourceView *tex_in);		//设置法线贴图纹理
	virtual HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //设置骨骼变换矩阵
	HRESULT set_light(pancy_light_basic light_need, int light_num);          //设置一个聚光灯光源
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_shadow : public shader_basic 
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //全套几何变换句柄
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //骨骼变换矩阵
public:
	light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //设置总变换
	HRESULT set_texture(ID3D11ShaderResourceView *tex_in);
	HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //设置骨骼变换矩阵
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_gbufferdepthnormal_map : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	ID3DX11EffectMatrixVariable           *BoneTransforms;             //骨骼变换矩阵
	ID3DX11EffectShaderResourceVariable   *texture_need;
public:
	shader_gbufferdepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_final);
	HRESULT set_texture(ID3D11ShaderResourceView *tex_in);
	HRESULT set_bone_matrix(const XMFLOAT4X4* M, int cnt);		     //设置骨骼变换矩阵
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
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
	void init_handle();//注册shader中所有全局变量的句柄
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
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cube mapping~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_reflect : public shader_basic 
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	ID3DX11EffectVariable                 *view_pos_handle;            //视点位置
	ID3DX11EffectShaderResourceVariable   *cubemap_texture;            //立方贴图资源
public:
	shader_reflect(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);                                 //设置视点位置
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                          //设置世界变换
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	HRESULT set_tex_resource(ID3D11ShaderResourceView* tex_cube);           //设置纹理资源
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_average_part~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class compute_averagelight : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *texture_input;      //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *buffer_input;       //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *buffer_output;	  //compute_shader计算完毕纹理资源
	ID3DX11EffectVariable                    *texture_range;      //输入纹理大小
public:
	compute_averagelight(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_compute_tex(ID3D11ShaderResourceView *tex_input);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num,int bytewidth);
	HRESULT set_compute_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need);
	
	void release();
	void dispatch(int width_need, int height_need, int final_need, int map_need);
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_preblur_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRpreblur : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *tex_input;       //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable      *buffer_input;    //shader中的纹理资源句柄
	ID3DX11EffectVariable                    *lum_message;     //亮度信息及参数
	ID3DX11EffectVariable                    *texture_range;   //输入纹理大小
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB;  //YUV2RGB变换句柄
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV;  //RGB2YUV变换句柄
public:
	shader_HDRpreblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_buffer_input(ID3D11ShaderResourceView *buffer_need, ID3D11ShaderResourceView *tex_need);
	//亮度信息(平均亮度，高光分界点，高光最大值，tonemapping参数)
	HRESULT set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_blur_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRblur : public shader_basic
{
	ID3DX11EffectScalarVariable*             TexelWidth;
	ID3DX11EffectScalarVariable*             TexelHeight;
	ID3DX11EffectShaderResourceVariable      *tex_input;      //shader中的纹理资源句柄
public:
	shader_HDRblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_resource(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_image_size(float width, float height);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_final_pass~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_HDRfinal : public shader_basic
{
	ID3DX11EffectVariable                    *lum_message;    //亮度信息及参数
	ID3DX11EffectShaderResourceVariable      *tex_input;      //原始图像
	ID3DX11EffectShaderResourceVariable      *tex_bloom;      //高亮曝光图形
	ID3DX11EffectShaderResourceVariable      *buffer_input;   //平均亮度buffer
	ID3DX11EffectVariable                    *texture_range;   //输入纹理大小
	ID3DX11EffectMatrixVariable              *matrix_YUV2RGB; //YUV2RGB变换句柄
	ID3DX11EffectMatrixVariable              *matrix_RGB2YUV; //RGB2YUV变换句柄
public:
	shader_HDRfinal(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_tex_resource(ID3D11ShaderResourceView *tex_input, ID3D11ShaderResourceView *tex_bloom,ID3D11ShaderResourceView *buffer_need);
	//亮度信息(平均亮度，高光分界点，高光最大值，tonemapping参数)
	HRESULT set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping);
	HRESULT set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~particle_system~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_particle :public shader_basic//粒子着色器
{
	ID3DX11EffectVariable         *view_pos_handle;                //视点位置
	ID3DX11EffectVariable         *start_position_handle;          //粒子产生源的位置
	ID3DX11EffectVariable         *start_direction_handle;         //粒子产生的方向
	ID3DX11EffectScalarVariable   *time_game_handle;               //粒子产生源的位置
	ID3DX11EffectScalarVariable   *time_delta_handle;              //粒子产生的方向
	ID3DX11EffectMatrixVariable   *project_matrix_handle;          //全套几何变换句柄
	ID3DX11EffectShaderResourceVariable   *texture_handle;         //粒子贴图纹理
	ID3DX11EffectShaderResourceVariable   *RandomTex_handle;       //随机数贴图纹理
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
	void init_handle();//注册shader中所有全局变量的句柄
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
	ID3DX11EffectMatrixVariable *world_matrix_handle; //世界变换句柄
	ID3DX11EffectMatrixVariable *normal_matrix_handle; //法线变换句柄
	ID3DX11EffectMatrixVariable *project_matrix_handle;//全套几何变换句柄
	ID3DX11EffectVariable   *position_light_handle;   //光源位置
	ID3DX11EffectVariable   *direction_light_handle;  //光源方向
public:
	shader_shadow_volume(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);      //设置世界变换
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);
	HRESULT set_light_pos(XMFLOAT3 light_pos);
	HRESULT set_light_dir(XMFLOAT3 light_dir);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow volume draw~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_shadow_volume_draw : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
public:
	shader_shadow_volume_draw(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	void release();
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~grass_billboard~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_grass : public shader_basic
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //全套几何变换句柄
	ID3DX11EffectShaderResourceVariable   *texture_need;
	ID3DX11EffectShaderResourceVariable   *texture_normal;
	ID3DX11EffectShaderResourceVariable   *texture_specular;
	ID3DX11EffectVariable   *view_pos_handle;            //视点位置
public:
	shader_grass(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //设置总变换
	HRESULT set_texture_diffuse(ID3D11ShaderResourceView *tex_in);
	HRESULT set_texture_normal(ID3D11ShaderResourceView *tex_in);
	HRESULT set_texture_specular(ID3D11ShaderResourceView *tex_in);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_resolvedepth : public shader_basic
{
	ID3DX11EffectShaderResourceVariable   *texture_MSAA;
	ID3DX11EffectVariable   *projmessage_handle;            //视点位置
public:
	shader_resolvedepth(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_texture_MSAA(ID3D11ShaderResourceView *tex_in);
	HRESULT set_projmessage(XMFLOAT3 proj_message);
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shader list~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_control
{
	light_pre                  *shader_light_pre;                //前向光照着色器
	light_shadow               *shader_shadowmap;                //阴影图着色器
	shader_shadow_volume       *shader_shadowvolume;             //阴影体着色器
	shader_shadow_volume_draw  *shader_shadowvolume_draw;        //阴影体绘制
	shader_resolvedepth        *shader_resolve_depthstencil;     //msaa深度模板缓冲区重采样
	shader_gbufferdepthnormal_map *shader_gbuffer_depthnormal;   //ssao深度纹理着色器
	shader_ssaomap             *shader_ssao_draw;                //ssao遮蔽图渲染着色器
	shader_ssaoblur            *shader_ssao_blur;                //ssao模糊着色器
	shader_reflect             *shader_cubemap;                  //立方贴图着色器
	compute_averagelight       *shader_HDR_average;              //HDR像素平均
	shader_HDRpreblur          *shader_HDR_preblur;              //HDR高光提取
	shader_HDRblur             *shader_HDR_blur;                 //HDR高光模糊
	shader_HDRfinal            *shader_HDR_final;                //HDR最终结果
	shader_particle            *particle_fire;                   //粒子系统着色器
	shader_grass               *shader_grass_billboard;          //草地公告板

	shader_basic *shader_light_deferred;
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
	
	void release();
};


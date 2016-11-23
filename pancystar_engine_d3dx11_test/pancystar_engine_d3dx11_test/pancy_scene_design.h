#pragma once
#include"geometry.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"engine_shadow.h"
#include"pancy_d3d11_basic.h"
#include"pancy_DXrenderstate.h"
#include"pancy_ssao.h"
#include"pancy_lighting.h"
#include"particle_system.h"
class shader_snakecompute : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *snakecontrol_input;      //shader�е�������Դ���
	ID3DX11EffectUnorderedAccessViewVariable *snakepoint_output;       //shader�е�������Դ���
	ID3DX11EffectUnorderedAccessViewVariable *snakecontrol_output;	   //compute_shader�������������Դ
	ID3DX11EffectMatrixVariable              *Bspline_matrix;          //b��������
	ID3DX11EffectVariable                    *snake_range;             //�����С
public:
	shader_snakecompute(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_input_buffer(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_piccturerange(int snake_body_num, int devide_num, int radius, int others);
	HRESULT set_output_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need);
	void release();
	void dispatch(int snake_num, int snake_devide);
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_snaketesselate : public shader_basic
{
	ID3DX11EffectMatrixVariable                    *final_mat;
public:
	shader_snaketesselate(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //�����ܱ任
	void release();
private:
	void init_handle();//ע��shader������ȫ�ֱ����ľ��
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};




struct point_snake
{
	XMFLOAT4 position;
	XMFLOAT4 center_position;
};
struct point_snake_control
{
	XMFLOAT3 position1;
	XMFLOAT3 position2;
	XMFLOAT3 position3;
	XMFLOAT3 position4;
};
class snake_draw
{
	ID3D11Device          *device_pancy;
	ID3D11DeviceContext   *contex_pancy;
	shader_snakecompute   *first_CSshader;
	shader_snaketesselate *second_TLshader;
	int max_snake_length;         //���峤������
	int snake_length;             //���峤��
	int snake_radius;           //����뾶
	int devide_num;               //ϸ������
	XMFLOAT3 snake_head_position; //��ͷλ��
	XMFLOAT3 snake_head_normal;   //��ͷ���߷���
	ID3D11UnorderedAccessView   *UAV_controlpoint_first;  //���Ƶ�Ļ�����1
	ID3D11ShaderResourceView    *SRV_controlpoint_first;  //���Ƶ�Ļ�����1
	ID3D11UnorderedAccessView   *UAV_controlpoint_second; //���Ƶ�Ļ�����2
	ID3D11ShaderResourceView    *SRV_controlpoint_second; //���Ƶ�Ļ�����2

	ID3D11Buffer                *index_buffer_render;
	ID3D11Buffer                *point_buffer_UAV;
	ID3D11Buffer                *CPU_read_buffer;
	ID3D11UnorderedAccessView   *UAV_draw_point_bufeer;   //���Ƶ�Ļ�����
public:
	snake_draw(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int max_length_need, int devide_num_need);
	HRESULT create();
	void draw(XMFLOAT4X4 view_projmat);
	void update();
	void release();
private:
	HRESULT build_controlbuffer();
	HRESULT build_render_buffer();
	HRESULT CreateCPUaccessBuf();
	void draw_pass(shader_control * shader_list);
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


class scene_root
{
protected:
	ID3D11Device              *device_pancy;     //d3d�豸
	ID3D11DeviceContext       *contex_pancy;     //�豸������
	geometry_control          *geometry_lib;     //��������Դ
	shader_control            *shader_lib;       //shader��Դ
	pancy_renderstate         *renderstate_lib;  //��Ⱦ��ʽ
	light_control             *light_list;       //��Դ
	pancy_input               *user_input;       //�����������
	pancy_camera              *scene_camera;     //���������
	//shadow_basic           *shadowmap_part;
	//ssao_pancy                *ssao_part;
	//d3d_pancy_basic        *engine_state;
	XMFLOAT4X4                view_matrix;
	XMFLOAT4X4                proj_matrix;
	int                       scene_window_width;
	int                       scene_window_height;
	float                     time_game;
	ID3D11ShaderResourceView  *gbuffer_normalspec;
	ID3D11ShaderResourceView  *gbuffer_depth;
	ID3D11ShaderResourceView  *lbuffer_diffuse;
	ID3D11ShaderResourceView  *lbuffer_specular;
	ID3D11ShaderResourceView  *environment_map;

public:
	scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need,int width,int height);
	virtual HRESULT scene_create() = 0;
	void get_gbuffer(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	void get_lbuffer(ID3D11ShaderResourceView *diffuse_need, ID3D11ShaderResourceView *specular_need);
	virtual HRESULT display() = 0;
	virtual HRESULT display_enviroment() = 0;
	virtual HRESULT display_nopost() = 0;
	virtual HRESULT update(float delta_time) = 0;
	virtual HRESULT release() = 0;
	void set_proj_matrix(XMFLOAT4X4 proj_mat_need);
	void reset_proj_matrix();
	void get_environment_map(ID3D11ShaderResourceView  *input) { environment_map = input; };
protected:
	virtual HRESULT camera_move();

};

class scene_engine_test : public scene_root
{
	snake_draw *test_snake;
	particle_system<fire_point>           *particle_fire;
public:
	scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height);
	HRESULT scene_create();
	HRESULT display();
	HRESULT display_nopost();
	HRESULT display_enviroment();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_ball();
	void show_floor();
	void show_aotestproj();
	void show_castel(LPCSTR techname, LPCSTR technamenormal);
	void show_castel_deffered(LPCSTR techname);
	void show_lightsource();
	void show_fire_particle();
	void show_yuri_animation();
	void show_yuri_animation_deffered();
	void show_billboard();
};
class scene_engine_snake : public scene_root
{
public:
	scene_engine_snake(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height);
	HRESULT scene_create();
	HRESULT display();
	HRESULT display_nopost();
	HRESULT display_enviroment();
	HRESULT update(float delta_time);
	HRESULT release();
private:
};

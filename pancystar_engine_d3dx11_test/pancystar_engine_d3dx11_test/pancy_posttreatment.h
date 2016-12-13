#pragma once
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"geometry.h"
#include"PancyCamera.h"
#include"pancy_model_import.h"
//HDR�����࣬����Ⱦ��ɺ�������Ļ�ռ�ͼ��
class render_posttreatment_HDR
{
	//ȫ���ı���
	ID3D11Buffer                *HDRMap_VB;            //aoͼƬ���㻺����
	ID3D11Buffer                *HDRMap_IB;            //aoͼƬ����������
													   //����ԭ��Ⱦ���ֵĹ������
	ID3D11Device                *device_pancy;
	ID3D11DeviceContext         *contex_pancy;
	ID3D11RenderTargetView      *rendertarget_input; //ǰһ������ɵ���ȾĿ��
	shader_control              *shader_list;        //shader��
	int                         width, height;       //��Ļ���
													 //�ڲ�����
	ID3D11UnorderedAccessView   *UAV_HDR_mid;        //HDR�Ļ������������м����
	ID3D11UnorderedAccessView   *UAV_HDR_final;      //HDR�Ļ����������ڴ洢���
	ID3D11ShaderResourceView    *SRV_HDR_map;        //HDR�Ļ����������ڴ洢map���
	ID3D11ShaderResourceView    *SRV_HDR_use;        //HDR���벿�֣�Ҫ����Ļ����ת���ɷǿ���ݵ�����

	ID3D11ShaderResourceView    *SRV_HDR_save;       //HDR�߹�洢������Ⱦ��Դ�����߹���д洢��
	ID3D11RenderTargetView      *RTV_HDR_save;       //HDR�߹�洢������ȾĿ�꣬���߹���д洢��

	ID3D11ShaderResourceView    *SRV_HDR_blur1;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur1;       //HDR�߹�ģ����ȾĿ�ꡣ
	ID3D11ShaderResourceView    *SRV_HDR_blur2;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur2;       //HDR�߹�ģ����ȾĿ�ꡣ

	D3D11_VIEWPORT              render_viewport;      //�ӿ���Ϣ
	ID3D11Buffer*               CPU_read_buffer;
	float                       average_light;
	float                       average_light_last;
	pancy_renderstate           *root_state_need;
	int width_rec, height_rec, buffer_num, map_num;
public:
	render_posttreatment_HDR(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, pancy_renderstate *rec_rootstate);
	HRESULT create();
	void release();
	HRESULT display();
private:
	HRESULT init_buffer();
	HRESULT init_texture();
	HRESULT CreateCPUaccessBuf(int size_need);
	HRESULT build_fullscreen_picturebuff();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	HRESULT count_average_light();
	HRESULT build_preblur_map();
	HRESULT blur_map();
	HRESULT HDR_map();

};
class render_posttreatment_SSR
{
	int                      map_width;
	int                      map_height;
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	pancy_renderstate        *renderstate_lib;
	geometry_control         *geometry_lib;
	pancy_camera             *camera_use;                 //�����
	ID3D11Buffer             *reflectMap_VB;              //aoͼƬ���㻺����
	ID3D11Buffer             *reflectMap_IB;              //aoͼƬ����������

	ID3D11ShaderResourceView *normaldepth_tex;            //�洢���ߺ���ȵ�������Դ
	ID3D11ShaderResourceView *depth_tex;                  //�洢���ߺ���ȵ�������Դ
	ID3D11ShaderResourceView *color_tex;                  //�洢��Ⱦ�����������Դ
	ID3D11ShaderResourceView *input_mask_tex;                  //�洢��Ⱦ�����������Դ

	ID3D11RenderTargetView   *reflect_target;             //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *reflect_tex;                //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *final_reflect_target;       //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *final_reflect_tex;          //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *blur_reflect_target;        //�洢��̬��Ļ�ռ䷴�����ȾĿ��
	ID3D11ShaderResourceView *blur_reflect_tex;           //�洢��̬��Ļ�ռ䷴���������Դ

	ID3D11RenderTargetView   *mask_target;                //�洢��̬��Ļ�ռ䷴���������ȾĿ��
	ID3D11ShaderResourceView *mask_tex;                   //�洢��̬��Ļ�ռ䷴�������������Դ

	ID3D11ShaderResourceView *reflect_cubestencil_SRV;    //�洢��̬cubemapping��������Դ
	ID3D11RenderTargetView   *reflect_cubestencil_RTV[6]; //�洢��̬cubemapping����ȾĿ��

	ID3D11ShaderResourceView *reflect_cube_SRV;           //�洢��̬cubemapping��������Դ
	ID3D11RenderTargetView   *reflect_cube_RTV[6];        //�洢��̬cubemapping����ȾĿ��

	ID3D11ShaderResourceView *reflect_cubeinput_SRV[6];   //�洢��̬cubemapping����������
	ID3D11RenderTargetView   *reflect_cubeinput_RTV[6];   //�洢��̬cubemapping������Ŀ��

	ID3D11DepthStencilView   *reflect_DSV[6];             //��Ȼ�����Ŀ��
	ID3D11ShaderResourceView *reflect_depthcube_SRV[6];      //���������ͼ

	XMFLOAT4                 FrustumFarCorner[4];         //ͶӰ�ӽ����Զ������ĸ��ǵ�
	D3D11_VIEWPORT           render_viewport;             //�ӿ���Ϣ
	D3D11_VIEWPORT           half_render_viewport;        //�ӿ���Ϣ

	XMFLOAT3                 center_position;
	XMFLOAT4X4               static_cube_view_matrix[6];  //������ͼ�����������ȡ���任
	shader_control           *shader_list;                //shader��

	float  width_static_cube;
public:
	render_posttreatment_SSR(pancy_camera *camera_need, pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_need, int width, int height, float fovy, float farZ);
	void set_normaldepthcolormap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	HRESULT create();
	void draw_reflect(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input);
	void draw_static_cube(int count_cube);
	void set_static_cube_rendertarget(int count_cube, XMFLOAT4X4 &mat_project);
	void set_static_cube_view_matrix(int count_cube, XMFLOAT4X4 mat_input);
	void set_static_cube_centerposition(XMFLOAT3 mat_input);
	XMFLOAT3 get_center_position() { return center_position; };
	ID3D11ShaderResourceView *get_cubemap() { return reflect_cube_SRV; };
	void release();
private:
	void set_size(int width, int height, float fovy, float farZ);
	void build_fullscreen_picturebuff();
	void BuildFrustumFarCorners(float fovy, float farZ);
	HRESULT build_texture();
	void build_reflect_map(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input);
	void blur_map();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void basic_blur(ID3D11ShaderResourceView *mask,ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void draw_to_posttarget();
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
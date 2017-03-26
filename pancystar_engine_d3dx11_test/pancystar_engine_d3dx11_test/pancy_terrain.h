#pragma once
#include<windows.h>
#include<string.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<directxmath.h>
#include<d3dx11effect.h>
#include<DDSTextureLoader.h>
#include<map>
#include"shader_pancy.h"
#include"geometry.h"
#include"pancy_physx.h"
//#include"pancy_model_import.h"
struct pancy_height_data 
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};
class pancy_terrain_build
{
	/*
	map_first_level_width = 2;
	map_second_level_width = 2;
	������һ����ͼ(���Ÿ߶�ͼ)
	ÿ��һ����ͼ�ֽ���ĸ�������ͼ
	����������������������������
	|      |      |      |       |
	|      |      |      |       |
	|������������ | ������������ |
	|      |      |      |       |
	|      |      |      |       |
	����������������������������
	|      |      |      |       |
	|      |      |      |       |
	|������������ | ������������ |
	|      |      |      |       |
	|      |      |      |       |
	����������������������������
	*/
	ID3D11Device             *device_pancy;           //d3d�豸
	ID3D11DeviceContext      *contex_pancy;           //�豸������
	pancy_physx              *physic_pancy;           //��������
	shader_control           *shader_list;
	pancy_height_data        *height_check_data;      //�߶ȼ�������
	ID3D11ShaderResourceView *height_check_buffer;    //�߶ȼ���buffer
	int                      map_first_level_width;   //һ����ͼ������(һ�Ÿ߶�ͼ����һ��һ����ͼ)
	int                      map_second_level_width;  //������ͼ������(��ÿ��һ����ͼ�ֽ�ɼ���������ͼ����ϸ��)
	//��ͼʵ�ʿ����Ϣ
	float                    height_terrain_scal;
	int                      depth_map_scal;          //�߶�ͼ��ƽƽ���ȼ�
	int                      sample_distance;         //�߶�ͼ�Ĳ�������
	float                    map_width_physics;       //һ����ͼ��������(��ͼ��С)
	int                      map_sample_width;        //�߶���������ؿ��
	int                      map_sample_height;       //�߶���������ظ߶�
	//��ͼ�Ĳ�����Ϣ
	ID3D11ShaderResourceView *diffuse_map_atrray;     //��������������
	ID3D11ShaderResourceView *height_map_atrray;      //�߶�ͼ��������
	ID3D11Buffer             *square_map_point;       //��ͼ���㻺����
	std::vector<LPCWSTR>     height_map_file_name;
	std::vector<LPCWSTR>     diffuse_map_file_name;
	physx::PxMaterial        *terrain_mat_force;
	physx::PxHeightFieldSample* samples;
	int numRows;
	int numCols;
public:
	pancy_terrain_build(pancy_physx* physic_need,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, shader_control *shader_list, int map_number, int map_detail, float map_range, std::vector<LPCWSTR> height_map, std::vector<LPCWSTR> diffuse_map);
	HRESULT create();
	void show_terrain(XMFLOAT4X4 viewproj_mat);
	ID3D11ShaderResourceView *get_heightmap() { return height_map_atrray; };
	ID3D11ShaderResourceView *get_diffusemap() { return diffuse_map_atrray; };
	void show_terrainshape(ID3DX11EffectTechnique* tech);
	HRESULT get_position_ID(int input_ID,float offset_range,XMFLOAT3 &map_position);
	HRESULT get_ID_position(XMFLOAT3 input_position, float offset_range,int &map_ID);
	void release();
private:
	HRESULT build_buffer();
	HRESULT build_texture();
	HRESULT build_physics();
	XMFLOAT3 vector_plus(XMFLOAT3 v1, XMFLOAT3 v2) { return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); };
	XMFLOAT3 vector_minus(XMFLOAT3 v1, XMFLOAT3 v2) { return XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); };
	XMFLOAT3 vector_mul_num(XMFLOAT3 v1, float num) { return XMFLOAT3(v1.x*num, v1.y *num, v1.z *num); };
};

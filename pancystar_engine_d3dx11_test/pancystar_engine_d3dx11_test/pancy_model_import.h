#pragma once
#include<windows.h>
#include"geometry.h"
#include <assimp/Importer.hpp>      // �������ڸ�ͷ�ļ��ж���
#include <assimp/scene.h>           // ��ȡ����ģ�����ݶ�����scene��
#include <assimp/postprocess.h>     // ��ͷ�ļ��а�������ı�־λ����
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
struct material_list
{
	char                       texture_diffuse[128];     //�����������ַ
	char                       texture_normal[128];      //������ͼ�����ַ
	ID3D11ShaderResourceView   *tex_diffuse_resource;    //����������
	ID3D11ShaderResourceView   *texture_normal_resource; //������ͼ����
	material_list()
	{
		texture_diffuse[0] = '\0';
		texture_normal[0] = '\0';
		tex_diffuse_resource = NULL;
		texture_normal_resource = NULL;
	}
};
struct mesh_list
{
	Geometry<point_with_tangent> *point_buffer;
	int material_use;
	mesh_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
class model_reader_assimp
{
protected:
	ID3D11Device           *device_pancy;     //d3d�豸
	ID3D11DeviceContext    *contex_pancy;     //�豸������
	ID3DX11EffectTechnique *teque_pancy;       //����·��
	std::string filename;        //ģ���ļ���
	char rec_texpath[128];       //����·��
	Assimp::Importer importer;   //ģ�͵�����
	const aiScene *model_need;   //ģ�ʹ洢��

	material_list *matlist_need; //���ʱ�
	mesh_list *mesh_need;        //�����
	int material_optimization;   //�Ż���Ĳ�������
	int mesh_optimization;       //�Ż������������
	Geometry<point_with_tangent> *mesh_scene;  //�洢�ϲ��ĳ�������
public:
	model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need,char* filename, char* texture_path);
	HRESULT model_create(bool if_optimize);
	int get_meshnum();
	void get_texture(material_list *texture_need, int i);
	void release();
	void draw_part(int i);
	void draw_mesh();
	HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
protected:
	virtual HRESULT init_mesh();
	HRESULT init_texture();
	void remove_texture_path(char rec[]);
	HRESULT combine_vertex_array();
	HRESULT optimization_mesh();//�����Ż�
};
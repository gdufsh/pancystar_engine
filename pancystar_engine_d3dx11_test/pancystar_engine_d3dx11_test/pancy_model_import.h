#pragma once
#include<windows.h>
#include"geometry.h"
#include"shader_pancy.h"
#include"pancy_terrain.h"
#include <assimp/Importer.hpp>      // �������ڸ�ͷ�ļ��ж���
#include <assimp/scene.h>           // ��ȡ����ģ�����ݶ�����scene��
#include <assimp/postprocess.h>     // ��ͷ�ļ��а�������ı�־λ����
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
#include<assimp/Exporter.hpp>
#include <map>
#include<string>
#define MAX_PLANT 300
enum model_data_type 
{
	pancy_model_buildin = 0,
	pancy_model_assimp = 1,
};
struct material_list
{
	char                       texture_diffuse[128];     //�����������ַ
	char                       texture_normal[128];      //������ͼ�����ַ
	char                       texture_specular[128];    //������ͼ�����ַ
	ID3D11ShaderResourceView   *tex_diffuse_resource;    //����������
	ID3D11ShaderResourceView   *texture_normal_resource; //������ͼ����
	ID3D11ShaderResourceView   *texture_specular_resource; //������ͼ����
	material_list()
	{
		texture_diffuse[0] = '\0';
		texture_normal[0] = '\0';
		texture_specular[0] = '\0';
		tex_diffuse_resource = NULL;
		texture_normal_resource = NULL;
		texture_specular_resource = NULL;
	}
};
struct meshview_list
{
	Geometry_basic *point_buffer;
	int material_use;
	meshview_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
template<typename T>
struct mesh_list
{
	Geometry<T> *point_buffer;
	//Geometry_basic *point_buffer;
	int material_use;
	mesh_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
class assimp_basic
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
	int material_optimization;   //�Ż���Ĳ�������
	int mesh_optimization;       //�Ż������������
	int mesh_normal_optimization;
	bool if_alpha_array[10000];    //�����ж��Ƿ���͸������
	bool if_normal_array[10000];    //�����ж��Ƿ��Ƿ�����ͼ����
public:
	assimp_basic(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	HRESULT model_create(bool if_adj, bool if_optimize, int alpha_partnum, int* alpha_part);
	int get_meshnum();
	int get_meshnormalnum();
	virtual void get_texture(material_list *texture_need, int i) = 0;
	virtual void get_normaltexture(material_list *texture_need, int i) = 0;
	virtual void release() = 0;
	virtual void draw_part(int i) = 0;
	virtual void draw_part_instance(int i, int copy_num) = 0;
	virtual void draw_normal_part(int i) = 0;
	virtual void draw_normal_part_instance(int i, int copy_num) = 0;
	virtual void draw_mesh() = 0;
	virtual void draw_mesh_adj() = 0;
	virtual void draw_mesh_instance(int copy_num) = 0;
	HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
	bool check_alpha(int part) { return if_alpha_array[part]; };
	bool check_normal(int part) { return if_normal_array[part]; };
protected:
	virtual HRESULT init_mesh(bool if_adj) = 0;
	HRESULT init_texture();
	void remove_texture_path(char rec[]);
	virtual HRESULT combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj) = 0;
	virtual HRESULT optimization_mesh(bool if_adj) = 0;//�����Ż�
	virtual HRESULT optimization_normalmesh(bool if_adj) = 0;//�����Ż�
	virtual HRESULT copy_Geometry_Resource() = 0;
	void change_texturedesc_2dds(char rec[]);
	void release_basic();
};
class modelview_basic_assimp : public assimp_basic
{
protected:
	meshview_list *mesh_need_view;        //�����
	meshview_list *mesh_need_normal_view; //�����Ż���������
	Geometry_basic *mesh_scene_view;  //�洢�ϲ��ĳ�������
public:
	modelview_basic_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	void release();
	void draw_part(int i);
	void draw_part_instance(int i, int copy_num);
	void draw_mesh();
	void draw_mesh_adj();
	void draw_mesh_instance(int copy_num);
	void draw_normal_part(int i);
	void draw_normal_part_instance(int i, int copy_num);
};
template<typename T>
class model_reader_assimp : public modelview_basic_assimp
{
protected:
	mesh_list<T> *mesh_need;        //�����
	mesh_list<T> *mesh_need_normal; //�����Ż���������
	Geometry<T> *mesh_scene;  //�洢�ϲ��ĳ�������
public:
	model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	void get_texture(material_list *texture_need, int i);
	void get_normaltexture(material_list *texture_need, int i);
    
	/*
	void release();
	void draw_part(int i);
	void draw_mesh();
	void draw_mesh_adj();
	void draw_normal_part(int i);
	*/
protected:
	virtual HRESULT init_mesh(bool if_adj);
	HRESULT combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj);
	HRESULT optimization_mesh(bool if_adj);//�����Ż�
	HRESULT optimization_normalmesh(bool if_adj);//�����Ż�
	HRESULT copy_Geometry_Resource();
};
template<typename T>
model_reader_assimp<T>::model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path) : modelview_basic_assimp(device_need, contex_need, pFile, texture_path)
{
	mesh_need = NULL;
	mesh_scene = NULL;
}
template<typename T>
HRESULT model_reader_assimp<T>::init_mesh(bool if_adj)
{
	T *point_need;
	unsigned int *index_need;
	//���������¼��
	mesh_need = new mesh_list<T>[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//��ȡģ�͵ĵ�i��ģ��
		const aiMesh* paiMesh = model_need->mMeshes[i];
		//��ȡģ�͵Ĳ��ʱ��
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		//ģ�͵ĵ�i��ģ��Ķ��㼰������Ϣ
		mesh_need[i].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, paiMesh->mNumVertices, paiMesh->mNumFaces * 3);
		point_need = (T*)malloc(paiMesh->mNumVertices * sizeof(T));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
		//���㻺��
		for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
		{
			point_need[j].position.x = paiMesh->mVertices[j].x;
			point_need[j].position.y = paiMesh->mVertices[j].y;
			point_need[j].position.z = paiMesh->mVertices[j].z;

			point_need[j].normal.x = paiMesh->mNormals[j].x;
			point_need[j].normal.y = paiMesh->mNormals[j].y;
			point_need[j].normal.z = paiMesh->mNormals[j].z;

			if (paiMesh->HasTextureCoords(0))
			{
				point_need[j].tex.x = paiMesh->mTextureCoords[0][j].x;
				point_need[j].tex.y = 1 - paiMesh->mTextureCoords[0][j].y;
			}
			else
			{
				point_need[j].tex.x = 0.0f;
				point_need[j].tex.y = 0.0f;
			}
			if (paiMesh->mTangents != NULL)
			{
				point_need[j].tangent.x = paiMesh->mTangents[j].x;
				point_need[j].tangent.y = paiMesh->mTangents[j].y;
				point_need[j].tangent.z = paiMesh->mTangents[j].z;
			}
			else
			{
				point_need[j].tangent.x = 0.0f;
				point_need[j].tangent.y = 0.0f;
				point_need[j].tangent.z = 0.0f;
			}
		}
		//����������
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[j].mNumIndices == 3)
			{
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[0];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[1];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[2];
			}
			else
			{
				return E_FAIL;
			}
		}
		//�����ڴ���Ϣ�����Դ���
		HRESULT hr = mesh_need[i].point_buffer->create_object(point_need, index_need, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"create model mesh error", L"tip", MB_OK);
			return hr;
		}
		//�ͷ��ڴ�
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}
	return S_OK;
}
template<typename T>
HRESULT model_reader_assimp<T>::combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj)
{
	T *vertex_out;
	UINT *index_out;
	int all_vertex = 0, all_index = 0;
	Geometry<T> *now_meshneed[700];
	for (int i = 0; i < mesh_optimization; ++i)
	{
		if (if_alpha_array[i] == false)
		{
			now_meshneed[i] = mesh_need[i].point_buffer;
		}
		else
		{
			int a = 0;
		}
	}
	for (int i = 0; i < mesh_optimization; ++i)
	{
		if (if_alpha_array[i] == false)
		{
			int rec_vertex, rec_index;
			now_meshneed[i]->get_point_num(rec_vertex, rec_index);
			all_vertex += rec_vertex;
			all_index += rec_index;
		}
	}
	vertex_out = (T*)malloc(all_vertex * sizeof(T) + 100);
	index_out = (UINT*)malloc(all_index * sizeof(UINT) + 100);
	//�Լ�������кϲ�����
	T *now_pointer = vertex_out;
	UINT *now_index_pointer = index_out;
	int vertex_num = 0;
	for (int i = 0; i < mesh_optimization; ++i)
	{
		material_list check_need;
		get_texture(&check_need, i);
		if (if_alpha_array[i] == false)
		{
			T *vertex_rec;
			UINT *index_rec;
			int all_vertex = 0, all_index = 0;
			//��ȡ������Ķ�����������Ŀ
			now_meshneed[i]->get_point_num(all_vertex, all_index);
			vertex_rec = (T*)malloc(all_vertex * sizeof(T) + 100);
			index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);
			//��ȡ������Ķ�������������
			now_meshneed[i]->get_bufferdata(vertex_rec, index_rec);
			for (int j = 0; j < all_vertex; ++j)
			{
				vertex_rec[j].tex_id.x = mesh_need[i].material_use - 1;
				vertex_rec[j].tex_id.y = mesh_need[i].material_use - 1;
			}
			//��������
			memcpy(now_pointer, vertex_rec, all_vertex * sizeof(T));
			//�޸�������
			for (int j = 0; j < all_index; ++j)
			{
				index_rec[j] += vertex_num;
			}
			//��������
			memcpy(now_index_pointer, index_rec, all_index * sizeof(UINT));
			//���¼�¼
			now_pointer += all_vertex;
			vertex_num += all_vertex;
			now_index_pointer += all_index;
			free(vertex_rec);
			free(index_rec);
		}
		else
		{
			if_normal_array[i] = true;
		}
	}
	mesh_scene = new mesh_comman<T>(device_pancy, contex_pancy, all_vertex, all_index);
	HRESULT hr = mesh_scene->create_object(vertex_out, index_out, if_adj);
	if (FAILED(hr))
	{
		MessageBox(0, L"combine scene error", L"tip", MB_OK);
		return hr;
	}

	free(vertex_out);
	free(index_out);
	return S_OK;
}
template<typename T>
void model_reader_assimp<T>::get_texture(material_list *texture_need, int i)
{
	*texture_need = matlist_need[mesh_need[i].material_use];
}
template<typename T>
void model_reader_assimp<T>::get_normaltexture(material_list *texture_need, int i)
{
	if (mesh_need_normal[i].material_use == 0)
	{
		texture_need->texture_normal_resource = NULL;
	}
	*texture_need = matlist_need[mesh_need_normal[i].material_use];
}
template<typename T>
HRESULT model_reader_assimp<T>::optimization_mesh(bool if_adj)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�Ⱥϲ�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int hash_UnionFind[1000];
	memset(hash_UnionFind, 0, 1000 * sizeof(int));
	int now_different = 1;//��ǰ�Ѿ��еĻ�����ͬ�Ĳ�������
	hash_UnionFind[0] = 0;
	/*
	material_list rec_need[1000];
	memcpy(rec_need, matlist_need,300*sizeof(material_list));
	*/
	for (int i = 1; i < model_need->mNumMaterials; ++i)
	{
		bool if_change = true;
		for (int j = 0; j < i; ++j)
		{
			if (strcmp(matlist_need[i].texture_diffuse, matlist_need[j].texture_diffuse) == 0 && strcmp(matlist_need[i].texture_normal, matlist_need[j].texture_normal) == 0)
			{
				//��ǰ������֮ǰ�Ĳ����ظ���ɾ��֮�����hash
				if (matlist_need[i].tex_diffuse_resource != NULL)
				{
					matlist_need[i].tex_diffuse_resource->Release();
					matlist_need[i].tex_diffuse_resource = NULL;
					matlist_need[i].texture_diffuse[0] = '\0';
				}
				if (matlist_need[i].texture_normal_resource != NULL)
				{
					matlist_need[i].texture_normal_resource->Release();
					matlist_need[i].texture_normal_resource = NULL;
					matlist_need[i].texture_normal[0] = '\0';
				}
				hash_UnionFind[i] = j;
				if_change = false;
				break;
			}
		}
		if (if_change == true)
		{
			//���һ����ͬ�Ĳ���
			matlist_need[now_different] = matlist_need[i];
			hash_UnionFind[i] = now_different;
			now_different += 1;
		}
	}
	material_optimization = now_different;
	mesh_optimization = now_different;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�ϲ�������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int all_vertex = 0, all_index = 0;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//����������
		mesh_need[i].material_use = hash_UnionFind[mesh_need[i].material_use];
		//�����ܶ�������
		int rec_vertex, rec_index;
		mesh_need[i].point_buffer->get_point_num(rec_vertex, rec_index);
		all_vertex += rec_vertex;
		all_index += rec_index;
	}
	//������ʱ�洢�ռ�
	T *vertex_rec;
	UINT *index_rec;
	vertex_rec = (T*)malloc(all_vertex * sizeof(T) + 100);
	index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);

	//������ʱ�洢�������ָ��
	mesh_list<T> *rec_optisave;
	rec_optisave = new mesh_list<T>[mesh_optimization + 10];

	int final_part_count = 0;
	//Ϊ�µļ�������֯�������������
	for (int i = 0; i < mesh_optimization; ++i)
	{
		T *now_pointer = vertex_rec;
		UINT *now_index_pointer = index_rec;
		int vertex_num = 0;
		int now_count_vertex = 0;
		int now_count_index = 0;
		for (int j = 0; j < model_need->mNumMeshes; j++)
		{
			//�ҵ�����ʹ�ò�����ͬ�����񣬲������Ǻϲ�
			if (mesh_need[j].material_use == i)
			{
				T *vertex_rec;
				UINT *index_rec;
				int check_vertex = 0, check_index = 0;
				//��ȡ������Ķ�����������Ŀ
				mesh_need[j].point_buffer->get_point_num(check_vertex, check_index);
				now_count_vertex += check_vertex;
				now_count_index += check_index;
				vertex_rec = (T*)malloc(check_vertex * sizeof(T) + 100);
				index_rec = (UINT*)malloc(check_index * sizeof(UINT) + 100);
				//��ȡ������Ķ�������������
				mesh_need[j].point_buffer->get_bufferdata(vertex_rec, index_rec);
				//��������
				memcpy(now_pointer, vertex_rec, check_vertex * sizeof(T));
				//�޸�������
				for (int k = 0; k < check_index; ++k)
				{
					index_rec[k] += vertex_num;
				}
				//��������
				memcpy(now_index_pointer, index_rec, check_index * sizeof(UINT));
				//���¼�¼
				now_pointer += check_vertex;
				vertex_num += check_vertex;
				now_index_pointer += check_index;
				free(vertex_rec);
				free(index_rec);
			}
		}
		if (now_count_vertex == 0 || now_count_index == 0) 
		{
			continue;
		}
		rec_optisave[final_part_count].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, now_count_vertex, now_count_index);
		rec_optisave[final_part_count].material_use = i;
		HRESULT hr = rec_optisave[final_part_count].point_buffer->create_object(vertex_rec, index_rec, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"combine scene error", L"tip", MB_OK);
			return hr;
		}
		final_part_count += 1;
	}
	mesh_optimization = final_part_count;
	free(vertex_rec);
	free(index_rec);
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		mesh_need[i].point_buffer->release();
	}
	for (int i = 0; i < material_optimization; ++i)
	{
		mesh_need[i] = rec_optisave[i];
	}
	return S_OK;
}
template<typename T>
HRESULT model_reader_assimp<T>::optimization_normalmesh(bool if_adj)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�Ⱥϲ�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int matlist_save[1000];
	memset(matlist_save, 0, 1000 * sizeof(int));
	int rec_meshtex_save[1000];
	memset(rec_meshtex_save, 0, 1000 * sizeof(int));
	//matlist_save[0] = mesh_need[0].material_use;
	mesh_normal_optimization = 0;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�ϲ�������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int all_vertex = 0, all_index = 0;
	for (int i = 0; i < mesh_optimization; i++)
	{
		bool if_change = true;
		material_list rec_now;
		get_texture(&rec_now, i);
		//�����ܶ�������
		int rec_vertex, rec_index;
		mesh_need[i].point_buffer->get_point_num(rec_vertex, rec_index);
		all_vertex += rec_vertex;
		all_index += rec_index;
		for (int j = 0; j < mesh_normal_optimization; ++j)
		{
			if (rec_now.texture_normal_resource != NULL && strcmp(rec_now.texture_normal, matlist_need[matlist_save[j]].texture_normal) == 0)
			{
				//����������
				rec_meshtex_save[i] = matlist_save[j];
				if_change = false;
				break;
			}
		}
		if (rec_now.texture_normal_resource != NULL && if_change == true && if_alpha_array[i] == false)
		{
			matlist_save[mesh_normal_optimization++] = mesh_need[i].material_use;
			rec_meshtex_save[i] = mesh_need[i].material_use;
		}
	}
	/*
	for (int i = 0; i < mesh_optimization; i++)
	{
		if (rec_meshtex_save[i] == 0 && if_alpha_array[i] == false)
		{
			//matlist_save[mesh_normal_optimization++] = 0;
			break;
		}
	}
	*/
	//������ʱ�洢�ռ�
	T *vertex_rec;
	UINT *index_rec;
	vertex_rec = (T*)malloc(all_vertex * sizeof(T) + 100);
	index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);

	//������ʱ�洢�������ָ��
	mesh_need_normal = new mesh_list<T>[mesh_normal_optimization + 10];
	//Ϊ�µļ�������֯�������������
	for (int i = 0; i < mesh_normal_optimization; ++i)
	{
		T *now_pointer = vertex_rec;
		UINT *now_index_pointer = index_rec;
		int vertex_num = 0;
		int now_count_vertex = 0;
		int now_count_index = 0;
		for (int j = 0; j < mesh_optimization; j++)
		{
			material_list rec_now;
			get_texture(&rec_now, i);
			//�ҵ�����ʹ�ò�����ͬ�����񣬲������Ǻϲ�
			if (rec_meshtex_save[j] == matlist_save[i])
			{
				T *vertex_rec;
				UINT *index_rec;
				int check_vertex = 0, check_index = 0;
				//��ȡ������Ķ�����������Ŀ
				mesh_need[j].point_buffer->get_point_num(check_vertex, check_index);
				now_count_vertex += check_vertex;
				now_count_index += check_index;
				vertex_rec = (T*)malloc(check_vertex * sizeof(T) + 100);
				index_rec = (UINT*)malloc(check_index * sizeof(UINT) + 100);
				//��ȡ������Ķ�������������
				mesh_need[j].point_buffer->get_bufferdata(vertex_rec, index_rec);
				//��������
				memcpy(now_pointer, vertex_rec, check_vertex * sizeof(T));
				//�޸�������
				for (int k = 0; k < check_index; ++k)
				{
					index_rec[k] += vertex_num;
				}
				//��������
				memcpy(now_index_pointer, index_rec, check_index * sizeof(UINT));
				//���¼�¼
				now_pointer += check_vertex;
				vertex_num += check_vertex;
				now_index_pointer += check_index;
				free(vertex_rec);
				free(index_rec);
			}
		}
		mesh_need_normal[i].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, now_count_vertex, now_count_index);
		mesh_need_normal[i].material_use = matlist_save[i];
		HRESULT hr = mesh_need_normal[i].point_buffer->create_object(vertex_rec, index_rec, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"combine scene error", L"tip", MB_OK);
			return hr;
		}
	}
	free(vertex_rec);
	free(index_rec);
	return S_OK;
}
template<typename T>
HRESULT model_reader_assimp<T>::copy_Geometry_Resource() 
{
	mesh_need_view = new meshview_list[mesh_optimization];
	for (int i = 0; i < mesh_optimization; i++)
	{
		mesh_need_view[i].point_buffer = mesh_need[i].point_buffer;
		mesh_need_view[i].material_use = mesh_need[i].material_use;
	}
	mesh_need_normal_view = new meshview_list[mesh_normal_optimization];
	for (int i = 0; i < mesh_normal_optimization; i++)
	{
		mesh_need_normal_view[i].point_buffer = mesh_need_normal[i].point_buffer;
		mesh_need_normal_view[i].material_use = mesh_need_normal[i].material_use;
	}
	mesh_scene_view = mesh_scene;
	return S_OK;
}
/*
template<typename T>
void model_reader_assimp<T>::release()
{
	//�ͷ�������Դ
	if (model_need != NULL)
	{
		for (int i = 0; i < material_optimization; ++i)
		{
			if (matlist_need[i].tex_diffuse_resource != NULL)
			{
				matlist_need[i].tex_diffuse_resource->Release();
				matlist_need[i].tex_diffuse_resource = NULL;
			}
			if (matlist_need[i].texture_normal_resource != NULL)
			{
				matlist_need[i].texture_normal_resource->Release();
				matlist_need[i].texture_normal_resource = NULL;
			}
		}
		//release_basic();
		//�ͷŻ�������Դ
		for (int i = 0; i < mesh_optimization; i++)
		{
			mesh_need[i].point_buffer->release();
		}
		for (int i = 0; i < mesh_normal_optimization; i++)
		{
			mesh_need_normal[i].point_buffer->release();
		}
		mesh_scene->release();
		//�ͷű���Դ
		delete[] mesh_need;
		//�ͷű���Դ
		delete[] matlist_need;
		model_need->~aiScene();
	}
}
template<typename T>
void model_reader_assimp<T>::draw_part(int i)
{
	mesh_need[i].point_buffer->get_teque(teque_pancy);
	mesh_need[i].point_buffer->show_mesh();
}
template<typename T>
void model_reader_assimp<T>::draw_normal_part(int i)
{
	mesh_need_normal[i].point_buffer->get_teque(teque_pancy);
	mesh_need_normal[i].point_buffer->show_mesh();
}
template<typename T>
void model_reader_assimp<T>::draw_mesh()
{
	mesh_scene->get_teque(teque_pancy);
	mesh_scene->show_mesh();
}
template<typename T>
void model_reader_assimp<T>::draw_mesh_adj()
{
	mesh_scene->get_teque(teque_pancy);
	mesh_scene->show_mesh_adj();
}
*/
struct skin_tree
{
	char bone_ID[128];
	int bone_number;
	XMFLOAT4X4 basic_matrix;
	XMFLOAT4X4 animation_matrix;
	XMFLOAT4X4 now_matrix;
	skin_tree *brother;
	skin_tree *son;
	skin_tree()
	{
		bone_ID[0] = '\0';
		bone_number = -1;
		brother = NULL;
		son = NULL;
		XMStoreFloat4x4(&basic_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&animation_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&now_matrix, XMMatrixIdentity());
	}
};
//�任����
struct vector_animation
{
	float time;               //֡ʱ��
	float main_key[3];        //֡����
};
//�任��Ԫ��
struct quaternion_animation
{
	float time;               //֡ʱ��
	float main_key[4];        //֡����
};
//�任����
struct matrix_animation
{
	float time;               //֡ʱ��
	float main_key[16];       //֡����
};
struct animation_data
{
	char bone_name[128];                                //���α任���ݶ�Ӧ�Ĺ�������
	skin_tree *bone_point;                              //���α任���ݶ�Ӧ�Ĺ�����ָ��

	DWORD number_translation;                           //ƽ�Ʊ任������
	vector_animation *translation_key;                  //����ƽ�Ʊ任����

	DWORD number_scaling;                               //�����任������
	vector_animation *scaling_key;                      //���������任����

	DWORD number_rotation;                              //��ת�任������
	quaternion_animation *rotation_key;                 //������ת�任������

	DWORD number_transform;                             //��ϱ任������
	matrix_animation *transform_key;                    //������ϱ任������	

	struct animation_data *next;                        //��һ���任����
};
struct animation_set
{
	char  animation_name[128];                          //�ö���������
	float animation_length;                             //�����ĳ���
	DWORD number_animation;                             //���������ı任����
	animation_data *head_animition;                     //�ö���������
	animation_set *next;                                //ָ����һ��������ָ��
};
class skin_mesh : public model_reader_assimp<point_withskin>
{
	skin_tree *root_skin;
	animation_set *first_animation;
	float time_all;
	int bone_num;
	XMFLOAT4X4 bone_matrix_array[100];
	XMFLOAT4X4 offset_matrix_array[100];
	XMFLOAT4X4 final_matrix_array[100];
	int tree_node_num[100][100];
public:
	skin_mesh(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	void update_root(skin_tree *root, XMFLOAT4X4 matrix_parent);
	void update_mesh_offset(int i);
	void update_mesh_offset();
	void update_animation(float delta_time);
	void specify_animation_time(float animation_time);
	XMFLOAT4X4* get_bone_matrix(int i, int &num_bone);
	XMFLOAT4X4* get_bone_matrix();
	float get_animation_length() { return first_animation->animation_length; };
	int get_bone_num() { return bone_num; };
	void release_all();
private:
	HRESULT init_mesh(bool if_adj);
	aiNode *find_skinroot(aiNode *now_node, char root_name[]);
	HRESULT build_skintree(aiNode *now_node, skin_tree *now_root);
	HRESULT build_animation_list();
	bool check_ifsame(char a[], char b[]);
	void set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in);
	skin_tree* find_tree(skin_tree* p, char name[]);
	skin_tree* find_tree(skin_tree* p, int num);
	void free_tree(skin_tree *now);
	void update_anim_data(animation_data *now);
	void find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation);
	void find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation);
	void Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor);
	void Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor);
	void Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut);
	int find_min(float x1, float x2, float x3, float x4);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�ⲿ����ģ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct model_resource_data 
{
	bool if_skin;
	std::string resource_name;
	int resource_index;
	modelview_basic_assimp* data;
};
class assimpmodel_resource_view
{
	float start_time;
	float end_time;
	float now_time;
	float animation_speed;
	std::string geometry_name;
	int indexnum_geometry;
	assimp_basic *model_data;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 *bone_matrix;
	bool if_skinmesh;
	int bone_num;
public:
	assimpmodel_resource_view(assimp_basic *model_input, bool if_skin, std::string name_need);
	void draw_full_geometry(ID3DX11EffectTechnique *tech_common);
	void draw_full_geometry_adj(ID3DX11EffectTechnique *tech_common);
	void draw_mesh_part(ID3DX11EffectTechnique *tech_transparent, int transparent_part);
	void draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part);
	bool check_if_skin() { return if_skinmesh; };
	//todo��ʹ��˯�߻��������涯��ʱ������
	void reset_animation_time(float time);
	float get_animation_time() { return now_time; };
	XMFLOAT4X4 get_world_matrix() { return world_matrix; };
	XMFLOAT4X4* get_bone_matrix() { return if_skinmesh ? bone_matrix : NULL; };
	int get_bone_num() { return bone_num; };
	std::string get_geometry_name() { return geometry_name; };
	int get_geometry_index() { return indexnum_geometry; };
	void set_geometry_index(int index_num) { indexnum_geometry = index_num; };
	int get_geometry_num() { return model_data->get_meshnum(); };
	int get_geometry_normal_num() { return model_data->get_meshnormalnum(); };
	void release();
	void update(XMFLOAT4X4 world_matrix_need, float delta_time);
	void get_texture(material_list *texture_need, int i) { model_data->get_texture(texture_need, i); };
	void get_normaltexture(material_list *texture_need, int i) { model_data->get_normaltexture(texture_need, i); };
	bool check_alpha(int part) { return model_data->check_alpha(part); };
	void reset_animation_data(float animation_start_need, float animation_end_need, float animation_speed_need);
	//assimp_basic *get_geometry_data() { return model_data; };
private:
	void reset_world_matrix(XMFLOAT4X4 world_matrix_need) { world_matrix = world_matrix_need; };
	void reset_bone_matrix(XMFLOAT4X4 *bone_matrix_need, int bone_num_need) { bone_matrix = bone_matrix_need; bone_num = bone_num_need; };
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���ü�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct BuiltIngeometry_resource_data 
{
	std::string resource_name;
	int resource_index;
	Geometry_basic *data;
};
class buildin_geometry_resource_view 
{
	Geometry_type mesh_type;           //����������
	std::string geometry_name;         //����������
	int indexnum_geometry;             //����������
	Geometry_basic *model_data;        //����������
	XMFLOAT4X4 world_matrix;           //������任����
	material_list *texture_need;       //�������ѧ����
	pancy_physx  *physic_pancy;        //�������浥��
	physx::PxRigidDynamic *physic_body;//����������ģ��
	physx::PxMaterial *mat_force;      //�������������
public:
	HRESULT update_physx_worldmatrix(float delta_time);
	buildin_geometry_resource_view(Geometry_basic *model_input, std::string name_need, Geometry_type type_need, pancy_physx  *physic_need);
	HRESULT create_physics();
	void draw_full_geometry(ID3DX11EffectTechnique *tech_common);
	XMFLOAT4X4 get_world_matrix() { return world_matrix; };
	void update(XMFLOAT4X4 world_matrix_need, float delta_time);
	std::string get_geometry_name() { return geometry_name; };
	int get_geometry_index() { return indexnum_geometry; };
	void set_geometry_index(int index_num) { indexnum_geometry = index_num; };
	void release();
private:
	void reset_world_matrix(XMFLOAT4X4 world_matrix_need) { world_matrix = world_matrix_need; };
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ֲ��ģ��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct plant_instance_data 
{
	int instance_index;
	XMFLOAT4X4 world_matrix;
};
class plant_resource_view
{
	std::string geometry_name;           //����������
	int indexnum_geometry;               //����������
	assimp_basic *model_data;             //����������
	std::map<int,plant_instance_data> view_data;  //�������������
	material_list *texture_need;          //�������ѧ����
public:  
	plant_resource_view(assimp_basic *model_input, std::string name_need);
	HRESULT add_a_instance(int map_position,XMFLOAT4X4 world_matrix);
	void clear_instance();
	void draw_full_geometry(ID3DX11EffectTechnique *tech_common);
	void draw_mesh_part(ID3DX11EffectTechnique *tech_transparent, int transparent_part);
	void draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part);
	void get_texture(material_list *texture_need, int i) { model_data->get_texture(texture_need, i); };
	void get_normaltexture(material_list *texture_need, int i) { model_data->get_normaltexture(texture_need, i); };
	int get_geometry_num() { return model_data->get_meshnum(); };
	int get_instance_num() { return view_data.size(); };
	int get_geometry_normal_num() { return model_data->get_meshnormalnum(); };
	void get_world_matrix_array(int &mat_num, XMFLOAT4X4 *mat);
	void update(float delta_time);
	std::string get_geometry_name() { return geometry_name; };
	int get_geometry_index() { return indexnum_geometry; };
	void set_geometry_index(int index_num) { indexnum_geometry = index_num; };
	void release();
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class texture_data 
{
	ID3D11ShaderResourceView *data;
public:
	texture_data() { data = NULL; };
	ID3D11ShaderResourceView *get_data() { return data; };
	void set_data(ID3D11ShaderResourceView *data_input) { data = data_input; };
	void release() { data->Release(); };
};
struct texture_pack
{
	std::string resource_name;
	int resource_index;
	texture_data *data;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~��������ṹ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<typename T>
class pancy_resource_list
{
	std::vector<T> model_resource_list;
public:
	pancy_resource_list();
	T* get_resource_by_name(std::string name_input);
	T* get_resource_by_index(int index_input);
	int get_geometry_num() { return model_resource_list.size(); };
	int add_resource(T data_input);
	void release();
};
template<typename T>
pancy_resource_list<T>::pancy_resource_list()
{
}
template<typename T>
T* pancy_resource_list<T>::get_resource_by_name(std::string name_input)
{
	for (auto data_GRV = model_resource_list.begin(); data_GRV != model_resource_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->resource_name;
		if (name == name_input)
		{
			return data_GRV._Ptr;
		}
	}
	return NULL;
}
template<typename T>
T* pancy_resource_list<T>::get_resource_by_index(int index_input)
{
	if (index_input > model_resource_list.size())
	{
		return NULL;
	}
	auto data_GRV = model_resource_list.begin() + index_input;
	return data_GRV._Ptr;
}
template<typename T>
int pancy_resource_list<T>::add_resource(T data_input)
{
	data_input.resource_index = model_resource_list.size();
	model_resource_list.push_back(data_input);
	return data_input.resource_index;
}
template<typename T>
void pancy_resource_list<T>::release()
{
	for (auto data_need = model_resource_list.begin(); data_need != model_resource_list.end(); ++data_need)
	{
		data_need._Ptr->data->release();
	}
	model_resource_list.clear();
}

template<typename T>
class geometry_ResourceView_list
{
protected:
	std::vector<T> ModelResourceView_list;
public:
	geometry_ResourceView_list();
	void add_new_geometry(T data_input);
	void delete_geometry_byname(std::string name_input);
	T *get_geometry_byname(std::string name_input);
	void delete_geometry_byindex(int index_input);
	T *get_geometry_byindex(int index_input);
	int get_geometry_num() { return ModelResourceView_list.size(); };
	void release();
};
template<typename T>
geometry_ResourceView_list<T>::geometry_ResourceView_list()
{
	/*
	head = NULL;
	tail = NULL;
	number_list = 0;
	*/
}
template<typename T>
void geometry_ResourceView_list<T>::add_new_geometry(T data_input)
{
	data_input.set_geometry_index(ModelResourceView_list.size());
	ModelResourceView_list.push_back(data_input);
	/*
	if (tail == NULL)
	{
	head = data_input;
	tail = data_input;
	tail->set_next_member(NULL);
	tail->set_pre_member(NULL);
	}
	else
	{
	data_input->set_pre_member(tail);
	tail->set_next_member(data_input);
	tail = tail->get_next_member();
	}
	number_list += 1;
	*/
}
template<typename T>
void geometry_ResourceView_list<T>::delete_geometry_byname(std::string name_input)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_input)
		{
			ModelResourceView_list.erase(data_GRV);
			break;
		}
	}
	/*
	assimpmodel_resource_view *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
	std::string name = find_ptr->get_geometry_name();
	if (name == name_input)
	{
	if (find_ptr->get_last_member() == NULL)
	{
	if (find_ptr->get_next_member() == NULL)
	{
	head = NULL;
	tail = NULL;
	delete find_ptr;
	number_list -= 1;
	}
	else
	{
	head = find_ptr->get_next_member();
	head->set_pre_member(NULL);
	delete find_ptr;
	number_list -= 1;
	}
	}
	else
	{
	if (find_ptr->get_next_member() == NULL)
	{
	tail = find_ptr->get_last_member();
	tail->set_next_member(NULL);
	delete find_ptr;
	number_list -= 1;
	}
	else
	{
	find_ptr->get_last_member()->set_next_member(find_ptr->get_next_member());
	find_ptr->get_next_member()->set_pre_member(find_ptr->get_last_member());
	delete find_ptr;
	number_list -= 1;
	}
	}
	break;
	}
	find_ptr = find_ptr->get_next_member();
	}*/
}
template<typename T>
T *geometry_ResourceView_list<T>::get_geometry_byname(std::string name_input)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_input)
		{
			return data_GRV._Ptr;
		}
	}
	/*
	assimpmodel_resource_view *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
	std::string name = find_ptr->get_geometry_name();
	if (name == name_input)
	{
	return find_ptr;
	}
	find_ptr = find_ptr->get_next_member();
	}
	*/
	return NULL;
}
template<typename T>
void geometry_ResourceView_list<T>::delete_geometry_byindex(int index_input)
{
	if (index_input > ModelResourceView_list.size())
	{
		return;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_input;
	ModelResourceView_list.erase(data_GRV);
}
template<typename T>
T* geometry_ResourceView_list<T>::get_geometry_byindex(int index_input)
{
	if (index_input > ModelResourceView_list.size())
	{
		return NULL;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_input;
	return data_GRV._Ptr;
}
template<typename T>
void geometry_ResourceView_list<T>::release()
{
	for (auto data_need = ModelResourceView_list.begin(); data_need != ModelResourceView_list.end(); ++data_need)
	{
		data_need._Ptr->release();
	}
	ModelResourceView_list.clear();
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		ModelResourceView_list.erase(data_GRV);
	}
	/*
	assimpmodel_resource_view *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
	assimpmodel_resource_view *now_rec = find_ptr;
	find_ptr = find_ptr->get_next_member();
	now_rec->release();
	delete now_rec;
	}
	*/
}

class assimp_ResourceView_list : public geometry_ResourceView_list<assimpmodel_resource_view>
{
public:
	void update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time);
	void update_geometry_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time);
};
class buildin_ResourceView_list : public geometry_ResourceView_list<buildin_geometry_resource_view>
{
public:
	void update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time);
	void update_geometry_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time);
};
class plant_ResourceView_list : public geometry_ResourceView_list<plant_resource_view> 
{
public:
	void update_geometry_byname(std::string name_input, float delta_time);
	void update_geometry_byindex(int index_input, float delta_time);
	HRESULT add_instance_by_name(std::string name_resource_view,int map_location,XMFLOAT4X4 mat_translation);
	HRESULT add_instance_by_idnex(int index_resource_view, int map_location, XMFLOAT4X4 mat_translation);
};
/*
template<typename T>
class special_assimpmodel_resource_view
{
	std::string geometry_name;
	int indexnum_geometry;
	Geometry<T> *mesh_data;
	XMFLOAT4X4 world_matrix;
	shader_basic *draw_shader;
};*/
class geometry_control
{
	ID3D11Device        *device_pancy;     //d3d�豸
	ID3D11DeviceContext *contex_pancy;     //�豸������
	pancy_physx         *physic_pancy;     //��������
	//assimp���ʱ���Դ��
	pancy_resource_list<model_resource_data> *list_model_resource;                     //assimpģ����Դ
	assimp_ResourceView_list *list_model_assimp;             //assimpģ�ͷ��ʱ�
	//���ü�����
	pancy_resource_list<BuiltIngeometry_resource_data> *list_buildin_geometry_resource;//���ü�������Դ��
	buildin_ResourceView_list *list_buildin_model_view;   //���ü�������ʱ�
	//ֲ��������
	plant_ResourceView_list *list_plant_model_view;
	bool if_have_terrain;
	pancy_terrain_build *terrain_tesselation;
	pancy_resource_list<texture_pack> *list_texture_use;

	mesh_billboard                *grass_billboard;     //�����
public:
	geometry_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_physx *physic_need);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~assimpģ�͵ĵ����������������洢��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT load_modelresource_from_file(char* filename,char* texture_path,bool if_animation, bool if_optimized, bool if_create_adj, int alpha_part_num,int*alpha_part_index,std::string resource_name,int &index_output);
	int get_assimp_model_view_num() { return list_model_assimp->get_geometry_num(); };
	int get_assimp_model_resource_num() { return list_model_resource->get_geometry_num(); };
	HRESULT add_assimp_modelview_by_name(std::string model_name, std::string model_view_name,float time_now_need = 0.0f);
	void delete_assimp_modelview_by_name(std::string model_view_name);
	HRESULT add_assimp_modelview_by_index(int model_ID, std::string model_view_name);
	assimpmodel_resource_view *get_assimp_ModelResourceView_by_name(std::string model_view_name);
	assimpmodel_resource_view *get_assimp_ModelResourceView_by_index(int model_view_idnex);
	void update_assimp_MRV_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time) { list_model_assimp->update_geometry_byname(name_input,world_matrix_need,delta_time); };
	void update_assimp_MRV_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time) { list_model_assimp->update_geometry_byindex(index_input, world_matrix_need, delta_time); };
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���ü�����ĵ����������������洢��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT init_buildin_geometry(Geometry_basic *data_in, std::string geometry_name,int &geometry_index);
	int get_BuiltIn_model_view_num() { return list_buildin_model_view->get_geometry_num(); };
	int get_BuiltIn_model_resource_num() { return list_buildin_geometry_resource->get_geometry_num(); };
	void delete_BuiltIn_modelview_by_name(std::string model_view_name);
	HRESULT add_buildin_modelview_by_name(std::string model_name, std::string model_view_name, Geometry_type type_need);
	HRESULT add_buildin_modelview_by_index(int model_ID, std::string model_view_name, Geometry_type type_need);
	buildin_geometry_resource_view *get_buildin_GeometryResourceView_by_name(std::string model_view_name);
	buildin_geometry_resource_view *get_buildin_GeometryResourceView_by_index(int model_view_idnex);
	void update_buildin_GRV_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time) { list_buildin_model_view->update_geometry_byname(name_input, world_matrix_need, delta_time); };
	void update_buildin_GRV_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time) { list_buildin_model_view->update_geometry_byindex(index_input, world_matrix_need, delta_time); };
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ֲ��������ĵ����������������洢��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int get_plant_model_view_num() { return list_plant_model_view->get_geometry_num(); };

	HRESULT add_plant_modelview_by_name(std::string model_name, std::string model_view_name);
	HRESULT add_plant_modelview_by_index(int model_ID, std::string model_view_name);

	HRESULT add_plant_instance_by_name(std::string model_view_name, int map_location, XMFLOAT4X4 mat_translation) { list_plant_model_view->add_instance_by_name(model_view_name, map_location, mat_translation); }
	HRESULT add_plant_instance_by_index(int model_ID, int map_location, XMFLOAT4X4 mat_translation) { list_plant_model_view->add_instance_by_idnex(model_ID, map_location, mat_translation); }
	plant_resource_view *get_plant_ResourceView_by_name(std::string model_view_name);
	plant_resource_view *get_plant_ResourceView_by_index(int model_view_idnex);
	void update_plant_GRV_byname(std::string name_input, float delta_time) { list_plant_model_view->update_geometry_byname(name_input, delta_time); };
	void update_plant_GRV_byindex(int index_input, float delta_time) { list_plant_model_view->update_geometry_byindex(index_input, delta_time); };

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���⼸����ĵ����������������洢��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int get_special_model_view_num() { return 0; };
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���μ����崴��~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool check_if_have_terrain() { return if_have_terrain; };
	void build_terrain_from_memory(pancy_terrain_build *terrain_data) { if_have_terrain = true; terrain_tesselation = terrain_data; };
	pancy_terrain_build* get_terrain_data() {return terrain_tesselation;};
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~������Դ������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT load_texture_from_file(wchar_t *file_name, bool if_use_mipmap, std::string resource_name, int &index_output);
	int get_texture_num() { return list_texture_use->get_geometry_num(); };
	texture_pack *get_texture_byname(std::string name) { return list_texture_use->get_resource_by_name(name); };
	texture_pack *get_texture_byindex(int index) { return list_texture_use->get_resource_by_index(index); };
	
	mesh_billboard                *get_grass_common() { return grass_billboard; };
	HRESULT create();
	void release();
};
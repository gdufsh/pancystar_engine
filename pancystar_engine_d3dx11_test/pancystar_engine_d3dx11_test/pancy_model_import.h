#pragma once
#include<windows.h>
#include"geometry.h"
#include <assimp/Importer.hpp>      // �������ڸ�ͷ�ļ��ж���
#include <assimp/scene.h>           // ��ȡ����ģ�����ݶ�����scene��
#include <assimp/postprocess.h>     // ��ͷ�ļ��а�������ı�־λ����
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
#include<assimp/Exporter.hpp>
#include<string>
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
template<typename T>
struct mesh_list
{
	Geometry<T> *point_buffer;
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
	virtual void draw_normal_part(int i) = 0;
	virtual void draw_mesh() = 0;
	virtual void draw_mesh_adj() = 0;
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
	void change_texturedesc_2dds(char rec[]);
	void release_basic();
};
template<typename T>
class model_reader_assimp : public assimp_basic
{
protected:
	mesh_list<T> *mesh_need;        //�����
	mesh_list<T> *mesh_need_normal; //�����Ż���������
	Geometry<T> *mesh_scene;  //�洢�ϲ��ĳ�������
public:
	model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	void get_texture(material_list *texture_need, int i);
	void get_normaltexture(material_list *texture_need, int i);
	void release();
	void draw_part(int i);
	void draw_mesh();
	void draw_mesh_adj();
	void draw_normal_part(int i);
protected:
	virtual HRESULT init_mesh(bool if_adj);
	HRESULT combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj);
	HRESULT optimization_mesh(bool if_adj);//�����Ż�
	HRESULT optimization_normalmesh(bool if_adj);//�����Ż�
};
template<typename T>
model_reader_assimp<T>::model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path) : assimp_basic(device_need, contex_need, pFile, texture_path)
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
	mesh_optimization = now_different - 1;
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
			if (mesh_need[j].material_use == i + 1)
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
		rec_optisave[i].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, now_count_vertex, now_count_index);
		rec_optisave[i].material_use = i + 1;
		HRESULT hr = rec_optisave[i].point_buffer->create_object(vertex_rec, index_rec, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"combine scene error", L"tip", MB_OK);
			return hr;
		}
	}
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
	for (int i = 0; i < mesh_optimization; i++) 
	{
		if (rec_meshtex_save[i] == 0 && if_alpha_array[i] == false)
		{
			matlist_save[mesh_normal_optimization++] = 0;
			break;
		}
	}
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
	XMFLOAT4X4* get_bone_matrix(int i, int &num_bone);
	XMFLOAT4X4* get_bone_matrix();
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
};
class geometry_member
{
	std::string geometry_name;
	int indexnum_geometry;
	assimp_basic *model_data;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 *bone_matrix;
	bool if_skinmesh;
	int bone_num;
	geometry_member *next;
	geometry_member *pre;
public:
	geometry_member(assimp_basic *model_input, bool if_skin, XMFLOAT4X4 matrix_need, XMFLOAT4X4 *matrix_bone, int bone_num_need, std::string name_need, int indexnum_need);
	void draw_full_geometry(ID3DX11EffectTechnique *tech_common);
	void draw_full_geometry_adj(ID3DX11EffectTechnique *tech_common);
	void draw_transparent_part(ID3DX11EffectTechnique *tech_transparent, int transparent_part);
	void draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part);
	bool check_if_skin() { return if_skinmesh; };

	XMFLOAT4X4 get_world_matrix() { return world_matrix; };
	XMFLOAT4X4* get_bone_matrix() { return if_skinmesh ? bone_matrix : NULL; };
	int get_bone_num() { return bone_num; };
	geometry_member *get_last_member() { return pre; };
	geometry_member *get_next_member() { return next; };
	void set_next_member(geometry_member *next_need) { next = next_need; };
	void set_pre_member(geometry_member *pre_data) { pre = pre_data; };
	std::string get_geometry_name() { return geometry_name; };
	int get_geometry_index() { return indexnum_geometry; };
	void release();
	void update(XMFLOAT4X4 world_matrix_need, float delta_time);
	void get_texture(material_list *texture_need, int i) { model_data->get_texture(texture_need, i); };
	assimp_basic *get_geometry_data() { return model_data; };
private:
	void reset_world_matrix(XMFLOAT4X4 world_matrix_need) { world_matrix = world_matrix_need; };
	void reset_bone_matrix(XMFLOAT4X4 *bone_matrix_need, int bone_num_need) { bone_matrix = bone_matrix_need; bone_num = bone_num_need; };
};
class scene_geometry_list
{
	geometry_member *head;
	geometry_member *tail;
	int number_list;
public:
	scene_geometry_list();
	void add_new_geometry(geometry_member *data_input);
	void delete_geometry_byname(std::string name_input);
	geometry_member *get_geometry_byname(std::string name_input);
	geometry_member *get_geometry_head() { return head; };
	geometry_member *get_geometry_tail() { return tail; };
	int get_geometry_num() { return number_list; };
	void update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time);
	void release();
};
class geometry_control
{
	ID3D11Device        *device_pancy;     //d3d�豸
	ID3D11DeviceContext *contex_pancy;     //�豸������
	scene_geometry_list *list_model_assimp;

	Geometry<point_with_tangent>  *floor_need;          //����ģ��
	Geometry<point_with_tangent>  *sky_need;            //����ģ��
	ID3D11ShaderResourceView      *tex_skycube;         //��պ�
	ID3D11ShaderResourceView      *tex_floor;           //����������ͼָ��
	ID3D11ShaderResourceView      *tex_normal;          //������ͼ
	mesh_billboard                *grass_billboard;     //�����

	ID3D11ShaderResourceView      *tex_grass;           //��պ�
	ID3D11ShaderResourceView      *tex_grassnormal;     //����������ͼָ��
	ID3D11ShaderResourceView      *tex_grassspec;       //������ͼ

public:
	geometry_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	Geometry<point_with_tangent>  *get_floor_geometry() { return floor_need; };
	Geometry<point_with_tangent>  *get_sky_geometry() { return sky_need; };
	ID3D11ShaderResourceView      *get_basic_floor_tex() { return tex_floor; };
	ID3D11ShaderResourceView      *get_floor_normal_tex() { return tex_normal; };
	ID3D11ShaderResourceView      *get_sky_cube_tex() { return tex_skycube; };

	ID3D11ShaderResourceView      *get_grass_tex() { return tex_grass; };
	ID3D11ShaderResourceView      *get_grassnormal_tex() { return tex_grassnormal; };
	ID3D11ShaderResourceView      *get_grassspec_tex() { return tex_grassspec; };

	scene_geometry_list *get_model_list() { return list_model_assimp; };
	mesh_billboard                *get_grass_common() { return grass_billboard; };
	HRESULT create();
	void release();
};
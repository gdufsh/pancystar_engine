#include"pancy_model_import.h"
model_reader_assimp::model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	matlist_need = NULL;
	mesh_need = NULL;
	mesh_scene = NULL;
	material_optimization = 0;
}
int model_reader_assimp::get_meshnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_optimization;
}
void model_reader_assimp::remove_texture_path(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '\\')
		{
			start = i + 1;
		}
	}
	strcpy(rec, &rec[start]);
}
HRESULT model_reader_assimp::model_create(bool if_optimize)
{
	aiProcess_ConvertToLeftHanded;
	model_need = importer.ReadFile(filename,
		//aiProcess_MakeLeftHanded |
		//aiProcess_FlipWindingOrder |
		aiProcess_CalcTangentSpace |             //�������ߺ͸�����
												 //aiProcess_Triangulate |                 //���ı�����ת��Ϊ������
		aiProcess_JoinIdenticalVertices		//�ϲ���ͬ�Ķ���
											//aiProcess_SortByPType
		);//����ͬͼԪ���õ���ͬ��ģ����ȥ��ͼƬ���Ϳ����ǵ㡢ֱ�ߡ������ε�
	if (!model_need)
	{
		return E_FAIL;
	}
	matlist_need = new material_list[model_need->mNumMaterials];
	for (unsigned int i = 0; i < model_need->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = model_need->mMaterials[i];
		aiString Path;
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 && pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_diffuse, Path.data);
			remove_texture_path(matlist_need[i].texture_diffuse);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_diffuse);
			strcpy(matlist_need[i].texture_diffuse, rec_name);
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_normal);
			strcpy(matlist_need[i].texture_normal, rec_name);
		}
	}
	HRESULT hr;
	hr = init_mesh();
	if (hr != S_OK)
	{
		MessageBox(0,L"create model error when init mesh",L"tip",MB_OK);
		return hr;
	}
	hr = init_texture();
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when init texture", L"tip", MB_OK);
		return hr;
	}
	hr = combine_vertex_array();
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when combine scene mesh", L"tip", MB_OK);
		return hr;
	}
	if (if_optimize == true) 
	{
		optimization_mesh();
	}
	return S_OK;
}
HRESULT model_reader_assimp::init_mesh()
{
	point_with_tangent *point_need;
	unsigned int *index_need;
	//���������¼��
	mesh_need = new mesh_list[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//��ȡģ�͵ĵ�i��ģ��
		const aiMesh* paiMesh = model_need->mMeshes[i];
		//��ȡģ�͵Ĳ��ʱ��
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		//ģ�͵ĵ�i��ģ��Ķ��㼰������Ϣ
		mesh_need[i].point_buffer = new mesh_comman(device_pancy, contex_pancy, paiMesh->mNumVertices, paiMesh->mNumFaces * 3);
		point_need = (point_with_tangent*)malloc(paiMesh->mNumVertices * sizeof(point_with_tangent));
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
				point_need[j].tex.y = 1-paiMesh->mTextureCoords[0][j].y;
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
		HRESULT hr = mesh_need[i].point_buffer->create_object(point_need, index_need);
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
HRESULT model_reader_assimp::init_texture()
{
	HRESULT hr_need;
	for (int i = 0; i < model_need->mNumMaterials; ++i)
	{
		//������������ͼ
		if (matlist_need[i].texture_diffuse[0] != '\0')
		{
			//ת���ļ���Ϊunicode
			size_t len = strlen(matlist_need[i].texture_diffuse) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_diffuse, _TRUNCATE);
			//�����ļ�������������Դ
			hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, 0, &matlist_need[i].tex_diffuse_resource, 0, 0);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//�ͷ���ʱ�ļ���
			free(texture_name);
			texture_name = NULL;
		}
		//����������ͼ
		if (matlist_need[i].texture_normal[0] != '\0')
		{
			//ת���ļ���Ϊunicode
			size_t len = strlen(matlist_need[i].texture_normal) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_normal, _TRUNCATE);
			//�����ļ�������������Դ
			hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, 0, &matlist_need[i].texture_normal_resource, 0, 0);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//�ͷ���ʱ�ļ���
			free(texture_name);
			texture_name = NULL;
		}
	}
	material_optimization = model_need->mNumMaterials;
	return S_OK;
}
HRESULT model_reader_assimp::get_technique(ID3DX11EffectTechnique *teque_need)
{
	if (teque_need == NULL) 
	{
		MessageBox(0, L"get render technique error", L"tip", MB_OK);
		return E_FAIL;
	}
	teque_pancy = teque_need;
	return S_OK;
}
HRESULT model_reader_assimp::combine_vertex_array()
{
	point_with_tangent *vertex_out;
	UINT *index_out;
	int all_vertex = 0, all_index = 0;
	Geometry<point_with_tangent> *now_meshneed[700];
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		now_meshneed[i] = mesh_need[i].point_buffer;
	}
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		int rec_vertex, rec_index;
		now_meshneed[i]->get_point_num(rec_vertex, rec_index);
		all_vertex += rec_vertex;
		all_index += rec_index;
	}
	vertex_out = (point_with_tangent*)malloc(all_vertex * sizeof(point_with_tangent) + 100);
	index_out = (UINT*)malloc(all_index * sizeof(UINT) + 100);
	//�Լ�������кϲ�����
	point_with_tangent *now_pointer = vertex_out;
	UINT *now_index_pointer = index_out;
	int vertex_num = 0;
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		point_with_tangent *vertex_rec;
		UINT *index_rec;
		int all_vertex = 0, all_index = 0;
		//��ȡ������Ķ�����������Ŀ
		now_meshneed[i]->get_point_num(all_vertex, all_index);
		vertex_rec = (point_with_tangent*)malloc(all_vertex * sizeof(point_with_tangent) + 100);
		index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);
		//��ȡ������Ķ�������������
		now_meshneed[i]->get_bufferdata(vertex_rec, index_rec);
		//��������
		memcpy(now_pointer, vertex_rec, all_vertex * sizeof(point_with_tangent));
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
	mesh_scene = new mesh_comman(device_pancy, contex_pancy, all_vertex, all_index);
	HRESULT hr = mesh_scene->create_object(vertex_out, index_out);
	if (FAILED(hr))
	{
		MessageBox(0, L"combine scene error", L"tip", MB_OK);
		return hr;
	}
	free(vertex_out);
	free(index_out);
	return S_OK;
}

void model_reader_assimp::get_texture(material_list *texture_need, int i)
{
	*texture_need = matlist_need[mesh_need[i].material_use];
}
void model_reader_assimp::release()
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
		//�ͷŻ�������Դ
		for (int i = 0; i < mesh_optimization; i++)
		{
			mesh_need[i].point_buffer->release();
		}
		mesh_scene->release();
		//�ͷű���Դ
		delete[] matlist_need;
		delete[] mesh_need;
		model_need->~aiScene();
	}
}
void model_reader_assimp::draw_part(int i)
{
	mesh_need[i].point_buffer->get_teque(teque_pancy);
	mesh_need[i].point_buffer->show_mesh();
}
void model_reader_assimp::draw_mesh() 
{
	mesh_scene->get_teque(teque_pancy);
	mesh_scene->show_mesh();
}
HRESULT model_reader_assimp::optimization_mesh() 
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�Ⱥϲ�����~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int hash_UnionFind[1000];
	memset(hash_UnionFind,0, 1000 * sizeof(int));
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
			if (strcmp(matlist_need[i].texture_diffuse,matlist_need[j].texture_diffuse) == 0 && strcmp(matlist_need[i].texture_normal,matlist_need[j].texture_normal) == 0) 
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
	mesh_optimization = now_different-1;
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
	point_with_tangent *vertex_rec;
	UINT *index_rec;
	vertex_rec = (point_with_tangent*)malloc(all_vertex * sizeof(point_with_tangent) + 100);
	index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);

	//������ʱ�洢�������ָ��
	mesh_list *rec_optisave;
	rec_optisave = new mesh_list[mesh_optimization + 10];
	//Ϊ�µļ�������֯�������������
	for (int i = 0; i < mesh_optimization; ++i)
	{
		point_with_tangent *now_pointer = vertex_rec;
		UINT *now_index_pointer = index_rec;
		int vertex_num = 0;
		int now_count_vertex = 0;
		int now_count_index = 0;
		for (int j = 0; j < model_need->mNumMeshes; j++) 
		{
			//�ҵ�����ʹ�ò�����ͬ�����񣬲������Ǻϲ�
			if (mesh_need[j].material_use == i+1) 
			{
					point_with_tangent *vertex_rec;
					UINT *index_rec;
					int check_vertex = 0, check_index = 0;
					//��ȡ������Ķ�����������Ŀ
					mesh_need[j].point_buffer->get_point_num(check_vertex, check_index);
					now_count_vertex += check_vertex;
					now_count_index += check_index;
					vertex_rec = (point_with_tangent*)malloc(check_vertex * sizeof(point_with_tangent) + 100);
					index_rec = (UINT*)malloc(check_index * sizeof(UINT) + 100);
					//��ȡ������Ķ�������������
					mesh_need[j].point_buffer->get_bufferdata(vertex_rec, index_rec);
					

					//��������
					memcpy(now_pointer, vertex_rec, check_vertex * sizeof(point_with_tangent));
					//�޸�������
					for (int k = 0; k< check_index; ++k)
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
		rec_optisave[i].point_buffer = new mesh_comman(device_pancy, contex_pancy, now_count_vertex, now_count_index);
		rec_optisave[i].material_use = i+1;
		HRESULT hr = rec_optisave[i].point_buffer->create_object(vertex_rec, index_rec);
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
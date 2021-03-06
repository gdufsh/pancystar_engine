#include"pancy_model_import.h"

assimp_basic::assimp_basic(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	matlist_need = NULL;
	material_optimization = 0;
}
int assimp_basic::get_meshnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_optimization;
}
int assimp_basic::get_meshnormalnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_normal_optimization;
}
void assimp_basic::remove_texture_path(char rec[])
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
void assimp_basic::change_texturedesc_2dds(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '.')
		{
			rec[i + 1] = 'd';
			rec[i + 2] = 'd';
			rec[i + 3] = 's';
			rec[i + 4] = '\0';
		}
	}
	strcpy(rec, &rec[start]);
}
HRESULT assimp_basic::model_create(bool if_adj, bool if_optimize, int alpha_partnum, int* alpha_part)
{
	for (int i = 0; i < 10000; ++i)
	{
		if_alpha_array[i] = false;
		if_normal_array[i] = false;
	}
	if (alpha_partnum != 0 && alpha_part != NULL)
	{
		for (int i = 0; i < alpha_partnum; ++i)
		{
			if_alpha_array[alpha_part[i]] = true;
		}
	}
	//aiProcess_ConvertToLeftHanded;
	model_need = importer.ReadFile(filename,
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_CalcTangentSpace |             //计算切线和副法线
												 //aiProcess_Triangulate |                 //将四边形面转换为三角面
		aiProcess_JoinIdenticalVertices		//合并相同的顶点
											//aiProcess_SortByPType
		);//将不同图元放置到不同的模型中去，图片类型可能是点、直线、三角形等
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
			change_texturedesc_2dds(matlist_need[i].texture_diffuse);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_diffuse);
			strcpy(matlist_need[i].texture_diffuse, rec_name);
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			change_texturedesc_2dds(matlist_need[i].texture_normal);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_normal);
			strcpy(matlist_need[i].texture_normal, rec_name);
		}
	}
	HRESULT hr;
	hr = init_mesh(if_adj);
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when init mesh", L"tip", MB_OK);
		return hr;
	}
	hr = init_texture();
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when init texture", L"tip", MB_OK);
		return hr;
	}
	if (if_optimize == true)
	{
		optimization_mesh(if_adj);
	}
	hr = combine_vertex_array(alpha_partnum, alpha_part, if_adj);
	optimization_normalmesh(if_adj);
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when combine scene mesh", L"tip", MB_OK);
		return hr;
	}
	copy_Geometry_Resource();
	return S_OK;
}
HRESULT assimp_basic::init_texture()
{
	HRESULT hr_need;
	for (int i = 0; i < model_need->mNumMaterials; ++i)
	{
		//创建漫反射贴图
		if (matlist_need[i].texture_diffuse[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_diffuse) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_diffuse, _TRUNCATE);
			//根据文件名创建纹理资源
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, 0, &matlist_need[i].tex_diffuse_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].tex_diffuse_resource);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
		//创建法线贴图
		if (matlist_need[i].texture_normal[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_normal) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_normal, _TRUNCATE);
			//根据文件名创建纹理资源

			ID3D11Resource* Texture = NULL;
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, &Texture, &matlist_need[i].texture_normal_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].texture_normal_resource);

			//contex_pancy->GenerateMips();
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
	}
	material_optimization = model_need->mNumMaterials;
	return S_OK;
}
HRESULT assimp_basic::get_technique(ID3DX11EffectTechnique *teque_need)
{
	if (teque_need == NULL)
	{
		MessageBox(0, L"get render technique error", L"tip", MB_OK);
		return E_FAIL;
	}
	teque_pancy = teque_need;
	return S_OK;
}
void assimp_basic::release_basic()
{
	//释放纹理资源
	/*
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
		//释放表资源
		delete[] matlist_need;
		model_need->~aiScene();
	}*/
}

modelview_basic_assimp::modelview_basic_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path) : assimp_basic(device_need, contex_need, filename, texture_path)
{
}
void modelview_basic_assimp::release()
{
	//释放纹理资源
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
		//释放缓冲区资源
		for (int i = 0; i < mesh_optimization; i++)
		{
			mesh_need_view[i].point_buffer->release();
		}
		for (int i = 0; i < mesh_normal_optimization; i++)
		{
			mesh_need_normal_view[i].point_buffer->release();
		}
		mesh_scene_view->release();
		//释放表资源
		delete[] mesh_need_view;
		//释放表资源
		delete[] matlist_need;
		//model_need->~aiScene();
	}
}
void modelview_basic_assimp::draw_part(int i)
{
	mesh_need_view[i].point_buffer->get_teque(teque_pancy);
	mesh_need_view[i].point_buffer->show_mesh();
}
void modelview_basic_assimp::draw_part_instance(int i, int copy_num)
{
	mesh_need_view[i].point_buffer->get_teque(teque_pancy);
	mesh_need_view[i].point_buffer->show_mesh_instance(copy_num);
}
void modelview_basic_assimp::draw_normal_part(int i)
{
	mesh_need_normal_view[i].point_buffer->get_teque(teque_pancy);
	mesh_need_normal_view[i].point_buffer->show_mesh();
}
void modelview_basic_assimp::draw_normal_part_instance(int i, int copy_num)
{
	mesh_need_normal_view[i].point_buffer->get_teque(teque_pancy);
	mesh_need_normal_view[i].point_buffer->show_mesh_instance(copy_num);
}
void modelview_basic_assimp::draw_mesh()
{
	mesh_scene_view->get_teque(teque_pancy);
	mesh_scene_view->show_mesh();
}
void modelview_basic_assimp::draw_mesh_adj()
{
	mesh_scene_view->get_teque(teque_pancy);
	mesh_scene_view->show_mesh_adj();
}
void modelview_basic_assimp::draw_mesh_instance(int copy_num)
{
	mesh_scene_view->get_teque(teque_pancy);
	mesh_scene_view->show_mesh_instance(copy_num);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~骨骼动画~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
skin_mesh::skin_mesh(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path) : model_reader_assimp(device_need, contex_need, filename, texture_path)
{
	hand_matrix_number = 0;
	root_skin = new skin_tree;
	strcpy(root_skin->bone_ID, "root_node");
	root_skin->son = new skin_tree;
	first_animation = NULL;
	bone_num = 0;
	time_all = 0.0f;
	//float rec_need1[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	for (int i = 0; i < 100; ++i)
	{
		XMStoreFloat4x4(&bone_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&offset_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&final_matrix_array[i], XMMatrixIdentity());
	}

	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			tree_node_num[i][j] = 0;
		}
	}
}
void skin_mesh::set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in)
{
	out._11 = in->a1;
	out._21 = in->a2;
	out._31 = in->a3;
	out._41 = in->a4;

	out._12 = in->b1;
	out._22 = in->b2;
	out._32 = in->b3;
	out._42 = in->b4;

	out._13 = in->c1;
	out._23 = in->c2;
	out._33 = in->c3;
	out._43 = in->c4;

	out._14 = in->d1;
	out._24 = in->d2;
	out._34 = in->d3;
	out._44 = in->d4;
}

bool skin_mesh::check_ifsame(char a[], char b[])
{
	int length = strlen(a);
	if (strlen(a) != strlen(b))
	{
		return false;
	}
	for (int i = 0; i < length; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}
aiNode *skin_mesh::find_skinroot(aiNode *now_node, char root_name[])
{
	if (now_node == NULL)
	{
		return NULL;
	}
	if (check_ifsame(root_name, now_node->mName.data))
	{
		return now_node;
	}
	for (int i = 0; i < now_node->mNumChildren; ++i)
	{
		aiNode *p = find_skinroot(now_node->mChildren[i], root_name);
		if (p != NULL)
		{
			return p;
		}
	}
	return NULL;
}
HRESULT skin_mesh::build_skintree(aiNode *now_node, skin_tree *now_root)
{
	if (now_node != NULL)
	{
		strcpy(now_root->bone_ID, now_node->mName.data);
		set_matrix(now_root->animation_matrix, &now_node->mTransformation);
		//set_matrix(now_root->basic_matrix, &now_node->mTransformation);
		now_root->bone_number = -10;
		//bone_num += 1;
		if (now_node->mNumChildren > 0)
		{
			//如果有至少一个儿子则建立儿子结点
			now_root->son = new skin_tree();
			build_skintree(now_node->mChildren[0], now_root->son);
		}
		skin_tree *p = now_root->son;
		for (int i = 1; i < now_node->mNumChildren; ++i)
		{
			//建立所有的兄弟链
			p->brother = new skin_tree();
			build_skintree(now_node->mChildren[i], p->brother);
			p = p->brother;
		}
	}
	return S_OK;

}
HRESULT skin_mesh::build_animation_list()
{
	for (int i = 0; i < model_need->mNumAnimations; ++i)
	{
		animation_set *p = new animation_set;
		p->animation_length = model_need->mAnimations[i]->mDuration;
		strcpy(p->animation_name, model_need->mAnimations[i]->mName.data);
		p->number_animation = model_need->mAnimations[i]->mNumChannels;
		p->head_animition = NULL;
		for (int j = 0; j < p->number_animation; ++j)
		{
			animation_data *q = new animation_data;
			strcpy(q->bone_name, model_need->mAnimations[i]->mChannels[j]->mNodeName.data);
			q->bone_point = find_tree(root_skin, q->bone_name);
			//旋转四元数
			q->number_rotation = model_need->mAnimations[i]->mChannels[j]->mNumRotationKeys;
			q->rotation_key = new quaternion_animation[q->number_rotation];

			for (int k = 0; k < q->number_rotation; ++k)
			{
				q->rotation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime;
				q->rotation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x;
				q->rotation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y;
				q->rotation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z;
				q->rotation_key[k].main_key[3] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w;
			}
			//平移向量
			q->number_translation = model_need->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			q->translation_key = new vector_animation[q->number_translation];
			for (int k = 0; k < q->number_translation; ++k)
			{
				q->translation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
				q->translation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.x;
				q->translation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.y;
				q->translation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.z;
			}
			//缩放向量
			q->number_scaling = model_need->mAnimations[i]->mChannels[j]->mNumScalingKeys;
			q->scaling_key = new vector_animation[q->number_scaling];
			for (int k = 0; k < q->number_scaling; ++k)
			{
				q->scaling_key[k].time = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime;
				q->scaling_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.x;
				q->scaling_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.y;
				q->scaling_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.z;
			}

			q->next = p->head_animition;
			p->head_animition = q;
		}
		p->next = first_animation;
		first_animation = p;
	}
	return S_OK;
}
skin_tree* skin_mesh::find_tree(skin_tree* p, char name[])
{
	if (check_ifsame(p->bone_ID, name))
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, name);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, name);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
skin_tree* skin_mesh::find_tree(skin_tree* p, int num)
{
	if (p->bone_number == num)
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, num);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, num);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
void skin_mesh::update_root(skin_tree *root, XMFLOAT4X4 matrix_parent)
{
	if (root == NULL)
	{
		return;
	}
	//float rec[16];
	XMMATRIX rec = XMLoadFloat4x4(&root->animation_matrix);
	XMStoreFloat4x4(&root->now_matrix, rec * XMLoadFloat4x4(&matrix_parent));
	//memcpy(rec, root->animation_matrix, 16 * sizeof(float));
	//MatrixMultiply(root->now_matrix, rec, matrix_parent);
	if (root->bone_number >= 0)
	{//memcpy(&bone_matrix_array[root->bone_number], root->now_matrix, 16 * sizeof(float));
		bone_matrix_array[root->bone_number] = root->now_matrix;
	}
	update_root(root->brother, matrix_parent);
	update_root(root->son, root->now_matrix);
}
void skin_mesh::update_mesh_offset(int i)
{
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
	}
}
void skin_mesh::update_mesh_offset()
{
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
		{
			set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
		}
		int a = 0;
	}
	int b = 0;
}
int skin_mesh::find_min(float x1, float x2, float x3, float x4) 
{
	int min = 0;
	float min_rec = x1;
	if (x2 < x1) 
	{
		min = 1;
		min_rec = x2;
	}
	if (x3 < x2)
	{
		min = 2;
		min_rec = x3;
	}
	if (x4 < x3)
	{
		min = 3;
		min_rec = x4;
	}
	return min;
}
XMFLOAT4X4 skin_mesh::get_hand_matrix() 
{
	return final_matrix_array[hand_matrix_number];
}
HRESULT skin_mesh::init_mesh(bool if_adj)
{
	point_withskin *point_need;
	unsigned int *index_need;
	build_skintree(model_need->mRootNode, root_skin->son);
	build_animation_list();
	//创建网格记录表
	mesh_need = new mesh_list<point_withskin>[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	int now_used_bone_num = 0;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{

		//创建顶点缓存区
		const aiMesh* paiMesh = model_need->mMeshes[i];
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		//模型的第i个模块的顶点及索引信息
		mesh_need[i].point_buffer = new mesh_comman<point_withskin>(device_pancy, contex_pancy, paiMesh->mNumVertices, paiMesh->mNumFaces * 3);
		point_need = (point_withskin*)malloc(paiMesh->mNumVertices * sizeof(point_withskin));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
		for (int j = 0; j < paiMesh->mNumVertices; ++j)
		{
			point_need[j].bone_id.x = 99;
			point_need[j].bone_id.y = 99;
			point_need[j].bone_id.z = 99;
			point_need[j].bone_id.w = 99;
			point_need[j].bone_weight.x = 0.0f;
			point_need[j].bone_weight.y = 0.0f;
			point_need[j].bone_weight.z = 0.0f;
			point_need[j].bone_weight.w = 0.0f;
		}
		//顶点缓存
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

		for (int j = 0; j < paiMesh->mNumBones; ++j)
		{
			skin_tree * now_node = find_tree(root_skin, paiMesh->mBones[j]->mName.data);
			if (now_node->bone_number == -10)
			{
				now_node->bone_number = now_used_bone_num++;
				if (strcmp(now_node->bone_ID, "GothGirlRArmPalm") == 0) 
				{
					hand_matrix_number = now_used_bone_num - 1;
				}
				bone_num += 1;
			}
			tree_node_num[i][j] = now_node->bone_number;
			//set_matrix(now_node->basic_matrix, &paiMesh->mBones[j]->mOffsetMatrix);
			for (int k = 0; k < paiMesh->mBones[j]->mNumWeights; ++k)
			{
				if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.x = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.y = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.z = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.w = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else 
				{
					XMUINT4 rec_number = point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id;
					XMFLOAT4 rec_weight = point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight;
					int rec_min_weight = find_min(rec_weight.x, rec_weight.y, rec_weight.z, rec_weight.w);

					if (rec_min_weight == 0 && rec_weight.x < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.x += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 1 && rec_weight.y < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.y += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 2 && rec_weight.z < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.z += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
					if (rec_min_weight == 3 && rec_weight.w < paiMesh->mBones[j]->mWeights[k].mWeight)
					{
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w = now_node->bone_number;
						point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.w += paiMesh->mBones[j]->mWeights[k].mWeight;
					}
				}
			}
		}
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[i].mNumIndices == 3)
			{
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[0];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[1];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[2];
			}
			else
			{
				index_need[count_index++] = 0;
				index_need[count_index++] = 0;
				index_need[count_index++] = 0;
				//return E_FAIL;
			}
		}
		//根据内存信息创建显存区
		HRESULT hr = mesh_need[i].point_buffer->create_object(point_need, index_need, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"create model mesh error", L"tip", MB_OK);
			return hr;
		}
		//释放内存
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}


	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
	return S_OK;
}
void skin_mesh::release_all()
{
	release();
	free_tree(root_skin);
}
void skin_mesh::free_tree(skin_tree *now)
{
	if (now->brother != NULL)
	{
		free_tree(now->brother);
	}
	if (now->son != NULL)
	{
		free_tree(now->son);
	}
	if (now != NULL)
	{
		free(now);
	}
}
XMFLOAT4X4* skin_mesh::get_bone_matrix(int i, int &num_bone)
{
	num_bone = model_need->mMeshes[i]->mNumBones;
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[tree_node_num[i][j]], XMLoadFloat4x4(&offset_matrix_array[tree_node_num[i][j]]) * XMLoadFloat4x4(&bone_matrix_array[tree_node_num[i][j]]));
	}
	return final_matrix_array;
}
XMFLOAT4X4* skin_mesh::get_bone_matrix()
{
	for (int i = 0; i < 100; ++i)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[i], XMLoadFloat4x4(&offset_matrix_array[i]) * XMLoadFloat4x4(&bone_matrix_array[i]));
	}
	return final_matrix_array;
}
void skin_mesh::update_animation(float delta_time)
{
	time_all = (time_all + delta_time);
	if (time_all >= first_animation->animation_length)
	{
		time_all -= first_animation->animation_length;
	}
	update_anim_data(first_animation->head_animition);
	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void skin_mesh::specify_animation_time(float animation_time)
{
	if (animation_time < 0.0f || animation_time > get_animation_length()) 
	{
		return;
	}
	time_all = animation_time;
	//if (time_all >= first_animation->animation_length)
	//{
	//	time_all -= first_animation->animation_length;
	//}
	update_anim_data(first_animation->head_animition);
	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void skin_mesh::update_anim_data(animation_data *now)
{
	//float rec_trans[16], rec_rot[16], rec_scal[16], rec_mid[16];
	XMMATRIX rec_trans, rec_scal;
	XMFLOAT4X4 rec_rot;
	int start_anim, end_anim;
	if (now == NULL)
	{
		return;
	}
	find_anim_sted(start_anim, end_anim, now->rotation_key, now->number_rotation);
	//四元数插值并寻找变换矩阵
	quaternion_animation rotation_now;
	//rotation_now = now->rotation_key[0];
	if (start_anim == end_anim || end_anim >= now->number_rotation)
	{
		rotation_now = now->rotation_key[start_anim];
	}
	else
	{
		Interpolate(rotation_now, now->rotation_key[start_anim], now->rotation_key[end_anim], (time_all - now->rotation_key[start_anim].time) / (now->rotation_key[end_anim].time - now->rotation_key[start_anim].time));
	}
	//
	Get_quatMatrix(rec_rot, rotation_now);
	//缩放变换
	find_anim_sted(start_anim, end_anim, now->scaling_key, now->number_scaling);
	vector_animation scalling_now;
	if (start_anim == end_anim)
	{
		scalling_now = now->scaling_key[start_anim];
	}
	else
	{
		Interpolate(scalling_now, now->scaling_key[start_anim], now->scaling_key[end_anim], (time_all - now->scaling_key[start_anim].time) / (now->scaling_key[end_anim].time - now->scaling_key[start_anim].time));
	}
	//scalling_now = now->scaling_key[0];
	rec_scal = XMMatrixScaling(scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
	//MatrixScaling(rec_scal, scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
	//平移变换
	find_anim_sted(start_anim, end_anim, now->translation_key, now->number_translation);
	vector_animation translation_now;
	if (start_anim == end_anim)
	{
		translation_now = now->translation_key[start_anim];
	}
	else
	{
		Interpolate(translation_now, now->translation_key[start_anim], now->translation_key[end_anim], (time_all - now->translation_key[start_anim].time) / (now->translation_key[end_anim].time - now->translation_key[start_anim].time));
	}
	//translation_now = now->translation_key[0];
	rec_trans = XMMatrixTranslation(translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//MatrixTranslation(rec_trans, translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//总变换
	//MatrixMultiply(rec_mid, rec_scal, rec_rot);
	//MatrixMultiply(now->bone_point->animation_matrix, rec_mid, rec_trans);
	XMStoreFloat4x4(&now->bone_point->animation_matrix, rec_scal * XMLoadFloat4x4(&rec_rot) * rec_trans);

	update_anim_data(now->next);
}
void skin_mesh::Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor)
{
	float cosom = pStart.main_key[0] * pEnd.main_key[0] + pStart.main_key[1] * pEnd.main_key[1] + pStart.main_key[2] * pEnd.main_key[2] + pStart.main_key[3] * pEnd.main_key[3];
	quaternion_animation end = pEnd;
	if (cosom < static_cast<float>(0.0))
	{
		cosom = -cosom;
		end.main_key[0] = -end.main_key[0];
		end.main_key[1] = -end.main_key[1];
		end.main_key[2] = -end.main_key[2];
		end.main_key[3] = -end.main_key[3];
	}
	float sclp, sclq;
	if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001))
	{
		float omega, sinom;
		omega = acos(cosom);
		sinom = sin(omega);
		sclp = sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
		sclq = sin(pFactor * omega) / sinom;
	}
	else
	{
		sclp = static_cast<float>(1.0) - pFactor;
		sclq = pFactor;
	}

	pOut.main_key[0] = sclp * pStart.main_key[0] + sclq * end.main_key[0];
	pOut.main_key[1] = sclp * pStart.main_key[1] + sclq * end.main_key[1];
	pOut.main_key[2] = sclp * pStart.main_key[2] + sclq * end.main_key[2];
	pOut.main_key[3] = sclp * pStart.main_key[3] + sclq * end.main_key[3];
}
void skin_mesh::Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor)
{
	for (int i = 0; i < 3; ++i)
	{
		pOut.main_key[i] = pStart.main_key[i] + pFactor * (pEnd.main_key[i] - pStart.main_key[i]);
	}
}
void skin_mesh::find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void skin_mesh::find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation-1; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void skin_mesh::Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut)
{
	resMatrix._11 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._21 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] - pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._31 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] + pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._41 = 0.0f;

	resMatrix._12 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._22 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._32 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] - pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._42 = 0.0f;

	resMatrix._13 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] - pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._23 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] + pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._33 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[1] * pOut.main_key[1]);
	resMatrix._43 = 0.0f;

	resMatrix._14 = 0.0f;
	resMatrix._24 = 0.0f;
	resMatrix._34 = 0.0f;
	resMatrix._44 = 1.0f;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~assimp导入模型~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
assimpmodel_resource_view::assimpmodel_resource_view(assimp_basic *model_input, bool if_skin, std::string name_need)
{
	model_data = model_input;
	if_skinmesh = if_skin;
	//bone_matrix = matrix_bone;
	bone_matrix = NULL;
	bone_num = 0;
	geometry_name = name_need;
	indexnum_geometry = -1;
	now_time = 0;
	start_time = 0.0f;
	animation_speed = 20.0f;
	end_time = 100.0f;
	//next = NULL;
	//pre = NULL;
	//XMFLOAT4X4 world_matrix;
	XMStoreFloat4x4(&world_matrix, XMMatrixIdentity());
	//world_matrix = matrix_need;
}
XMFLOAT4X4 assimpmodel_resource_view::get_hand_matrix() 
{
	XMFLOAT4X4 hand_matrix;
	XMStoreFloat4x4(&hand_matrix,XMMatrixIdentity());
	if (if_skinmesh == true)
	{
		skin_mesh *model_animation = static_cast<skin_mesh *>(model_data);
		hand_matrix = model_animation->get_hand_matrix();
	}
	return hand_matrix;
}
void assimpmodel_resource_view::draw_full_geometry(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_technique(tech_common);
	model_data->draw_mesh();
}
void assimpmodel_resource_view::draw_full_geometry_adj(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_technique(tech_common);
	model_data->draw_mesh_adj();
}
void assimpmodel_resource_view::draw_mesh_part(ID3DX11EffectTechnique *tech_transparent,int transparent_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_part(transparent_part);
}
void assimpmodel_resource_view::draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_normal_part(normal_part);
}
void assimpmodel_resource_view::release()
{ 
	/*
	if (if_skinmesh == true)
	{
		skin_mesh *model_animation = static_cast<skin_mesh *>(model_data);
		model_animation->release_all();
	}
	else 
	{
		model_data->release();
	}
	*/
};
void assimpmodel_resource_view::reset_animation_time(float time)
{
	now_time = time;
}
void assimpmodel_resource_view::reset_animation_data(float animation_start_need, float animation_end_need, float animation_speed_need)
{
	start_time = animation_start_need;
	end_time = animation_end_need;
	animation_speed = animation_speed_need;
	//now_time = start_time;
}
void assimpmodel_resource_view::update(XMFLOAT4X4 world_matrix_need, float delta_time)
{
	reset_world_matrix(world_matrix_need);
	if (if_skinmesh == true)
	{
		skin_mesh *model_animation = static_cast<skin_mesh *>(model_data);
		now_time += delta_time * animation_speed;
		if (now_time > end_time)
		{
			now_time -= end_time;
			now_time += start_time;
		}
		//model_animation->update_animation(delta_time * 20);
		model_animation->specify_animation_time(now_time);
		model_animation->update_mesh_offset();
		XMFLOAT4X4 *rec_bonematrix = model_animation->get_bone_matrix();
		int rec_bone_num = model_animation->get_bone_num();
		reset_bone_matrix(rec_bonematrix, rec_bone_num);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~内置几何体~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void buildin_geometry_resource_view::update(XMFLOAT4X4 world_matrix_need, float delta_time)
{
	reset_world_matrix(world_matrix_need);
}
buildin_geometry_resource_view::buildin_geometry_resource_view(Geometry_basic *model_input, std::string name_need, Geometry_type type_need, pancy_physx  *physic_need)
{
	physic_body = NULL;
	physic_pancy = physic_need;
	model_data = model_input;
	geometry_name = name_need;
	mesh_type = type_need;
	indexnum_geometry = -1;
	texture_need = new material_list();
	XMStoreFloat4x4(&world_matrix, XMMatrixIdentity());
}
HRESULT buildin_geometry_resource_view::create_physics()
{
	mat_force = physic_pancy->create_material(0.5,0.5,0.5);
	if (mesh_type == pancy_geometry_cube)
	{
		physx::PxTransform box_pos = physx::PxTransform(physx::PxVec3(0.0f, 200.0f, 0.0f));
		physx::PxBoxGeometry box_geo = physx::PxBoxGeometry(physx::PxVec3(0.5f, 0.5f, 0.5f));
		HRESULT hr = physic_pancy->create_dynamic_box(box_pos, box_geo, mat_force, &physic_body);
		if (FAILED(hr)) 
		{
			return hr;
		}
	}
	return S_OK;
}
HRESULT buildin_geometry_resource_view::update_physx_worldmatrix(float delta_time) 
{
	if (physic_body == NULL) 
	{
		return E_FAIL;
	}
	//设定世界变换
	XMMATRIX trans_world, scal_world, rotation_world, rec_world;
	trans_world = XMMatrixTranslation(physic_pancy->get_position(physic_body).x, physic_pancy->get_position(physic_body).y, physic_pancy->get_position(physic_body).z);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	float angle_rot;
	XMFLOAT3 rotation_data;
	physic_pancy->get_rotation_data(physic_body,angle_rot, rotation_data);
	XMVECTOR rotation_vec = XMLoadFloat3(&rotation_data);
	rotation_world = XMMatrixRotationAxis(rotation_vec, angle_rot);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	update(world_matrix, delta_time);
	return S_OK;
}
void buildin_geometry_resource_view::draw_full_geometry(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_teque(tech_common);
	model_data->show_mesh();
}
void buildin_geometry_resource_view::release() 
{
	delete texture_need;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~植被几何体~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
plant_resource_view::plant_resource_view(assimp_basic *model_input, std::string name_need)
{
	model_data = model_input;
	geometry_name = name_need;
}
HRESULT plant_resource_view::add_a_instance(int map_position, XMFLOAT4X4 world_matrix)
{
	std::pair<int, plant_instance_data> data_need;
	data_need.second.instance_index = map_position;
	data_need.second.world_matrix = world_matrix;
	data_need.first = map_position;
	auto check_iferror = view_data.insert(data_need);
	if (!check_iferror.second)
	{
		MessageBox(0, L"add instance failed,a repeat one already in", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
void plant_resource_view::clear_instance() 
{
	view_data.clear();
}
void plant_resource_view::draw_full_geometry(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_technique(tech_common);
	model_data->draw_mesh_instance(view_data.size());
}
void plant_resource_view::draw_mesh_part(ID3DX11EffectTechnique *tech_transparent, int transparent_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_part_instance(transparent_part,view_data.size());
}
void plant_resource_view::draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_normal_part_instance(normal_part,view_data.size());
}
void plant_resource_view::get_world_matrix_array(int &mat_num, XMFLOAT4X4 *mat)
{
	mat_num = 0;
	for (auto data = view_data.begin(); data != view_data.end(); data++)
	{
		mat[mat_num] = data->second.world_matrix;
		mat_num += 1;
	}
}
void plant_resource_view::update(float delta_time)
{
}
void plant_resource_view::release()
{}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~模型管理表~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~·
void assimp_ResourceView_list::update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_input)
		{
			data_GRV._Ptr->update(world_matrix_need, delta_time);
			break;
		}
	}
}
void assimp_ResourceView_list::update_geometry_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time)
{
	if (index_input > ModelResourceView_list.size())
	{
		return;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_input;
	data_GRV._Ptr->update(world_matrix_need, delta_time);
}
void buildin_ResourceView_list::update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_input)
		{
			data_GRV._Ptr->update(world_matrix_need, delta_time);
			break;
		}
	}
}
void buildin_ResourceView_list::update_geometry_byindex(int index_input, XMFLOAT4X4 world_matrix_need, float delta_time)
{
	if (index_input > ModelResourceView_list.size())
	{
		return;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_input;
	data_GRV._Ptr->update(world_matrix_need, delta_time);
}
void plant_ResourceView_list::update_geometry_byname(std::string name_input, float delta_time)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_input)
		{
			data_GRV._Ptr->update(delta_time);
			break;
		}
	}
}
void plant_ResourceView_list::update_geometry_byindex(int index_input, float delta_time)
{
	if (index_input > ModelResourceView_list.size())
	{
		return;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_input;
	data_GRV._Ptr->update(delta_time);
}
HRESULT plant_ResourceView_list::add_instance_by_name(std::string name_resource_view, int map_location, XMFLOAT4X4 mat_translation)
{
	for (auto data_GRV = ModelResourceView_list.begin(); data_GRV != ModelResourceView_list.end(); data_GRV++)
	{
		std::string name = data_GRV._Ptr->get_geometry_name();
		if (name == name_resource_view)
		{
			HRESULT hr = data_GRV._Ptr->add_a_instance(map_location, mat_translation);
			if (FAILED(hr))
			{
				return E_FAIL;
			}
			return S_OK;
		}
	}
	return E_FAIL;
}
HRESULT plant_ResourceView_list::add_instance_by_idnex(int index_resource_view, int map_location, XMFLOAT4X4 mat_translation)
{
	if (index_resource_view > ModelResourceView_list.size())
	{
		return E_FAIL;
	}
	auto data_GRV = ModelResourceView_list.begin() + index_resource_view;
	HRESULT hr = data_GRV._Ptr->add_a_instance(map_location, mat_translation);
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	return S_OK;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~几何体管理器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
geometry_control::geometry_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_physx *physic_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	physic_pancy = physic_need;
	list_model_resource = new pancy_resource_list<model_resource_data>();
	list_model_assimp = new assimp_ResourceView_list();
	list_buildin_geometry_resource = new pancy_resource_list<BuiltIngeometry_resource_data>();
	list_buildin_model_view = new buildin_ResourceView_list();

	list_plant_model_view = new plant_ResourceView_list();

	grass_billboard = new mesh_billboard(device_need, contex_need);
	list_texture_use = new pancy_resource_list<texture_pack>();
	terrain_tesselation = NULL;
	if_have_terrain = false;
	/*
	tex_floor = NULL;
	tex_normal = NULL;
	tex_skycube = NULL;

	tex_grass = NULL;
	tex_grassnormal = NULL;
	tex_grassspec = NULL;
	*/
}
HRESULT geometry_control::load_modelresource_from_file(char* filename, char* texture_path, bool if_animation,bool if_optimized,bool if_create_adj, int alpha_part_num, int*alpha_part_index, std::string resource_name, int &index_output)
{
	model_resource_data *data_check = list_model_resource->get_resource_by_name(resource_name);
	if (data_check != NULL) 
	{
		MessageBox(0, L"find another assimp model resource which have a same name as Specified assimp model resource", L"tip", MB_OK);
		return E_FAIL;
	}
	model_resource_data data_need;
	data_need.resource_name = resource_name;
	if (!if_animation) 
	{
		data_need.if_skin = false;
		data_need.data = new model_reader_assimp<point_with_tangent>(device_pancy, contex_pancy, filename, texture_path);
	}
	else 
	{
		data_need.if_skin = true;
		data_need.data = new skin_mesh(device_pancy, contex_pancy, filename, texture_path);
	}
	HRESULT hr_need = data_need.data->model_create(if_create_adj, if_optimized, alpha_part_num, alpha_part_index);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model file error", L"tip", MB_OK);
		return hr_need;
	}
	index_output = list_model_resource->add_resource(data_need);
	return S_OK;
}
HRESULT geometry_control::add_assimp_modelview_by_name(std::string model_name,std::string model_view_name, float time_now_need)
{
	model_resource_data *data_need = list_model_resource->get_resource_by_name(model_name);
	if (data_need == NULL)
	{
		MessageBox(0, L"could not find the Specified assimp model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	assimpmodel_resource_view *rec_mesh_check = list_model_assimp->get_geometry_byname(model_view_name);
	if (rec_mesh_check != NULL)
	{
		MessageBox(0, L"find another assimp model view which have a same name as Specified assimp model view", L"tip", MB_OK);
		return E_FAIL;
	}
	assimpmodel_resource_view *rec_mesh_castel = new assimpmodel_resource_view(data_need->data, data_need->if_skin, model_view_name);
	rec_mesh_castel->reset_animation_time(time_now_need);
	list_model_assimp->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
void geometry_control::delete_assimp_modelview_by_name(std::string model_view_name)
{
	list_model_assimp->delete_geometry_byname(model_view_name);
}
void geometry_control::delete_BuiltIn_modelview_by_name(std::string model_view_name)
{
	list_buildin_model_view->delete_geometry_byname(model_view_name);
}
HRESULT geometry_control::add_assimp_modelview_by_index(int model_ID, std::string model_view_name)
{
	model_resource_data *data_need = list_model_resource->get_resource_by_index(model_ID);
	if (data_need == NULL) 
	{
		MessageBox(0, L"could not find the Specified assimp model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	assimpmodel_resource_view *rec_mesh_castel = new assimpmodel_resource_view(data_need->data, data_need->if_skin, model_view_name);
	list_model_assimp->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
assimpmodel_resource_view* geometry_control::get_assimp_ModelResourceView_by_name(std::string model_view_name) 
{
	return list_model_assimp->get_geometry_byname(model_view_name);
}
assimpmodel_resource_view* geometry_control::get_assimp_ModelResourceView_by_index(int model_view_idnex) 
{
	return list_model_assimp->get_geometry_byindex(model_view_idnex);
}

HRESULT geometry_control::init_buildin_geometry(Geometry_basic *data_in, std::string geometry_name, int &geometry_index)
{
	BuiltIngeometry_resource_data *data_check = list_buildin_geometry_resource->get_resource_by_name(geometry_name);
	if (data_check != NULL)
	{
		MessageBox(0, L"find another buildin model resource which have a same name as Specified assimp model resource", L"tip", MB_OK);
		return E_FAIL;
	}
	BuiltIngeometry_resource_data data_need;
	data_need.resource_name = geometry_name;
	if (data_in == NULL) 
	{
		return E_FAIL;
	}
	data_need.data = data_in;
	data_need.resource_index = list_buildin_geometry_resource->add_resource(data_need);
	geometry_index = data_need.resource_index;
	return S_OK;
}
HRESULT geometry_control::add_buildin_modelview_by_name(std::string model_name, std::string model_view_name, Geometry_type type_need)
{
	BuiltIngeometry_resource_data *data_need = list_buildin_geometry_resource->get_resource_by_name(model_name);
	if (data_need == NULL)
	{
		MessageBox(0, L"could not find the Specified buildin model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	buildin_geometry_resource_view *rec_mesh_check = list_buildin_model_view->get_geometry_byname(model_view_name);
	if (rec_mesh_check != NULL)
	{
		MessageBox(0, L"find another assimp model view which have a same name as Specified buildin model view", L"tip", MB_OK);
		return E_FAIL;
	}
	buildin_geometry_resource_view *rec_mesh_castel = new buildin_geometry_resource_view(data_need->data, model_view_name, type_need,physic_pancy);
	rec_mesh_castel->create_physics();
	list_buildin_model_view->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
HRESULT geometry_control::add_buildin_modelview_by_index(int model_ID, std::string model_view_name, Geometry_type type_need)
{
	BuiltIngeometry_resource_data *data_need = list_buildin_geometry_resource->get_resource_by_index(model_ID);
	if (data_need == NULL)
	{
		MessageBox(0, L"could not find the Specified buildin model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	buildin_geometry_resource_view *rec_mesh_castel = new buildin_geometry_resource_view(data_need->data, model_view_name, type_need, physic_pancy);
	rec_mesh_castel->create_physics();
	list_buildin_model_view->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
buildin_geometry_resource_view* geometry_control::get_buildin_GeometryResourceView_by_name(std::string model_view_name) 
{
	return list_buildin_model_view->get_geometry_byname(model_view_name);
}
buildin_geometry_resource_view* geometry_control::get_buildin_GeometryResourceView_by_index(int model_view_idnex) 
{
	return list_buildin_model_view->get_geometry_byindex(model_view_idnex);
}

HRESULT geometry_control::add_plant_modelview_by_name(std::string model_name, std::string model_view_name) 
{
	model_resource_data *data_need = list_model_resource->get_resource_by_name(model_name);
	if (data_need == NULL)
	{
		MessageBox(0, L"could not find the Specified assimp model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	plant_resource_view *rec_mesh_check = list_plant_model_view->get_geometry_byname(model_view_name);
	if (rec_mesh_check != NULL)
	{
		MessageBox(0, L"find another plant model view which have a same name as Specified plant model view", L"tip", MB_OK);
		return E_FAIL;
	}
	plant_resource_view *rec_mesh_castel = new plant_resource_view(data_need->data, model_view_name);
	list_plant_model_view->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
HRESULT geometry_control::add_plant_modelview_by_index(int model_ID, std::string model_view_name) 
{
	model_resource_data *data_need = list_model_resource->get_resource_by_index(model_ID);
	if (data_need == NULL)
	{
		MessageBox(0, L"could not find the Specified assimp model in resource list", L"tip", MB_OK);
		return E_FAIL;
	}
	plant_resource_view *rec_mesh_check = list_plant_model_view->get_geometry_byname(model_view_name);
	if (rec_mesh_check != NULL)
	{
		MessageBox(0, L"find another plant model view which have a same name as Specified plant model view", L"tip", MB_OK);
		return E_FAIL;
	}
	plant_resource_view *rec_mesh_castel = new plant_resource_view(data_need->data, model_view_name);
	list_plant_model_view->add_new_geometry(*rec_mesh_castel);
	delete rec_mesh_castel;
	return S_OK;
}
plant_resource_view *geometry_control::get_plant_ResourceView_by_name(std::string model_view_name) 
{
	return list_plant_model_view->get_geometry_byname(model_view_name);
}
plant_resource_view *geometry_control::get_plant_ResourceView_by_index(int model_view_idnex) 
{
	return list_plant_model_view->get_geometry_byindex(model_view_idnex);
}

HRESULT geometry_control::load_texture_from_file(wchar_t *file_name, bool if_use_mipmap, std::string resource_name, int &index_output)
{
	texture_pack now_use;
	HRESULT hr_need;
	ID3D11ShaderResourceView *rec;
	if (!if_use_mipmap) 
	{
		hr_need = CreateDDSTextureFromFile(device_pancy, file_name, 0, &rec, 0, 0);
	}
	else 
	{
		hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, file_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &rec);
	}
	if (FAILED(hr_need)) 
	{
		MessageBox(0,L"load texture which used to input texlist failed",L"tip",MB_OK);
		return hr_need;
	}
	now_use.resource_name = resource_name;
	now_use.data = new texture_data();
	now_use.data->set_data(rec);
	index_output = list_texture_use->add_resource(now_use);
	return S_OK;
}
HRESULT geometry_control::create()
{
	HRESULT hr_need;
	int index_buildin_rec;
	Geometry<point_with_tangent>  *floor_need = new mesh_cubewithtargent(device_pancy, contex_pancy);          //盒子模型
	//盒子模型
	hr_need = floor_need->create_object();
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load object error", L"tip", MB_OK);
		return hr_need;
	}
	init_buildin_geometry(floor_need,"geometry_cube", index_buildin_rec);
	//add_buildin_modelview_by_index(index_buildin_rec,"geometry_floor");
	Geometry<point_with_tangent>  *sky_need = new mesh_ball(device_pancy, contex_pancy, 50, 50);            //球体模型
	//球体模型
	hr_need = sky_need->create_object();
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load object error", L"tip", MB_OK);
		return hr_need;
	}
	init_buildin_geometry(sky_need, "geometry_ball", index_buildin_rec);
	//add_buildin_modelview_by_index(index_buildin_rec, "geometry_sky");
	//草地模型
	hr_need = grass_billboard->create_object();
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load object error", L"tip", MB_OK);
		return hr_need;
	}
	
	return S_OK;
}
void geometry_control::release()
{
	list_model_resource->release();
	list_buildin_geometry_resource->release();
	list_model_assimp->release();
	list_buildin_model_view->release();
	list_model_assimp->release();
	if (check_if_have_terrain())
	{
		//terrain_tesselation->release();
	}
	list_texture_use->release();
	grass_billboard->release();
	delete list_model_resource; 
	delete list_model_assimp;
	delete list_buildin_geometry_resource;
	delete list_buildin_model_view;
	delete list_texture_use;
}

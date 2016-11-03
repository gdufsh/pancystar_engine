#include"pancy_scene_design.h"
scene_root::scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need,light_control *light_need,int width, int height)
{
	user_input = input_need;
	scene_camera = camera_need;
	shader_lib = lib_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	scene_window_width = width;
	scene_window_height = height;
	//engine_state = engine_root;
	renderstate_lib = render_state;
	geometry_lib = geometry_need;
	light_list = light_need;
	time_game = 0.0f;
	//初始化投影以及取景变换矩阵
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, scene_window_width*1.0f / scene_window_height*1.0f, 0.1f, 300.f);
	ssao_part = new ssao_pancy(render_state,device_need, contex_need, shader_lib,geometry_lib,scene_window_width, scene_window_height, XM_PI*0.25f, 300.0f);
	XMStoreFloat4x4(&proj_matrix, proj);
	XMMATRIX iden = XMMatrixIdentity();
	XMStoreFloat4x4(&view_matrix, iden);
}
HRESULT scene_root::camera_move()
{
	XMMATRIX view;
	user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-0.01f);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(0.01f);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(0.01f);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(0.01f);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-0.01f);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-0.01f);
	}
	if (user_input->check_keyboard(DIK_Q))
	{
		scene_camera->rotation_look(0.001f);
	}
	if (user_input->check_keyboard(DIK_E))
	{
		scene_camera->rotation_look(-0.001f);
	}
	if (user_input->check_mouseDown(1))
	{
		scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
		scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	}
	scene_camera->count_view_matrix(&view_matrix);
	//XMStoreFloat4x4(&view_matrix, view);
	return S_OK;
}
void scene_root::set_proj_matrix(XMFLOAT4X4 proj_mat_need) 
{
	proj_matrix = proj_mat_need;
}
void scene_root::reset_proj_matrix() 
{
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, scene_window_width*1.0f / scene_window_height*1.0f, 0.1f, 300.f);
	XMStoreFloat4x4(&proj_matrix, proj);
}
void scene_root::get_gbuffer(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need)
{
	gbuffer_normalspec = normalspec_need;
	gbuffer_depth = depth_need;
}
void scene_root::get_lbuffer(ID3D11ShaderResourceView *diffuse_need, ID3D11ShaderResourceView *specular_need)
{
	lbuffer_diffuse = diffuse_need;
	lbuffer_specular = specular_need;
}
scene_engine_test::scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need,geometry_need, light_need,width, height)
{
	//nonshadow_light_list.clear();
	//shadowmap_light_list.clear();
	particle_fire = new particle_system<fire_point>(device_need, contex_need, 3000, lib_need, PARTICLE_TYPE_FIRE);
}
HRESULT scene_engine_test::scene_create()
{
	HRESULT hr_need;
	/*
	basic_lighting rec_need(point_light,shadow_none,shader_lib,device_pancy,contex_pancy,renderstate_lib, geometry_lib);
	nonshadow_light_list.push_back(rec_need);

	light_with_shadowmap rec_shadow(spot_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	hr_need = rec_shadow.create(1024, 1024);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	shadowmap_light_list.push_back(rec_shadow);
	*/
	hr_need = ssao_part->basic_create();
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	/*
	light_with_shadowvolume rec_shadowvalum(spot_light, shadow_volume, shader_lib, device_pancy, contex_pancy, renderstate_lib);
	hr_need = rec_shadowvalum.create(1000000);
	
	shadowvalume_light_list.push_back(rec_shadowvalum);
	*/
	hr_need = particle_fire->create(L"flare0.dds");
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load fire particle error", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
HRESULT scene_engine_test::display_shadowao(bool if_shadow, bool if_ao)
{
	renderstate_lib->clear_posttreatmentcrendertarget();
	if (if_ao)
	{
		draw_ssaomap();
	}
	if (if_shadow) 
	{
		draw_shadowmap();
	}
	return S_OK;
}
HRESULT scene_engine_test::display()
{
	show_ball();
	show_lightsource();
	show_floor();
	show_castel_deffered("LightTech");
	//show_castel();
	//show_aotestproj();
	//show_yuri();
	show_yuri_animation_deffered();
	show_yuri_animation();
	//show_billboard();
	//清空深度模板缓冲，在AO绘制阶段记录下深度信息
	//show_fire_particle();
	return S_OK;
}
HRESULT scene_engine_test::display_enviroment() 
{
	//show_ball();
	show_lightsource();
	show_floor();
	show_castel("draw_withtexture", "draw_withtexturenormal");
	return S_OK;
}
HRESULT scene_engine_test::display_nopost()
{
	//renderstate_lib->restore_rendertarget(ssao_part->get_depthstencilmap());
	renderstate_lib->restore_rendertarget();
	show_fire_particle();
	return S_OK;
}
void scene_engine_test::show_yuri_animation() 
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_yuri_pack = geometry_lib->get_model_list()->get_geometry_byname("model_yuri");
	//几何体的固有属性
	auto* model_yuri = model_yuri_pack->get_geometry_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal, *teque_hair;
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES"  ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	shader_test->get_technique(rec_point, num_member ,&teque_need, "drawskin_withshadowssao");
	shader_test->get_technique(rec_point, num_member, &teque_normal, "drawskin_withshadowssaonormal");
	shader_test->get_technique(rec_point, num_member, &teque_hair, "drawskin_hair");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定阴影变换以及阴影贴图
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		XMFLOAT4X4 shadow_matrix_pre = rec_shadow_light._Ptr->get_ViewProjTex_matrix();
		XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
		shadow_matrix = rec_world * shadow_matrix;
		XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
		shader_test->set_trans_shadow(&shadow_matrix_pre);
		shader_test->set_shadowtex(rec_shadow_light._Ptr->get_mapresource());
	}
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	//获取渲染路径并渲染
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//绘制头发
	model_yuri->get_technique(teque_hair);
	material_list rec_need;
	model_yuri->get_texture(&rec_need, yuri_render_order[7]);
	shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
	shader_test->set_normaltex(rec_need.texture_normal_resource);
	model_yuri->draw_part(yuri_render_order[7]);
	contex_pancy->OMSetDepthStencilState(NULL, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_engine_test::show_yuri_animation_deffered()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_yuri_pack = geometry_lib->get_model_list()->get_geometry_byname("model_yuri");
	//几何体的固有属性
	auto* model_yuri = model_yuri_pack->get_geometry_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES"  ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.4f, 0.4f, 0.4f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//获取渲染路径并渲染
	//model_yuri->get_technique(teque_need);
	//model_yuri->draw_mesh();
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	for (int i = 0; i < 7; ++i)
	{
		material_list rec_need;
		model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri->get_technique(teque_need);
		model_yuri->draw_part(yuri_render_order[i]);
	}
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState *rec = renderstate_lib->get_blend_common();
	contex_pancy->OMSetBlendState(rec, blendFactor, 0xffffffff);
	for (int i = 8; i < model_yuri->get_meshnum(); ++i)
	{
		material_list rec_need;
		model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri->get_technique(teque_need);
		model_yuri->draw_part(yuri_render_order[i]);
	}
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	shader_test->set_ssaotex(NULL);
	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void scene_engine_test::show_castel(LPCSTR techname,LPCSTR technamenormal)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	//几何体的固有属性
	auto* model_castel = model_castel_pack->get_geometry_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need,*teque_normal;
	shader_test->get_technique(&teque_need, techname);
	shader_test->get_technique(&teque_normal, technamenormal);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f,1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);


	//设定世界变换
	/*
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	*/
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	shader_test->set_trans_world(&world_matrix);
	//设定阴影变换以及阴影贴图
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		XMFLOAT4X4 shadow_matrix_pre = rec_shadow_light._Ptr->get_ViewProjTex_matrix();

		XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
		shadow_matrix = rec_world * shadow_matrix;
		XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
		shader_test->set_trans_shadow(&shadow_matrix_pre);
		shader_test->set_shadowtex(rec_shadow_light._Ptr->get_mapresource());
	}
	/*
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());
	*/
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	//获取渲染路径并渲染
	//model_castel->get_technique(teque_need);
	//model_castel->draw_mesh();
	//alpha混合设定
	for (int i = 0; i < model_castel->get_meshnum(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL)
		{
			model_castel->get_technique(teque_normal);
			shader_test->set_normaltex(rec_need.texture_normal_resource);
		}
		else 
		{
			model_castel->get_technique(teque_need);
		}
		//shader_test->set_normaltex(tex_normal);
		model_castel->draw_part(i);
	}
	/*
	//设置阴影部分
	geometry_member rec_mesh_need(model_castel, false, false, -1, world_matrix,0,NULL, NULL);
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		//rec_shadow_light._Ptr->add_mesh(rec_mesh_need);
	}
	//设置ssao
	ssao_part->add_mesh(rec_mesh_need);
	*/
}
void scene_engine_test::show_castel_deffered(LPCSTR techname) 
{
	
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	//几何体的固有属性
	auto* model_castel = model_castel_pack->get_geometry_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, techname);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//shader_test->set_enviroment_tex(environment_map);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel->get_meshnum(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel->get_technique(teque_need);
		model_castel->draw_part(i);
	}

	shader_test->set_ssaotex(NULL);
	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
	
	contex_pancy->RSSetState(NULL);
}
void scene_engine_test::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	auto* ball_need = geometry_lib->get_sky_geometry();
	auto* tex_skycube = geometry_lib->get_sky_cube_tex();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_reflect");

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	ball_need->get_teque(teque_need);
	ball_need->show_mesh();
	contex_pancy->RSSetState(NULL);
}
void scene_engine_test::show_lightsource()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_floor_geometry();
	auto* tex_floor = geometry_lib->get_basic_floor_tex();
	auto* tex_normal = geometry_lib->get_floor_normal_tex();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 2.5, 2.5);
	//trans_world = XMMatrixTranslation(-7.3, 4.8, 12.85f);
	scal_world = XMMatrixScaling(0.1f, 0.1f, 0.1f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::show_floor()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_floor_geometry();
	auto* tex_floor = geometry_lib->get_basic_floor_tex();
	auto* tex_normal = geometry_lib->get_floor_normal_tex();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(35.0f, 0.55f, 35.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定阴影变换以及阴影贴图
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		XMFLOAT4X4 shadow_matrix_pre = rec_shadow_light._Ptr->get_ViewProjTex_matrix();

		XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
		shadow_matrix = rec_world * shadow_matrix;
		XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
		shader_test->set_trans_shadow(&shadow_matrix_pre);
		shader_test->set_shadowtex(rec_shadow_light._Ptr->get_mapresource());
	}
	/*
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());
	*/

	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::show_aotestproj()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_floor_geometry();
	auto* tex_floor = geometry_lib->get_basic_floor_tex();
	auto* tex_normal = geometry_lib->get_floor_normal_tex();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadowssao");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.6f, 0.6f, 0.6f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定阴影变换以及阴影贴图
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		XMFLOAT4X4 shadow_matrix_pre = rec_shadow_light._Ptr->get_ViewProjTex_matrix();

		XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
		shadow_matrix = rec_world * shadow_matrix;
		XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
		shader_test->set_trans_shadow(&shadow_matrix_pre);
		shader_test->set_shadowtex(rec_shadow_light._Ptr->get_mapresource());
	}
	/*
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());
	*/

	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::draw_shadowmap()
{
	light_list->draw_shadow();
	/*
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->draw_shadow();
	}
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		//rec_shadow_volume._Ptr->build_shadow(ssao_part->get_depthstencilmap());
		//rec_shadow_volume._Ptr->draw_shadow_volume();
	}
	contex_pancy->RSSetState(NULL);
	*/
}
void scene_engine_test::draw_ssaomap()
{
	//ssao_part->draw_ao(view_matrix,proj_matrix);
	ssao_part->get_normaldepthmap(gbuffer_normalspec, gbuffer_depth);
	ssao_part->compute_ssaomap();
	ssao_part->blur_ssaomap();
	renderstate_lib->set_posttreatment_rendertarget();
}
void scene_engine_test::show_fire_particle()
{
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT3 st_pos = XMFLOAT3(7.25f, 4.6f, 13.2f);
	XMFLOAT3 st_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	particle_fire->set_particle_direct(&st_pos, &st_dir);
	particle_fire->draw_particle();
	contex_pancy->RSSetState(0);
	contex_pancy->OMSetDepthStencilState(0, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
}
void scene_engine_test::show_billboard() 
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_none_rs());
	auto* shader_test = shader_lib->get_shader_grass_billboard();
	auto* floor_need = geometry_lib->get_grass_common();
	shader_test->set_texture_diffuse(geometry_lib->get_grass_tex());
	shader_test->set_texture_normal(geometry_lib->get_grassnormal_tex());
	shader_test->set_texture_specular(geometry_lib->get_grassspec_tex());
	XMFLOAT4X4 rec_mat;
	XMStoreFloat4x4(&rec_mat,XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	shader_test->set_trans_all(&rec_mat);
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_with_tex");
	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
	contex_pancy->RSSetState(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
HRESULT scene_engine_test::update(float delta_time)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_ref = shader_lib->get_shader_reflect();
	shader_ref->set_view_pos(eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	auto* shader_grass = shader_lib->get_shader_grass_billboard();
	shader_grass->set_view_pos(eyePos_rec);
	auto* shader_deff = shader_lib->get_shader_light_deffered_draw();
	shader_deff->set_view_pos(eyePos_rec);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新几何体世界变换~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	auto* model_list = geometry_lib->get_model_list();
	//更新yuri世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.5);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotation_world = XMMatrixRotationY(3.141592653f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	model_list->update_geometry_byname("model_yuri", world_matrix, delta_time);
	model_list->update_geometry_byname("model_yuri_trans", world_matrix, delta_time);
	//更新castel世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	model_list->update_geometry_byname("model_castel", world_matrix, delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	/*
	int count = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->set_frontlight(count);
		rec_shadow_light._Ptr->set_defferedlight(count);
		count += 1;
	}
	//设置无影光源
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light) 
	{
		rec_non_light._Ptr->set_frontlight(count);
		rec_non_light._Ptr->set_defferedlight(count);
		count += 1;
	}
	//设置shadowvolume光源
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		//rec_shadow_volume._Ptr->set_frontlight(count);
		//rec_shadow_volume._Ptr->set_defferedlight(count);
		//count++;
		//rec_shadow_volume._Ptr->update_view_proj_matrix(view_proj);
	}*/
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;
	particle_fire->update(delta_time*0.3, time_game,&view_proj,&eyePos_rec);
	return S_OK;
}
HRESULT scene_engine_test::release()
{
	ssao_part->release();
	particle_fire->release();
	/*
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->release();
	}
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		rec_shadow_volume._Ptr->release();
	}
	*/
	//light_list->release();
	return S_OK;
}
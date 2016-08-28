#include"engine_shadow.h"
shadow_basic::shadow_basic(ID3D11Device *device_need, ID3D11DeviceContext* contex_need, shader_control *shader_list_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_list = shader_list_need;
	depthmap_tex = NULL;
	depthmap_target = NULL;
}
HRESULT shadow_basic::set_renderstate(XMFLOAT3 light_position, XMFLOAT3 light_dir, BoundingSphere shadow_range, light_type check)
{
	if (check == direction_light) 
	{
		//��Դ�ӽ�ȡ���任
		XMVECTOR lightDir = XMLoadFloat3(&light_dir);

		XMVECTOR lightPos = -2.0f*shadow_range.Radius*lightDir;
		XMVECTOR targetPos = XMLoadFloat3(&shadow_range.Center);
		XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMMATRIX viewmat = XMMatrixLookAtLH(lightPos, targetPos, up);


		XMFLOAT3 sphereCenterLS;
		XMVECTOR a = XMVector3TransformCoord(targetPos, viewmat);
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, viewmat));
		//ƽ��ͶӰ
		float l = sphereCenterLS.x - shadow_range.Radius;
		float b = sphereCenterLS.y - shadow_range.Radius;
		float n = sphereCenterLS.z - shadow_range.Radius;
		float r = sphereCenterLS.x + shadow_range.Radius;
		float t = sphereCenterLS.y + shadow_range.Radius;
		float f = sphereCenterLS.z + shadow_range.Radius;
		XMMATRIX proj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

		//��ͶӰ����
		XMMATRIX final_matrix = viewmat * proj;
		XMStoreFloat4x4(&shadow_build, final_matrix);

		//3D�ؽ���ĶԱ�ͶӰ����
		XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
		XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);
	}
	else if (check == spot_light) 
	{
		//��Դ�ӽ�ȡ���任
		XMVECTOR lightDir = XMLoadFloat3(&light_dir);

		XMVECTOR lightPos = -2.0f*shadow_range.Radius*lightDir;
		XMVECTOR targetPos = XMLoadFloat3(&shadow_range.Center);
		XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMMATRIX viewmat = XMMatrixLookAtLH(lightPos, targetPos, up);

		//͸��ͶӰ
		XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, shadowmap_width*1.0f / shadowmap_height*1.0f, 0.1f, 1000.f);

		//��ͶӰ����
		XMMATRIX final_matrix = viewmat * proj;
		XMStoreFloat4x4(&shadow_build, final_matrix);

		//3D�ؽ���ĶԱ�ͶӰ����
		XMMATRIX T_need(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			);
		XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);
	}
	contex_pancy->RSSetViewports(1, &shadow_map_VP);
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

ID3D11ShaderResourceView* shadow_basic::get_mapresource()
{
	return depthmap_tex;
}
void shadow_basic::release()
{
	depthmap_tex->Release();
	depthmap_tex = NULL;
	depthmap_target->Release();
	depthmap_target = NULL;
}
HRESULT shadow_basic::set_viewport(int width_need, int height_need)
{
	shadowmap_width = width_need;
	shadowmap_height = height_need;
	//�ͷ�֮ǰ��shader resource view�Լ�render target view
	if(depthmap_tex != NULL)
	{
		depthmap_tex->Release();
		depthmap_tex = NULL;
	}
	if (depthmap_target != NULL)
	{
		depthmap_target->Release();
		depthmap_target = NULL;
	}
	//ָ����Ⱦ�ӿڵĴ�С
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = static_cast<float>(shadowmap_width);
	shadow_map_VP.Height = static_cast<float>(shadowmap_height);
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	//ָ�����ڴ洢��ȼ�¼��shaderͼƬ��Դ�ĸ�ʽ
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = shadowmap_width;
	texDesc.Height = shadowmap_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//����CPU�ϵ�������Դ
	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = device_pancy->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create texture2D error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//����GPU�ϵ�������Դ��������Դ�Լ���ȾĿ����Դ
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device_pancy->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"create shader resource view error when create shadowmap resource",L"tip",MB_OK);
		return hr;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(depthMap, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create render target view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//�ͷ�CPU�ϵ�������Դ
	if (depthMap != NULL)
	{
		depthMap->Release();
	}
	return S_OK;
}
HRESULT shadow_basic::create(int width_need, int height_need)
{
	return set_viewport(width_need, height_need);
}
HRESULT shadow_basic::set_transparent_tex(ID3D11ShaderResourceView *tex_in)
{
	auto *shader_shadow = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_shadow->set_texture(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set normal depth texture error", L"tip", MB_OK);
		return hr;
	}
}
HRESULT shadow_basic::set_shaderresource(XMFLOAT4X4 word_matrix) 
{
	auto* shader_test = shader_list->get_shader_shadowmap();
	//ѡ������·��
	HRESULT hr = shader_test->get_technique(&teque_need, "ShadowTech");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	hr = shader_test->get_technique(&teque_transparent, "ShadowTech_transparent");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	XMMATRIX rec_final,rec_world;
	XMFLOAT4X4 rec_mat;
	rec_final = XMLoadFloat4x4(&shadow_build);
	rec_world = XMLoadFloat4x4(&word_matrix);
	rec_final = rec_world*rec_final;
	XMStoreFloat4x4(&rec_mat, rec_final);
	hr = shader_test->set_trans_all(&rec_mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set shader matrix error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
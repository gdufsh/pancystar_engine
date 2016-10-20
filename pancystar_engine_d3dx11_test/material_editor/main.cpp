#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_d3d11_basic.h"
int mouse_position_x;
int mouse_position_y;
class GUI_Progress_bar
{
	ID3D11Device            *device_pancy;       //d3d�豸
	ID3D11DeviceContext     *contex_pancy;       //�豸������
	pancy_input             *user_input;          //�����������
	shader_control          *shader_list;         //shader��
	float            now_percent;
	XMFLOAT2         corner_uv[4];
	ID3D11Buffer     *button_VB;          //��ťͼƬ����
	ID3D11Buffer     *button_IB;             //ͼƬ����
	D3D11_VIEWPORT   draw_position;     //����λ��
	ID3D11ShaderResourceView *tex_ui;
public:
	GUI_Progress_bar(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *user_input_need, shader_control *shader_need);
	HRESULT create(wchar_t* texture_path);
	HRESULT init_buffer();
	HRESULT init_texture(wchar_t* texture_path);
	void update();
	void display();
	void release();
};
GUI_Progress_bar::GUI_Progress_bar(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *user_input_need, shader_control *shader_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	user_input = user_input_need;
	shader_list = shader_need;
	now_percent = 0.0f;
}
HRESULT GUI_Progress_bar::init_buffer()
{
	HRESULT hr;
	float button_size = 0.3f *0.5f;
	float line_size = 2.0f *0.5f;
	pancy_point v[8];
	float width = 0.3725f;
	float height = 0.843f - 0.784f;
	//������
	v[0].position = XMFLOAT3(-line_size * width, -line_size * height, 0.0f);
	v[1].position = XMFLOAT3(-line_size * width, +line_size * height, 0.0f);
	v[2].position = XMFLOAT3(+line_size * width, +line_size * height, 0.0f);
	v[3].position = XMFLOAT3(+line_size * width, -line_size * height, 0.0f);

	v[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[2].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[3].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 0.843f);
	v[1].tex = XMFLOAT2(0.0f, 0.784f);
	v[2].tex = XMFLOAT2(0.3725f, 0.784f);
	v[3].tex = XMFLOAT2(0.3725f, 0.843f);
	//��ť
	v[4].position = XMFLOAT3(-button_size, -button_size, 0.0f);
	v[5].position = XMFLOAT3(-button_size, +button_size, 0.0f);
	v[6].position = XMFLOAT3(+button_size, +button_size, 0.0f);
	v[7].position = XMFLOAT3(+button_size, -button_size, 0.0f);

	v[4].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[5].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[6].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[7].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	v[4].tex = XMFLOAT2(0.588f, 0.9215f);
	v[5].tex = XMFLOAT2(0.588f, 0.745f);
	v[6].tex = XMFLOAT2(0.753f, 0.745f);
	v[7].tex = XMFLOAT2(0.753f, 0.9215f);
	//������
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(pancy_point) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &button_VB);
	if (FAILED(hr)) 
	{
		MessageBox(NULL,L"create vertex buffer error in GUI",L"tip",MB_OK);
		return hr;
	}
	USHORT indices[] =
	{
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 12;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &button_IB);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create vertex buffer error in GUI", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT GUI_Progress_bar::init_texture(wchar_t* texture_path)
{
	draw_position.Width  = 200;
	draw_position.Height = 200;
	draw_position.MaxDepth = 1.0f;
	draw_position.MinDepth = 0.0f;
	draw_position.TopLeftX = window_width-300;
	draw_position.TopLeftY = 20;
	HRESULT hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_path, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_ui);
	if (FAILED(hr_need)) 
	{
		MessageBox(NULL, L"create texture error in GUI", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
HRESULT GUI_Progress_bar::create(wchar_t* texture_path)
{
	HRESULT hr = init_buffer();
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = init_texture(texture_path);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void GUI_Progress_bar::release() 
{
	button_VB->Release();
	button_IB->Release();
	tex_ui->Release();
}
void GUI_Progress_bar::update() 
{
}
void GUI_Progress_bar::display() 
{
	contex_pancy->RSSetViewports(1, &draw_position);
	auto shader_gui = shader_list->get_shader_GUI();
	shader_gui->set_tex(tex_ui);
	shader_gui->set_mov_xy(XMFLOAT2(0.0f,0.0f));
	//��Ⱦ��Ļ�ռ�����ͼ
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &button_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(button_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//ѡ������·��
	shader_gui->get_technique(&tech, "draw_ui");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(12, 0, 0);
	}
	shader_gui->set_tex(NULL);
}
//�̳е�d3dע����
class d3d_pancy_1 :public d3d_pancy_basic
{
	float                    delta_need;
	HINSTANCE                hInstance;
	GUI_Progress_bar         *gui_test;
	geometry_control         *geometry_lib;       //�������
	shader_control           *shader_lib;         //shader��
	pancy_input              *user_input;          //�����������
	pancy_camera             *scene_camera;         //���������
	XMFLOAT4X4               proj_matrix;
	XMFLOAT4X4               view_matrix;
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
private:
	void show_model();
	HRESULT camera_move();
};
HRESULT d3d_pancy_1::camera_move()
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
void d3d_pancy_1::release()
{
	gui_test->release();
	geometry_lib->release();       //�������
	shader_lib->release();         //shader��
	m_renderTargetView->Release();
	swapchain->Release();
	depthStencilView->Release();
	contex_pancy->Release();
	//device_pancy->Release();
#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug *d3dDebug;
	HRESULT hr = device_pancy->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	if (d3dDebug != nullptr)            d3dDebug->Release();
#endif
	if (device_pancy != nullptr)            device_pancy->Release();
	
}
d3d_pancy_1::d3d_pancy_1(HWND hwnd_need, UINT width_need, UINT hight_need, HINSTANCE hInstance_need) :d3d_pancy_basic(hwnd_need,width_need,hight_need)
{
	hInstance = hInstance_need;
	shader_lib = new shader_control();
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, window_width*1.0f / window_hight*1.0f, 0.1f, 300.f);
	XMStoreFloat4x4(&proj_matrix, proj);
	//��Ϸʱ��
	delta_need = 0.0f;
}
HRESULT d3d_pancy_1::init_create()
{

	check_init = init(wind_hwnd, wind_width, wind_hight);
	if (check_init == false) 
	{
		MessageBox(0, L"create d3dx11 failed", L"tip", MB_OK);
		return E_FAIL;
	}

	scene_camera = new pancy_camera(device_pancy, window_width, window_hight);
	user_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	geometry_lib = new geometry_control(device_pancy, contex_pancy);
	HRESULT hr = shader_lib->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader failed", L"tip", MB_OK);
		return hr;
	}
	hr = geometry_lib->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create geometry list failed", L"tip", MB_OK);
		return hr;
	}

	gui_test = new GUI_Progress_bar(device_pancy, contex_pancy, user_input, shader_lib);
	hr = gui_test->create(L"dxutcontrols.dds");
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void d3d_pancy_1::update()
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~���³��������~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	//����castel����任
	auto* model_list = geometry_lib->get_model_list();
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	model_list->update_geometry_byname("model_castel", world_matrix, 0.0f);
	//light
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_decay(0.0f, 0.1f, 0.0f);
	pancy_light_basic light_data;
	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay;
	light_data.range = 150.0f;
	light_data.position = XMFLOAT3(0.0f, 5.0f, 0.0f);
	light_data.light_type.x = point_light;
	light_data.light_type.y = shadow_none;
	shader_test->set_light(light_data, 0);
	return;
}
void d3d_pancy_1::display()
{
	//��ʼ��
	XMVECTORF32 color = { 0.0f,0.0f,0.0f,1.0f };
	contex_pancy->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	show_model();
	gui_test->display();
	contex_pancy->RSSetViewports(1,&viewPort);
	//��������Ļ
	HRESULT hr = swapchain->Present(0, 0);
	int a = 0;
}
void d3d_pancy_1::show_model() 
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//������Ĵ��(����)����
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	//������Ĺ�������
	auto* model_castel = model_castel_pack->get_geometry_data();
	//ѡ������·��
	ID3DX11EffectTechnique *teque_need, *teque_normal;
	shader_test->get_technique(&teque_need, "draw_withtexture");
	shader_test->get_technique(&teque_normal, "draw_withtexturenormal");
	//����
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//�趨����任
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	shader_test->set_trans_world(&world_matrix);
	//�趨�ܱ任
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	for (int i = 0; i < model_castel->get_meshnum(); ++i)
	{
		//�����趨
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
		model_castel->draw_part(i);
	}
}
//endl
class engine_windows_main
{
	HWND         hwnd;                                                  //ָ��windows��ľ����
	MSG          msg;                                                   //�洢��Ϣ�Ľṹ��
	WNDCLASS     wndclass;
	int          viewport_width;
	int          viewport_height;
	HINSTANCE    hInstance;
	HINSTANCE    hPrevInstance;
	PSTR         szCmdLine;
	int          iCmdShow;
public:
	engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height);
	HRESULT game_create();
	HRESULT game_loop();
	WPARAM game_end();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
LRESULT CALLBACK engine_windows_main::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:                // ���̰�����Ϣ
		if (wParam == VK_ESCAPE)    // ESC��
			DestroyWindow(hwnd);    // ���ٴ���, ������һ��WM_DESTROY��Ϣ
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
engine_windows_main::engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height)
{
	hwnd = NULL;
	hInstance = hInstance_need;
	hPrevInstance = hPrevInstance_need;
	szCmdLine = szCmdLine_need;
	iCmdShow = iCmdShow_need;
	viewport_width = width;
	viewport_height = height;
}
HRESULT engine_windows_main::game_create() 
{
	wndclass.style = CS_HREDRAW | CS_VREDRAW;                   //����������ͣ��˴�������ֱ��ˮƽƽ�ƻ��ߴ�С�ı�ʱʱ��ˢ�£���msdnԭ�Ľ��ܣ�Redraws the entire window if a movement or size adjustment changes the width of the client area.
	wndclass.lpfnWndProc = WndProc;                                   //ȷ�����ڵĻص������������ڻ��windows�Ļص���Ϣʱ���ڴ�����Ϣ�ĺ�����
	wndclass.cbClsExtra = 0;                                         //Ϊ������ĩβ���������ֽڡ�
	wndclass.cbWndExtra = 0;                                         //Ϊ�������ʵ��ĩβ���������ֽڡ�
	wndclass.hInstance = hInstance;                                 //�����ô�����Ĵ��ڵľ����
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);          //�������ͼ������
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);              //������Ĺ������
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);     //������ı�����ˢ�����
	wndclass.lpszMenuName = NULL;                                      //������Ĳ˵���
	wndclass.lpszClassName = TEXT("pancystar_engine");                                 //����������ơ�

	if (!RegisterClass(&wndclass))                                      //ע�ᴰ���ࡣ
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			TEXT("pancystar_engine"), MB_ICONERROR);
		return E_FAIL;
	}
	RECT R = { 0, 0, window_width, window_hight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	hwnd = CreateWindow(TEXT("pancystar_engine"), // window class name�����������õĴ���������֡�
		TEXT("pancystar_engine"), // window caption��Ҫ�����Ĵ��ڵı��⡣
		WS_OVERLAPPEDWINDOW,        // window style��Ҫ�����Ĵ��ڵ����ͣ�����ʹ�õ���һ��ӵ�б�׼������״�����ͣ������˱��⣬ϵͳ�˵��������С���ȣ���
		CW_USEDEFAULT,              // initial x position���ڵĳ�ʼλ��ˮƽ���ꡣ
		CW_USEDEFAULT,              // initial y position���ڵĳ�ʼλ�ô�ֱ���ꡣ
		width,               // initial x size���ڵ�ˮƽλ�ô�С��
		height,               // initial y size���ڵĴ�ֱλ�ô�С��
		NULL,                       // parent window handle�丸���ڵľ����
		NULL,                       // window menu handle��˵��ľ����
		hInstance,                  // program instance handle���ڳ����ʵ�������
		NULL);                     // creation parameters�������ڵ�ָ��
	if (hwnd == NULL) 
	{
		return E_FAIL;
	}
	ShowWindow(hwnd, SW_SHOW);   // ��������ʾ�������ϡ�
	UpdateWindow(hwnd);           // ˢ��һ�鴰�ڣ�ֱ��ˢ�£�����windows��Ϣѭ����������ʾ����
	return S_OK;
}
HRESULT engine_windows_main::game_loop()
{
	//��Ϸѭ��
	ZeroMemory(&msg, sizeof(msg));
	d3d_pancy_1 *d3d11_test = new d3d_pancy_1(hwnd, viewport_width, viewport_height, hInstance);
	if (d3d11_test->init_create() == S_OK)
	{
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);//��Ϣת��
				DispatchMessage(&msg);//��Ϣ���ݸ����ڹ��̺���
				d3d11_test->update();
				d3d11_test->display();
			}
			else
			{
				d3d11_test->update();
				d3d11_test->display();
			}
		}
		d3d11_test->release();
	}
	
	return S_OK;
}
WPARAM engine_windows_main::game_end()
{
	return msg.wParam;
}

//windows���������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	engine_windows_main *engine_main = new engine_windows_main(hInstance, hPrevInstance, szCmdLine, iCmdShow, window_width, window_hight);
	engine_main->game_create();
	engine_main->game_loop();
	return engine_main->game_end();
}


/*
	data 2016.7.7�� pancystar engine��Ƴ���������Ⱦ����ǰ�����Ⱦ���ߡ�
	data 2016.7.10��pancystar engine��Ƴ���������Ⱦ����ĳ�����Ⱦ����ܹ���
	data 2016.7.12��pancystar engine��Ƴ���ǰ�����Ⱦ������ɡ�
	data 2016.7.15��pancystar engine��Ƴ���ģ������ϵͳ��ɡ�
	data 2016.7.15: pancystar engine��Ƴ���mesh�ϲ�ϵͳ��ɡ�
	data 2016.7.19: pancystar engine��Ƴ���shadow mapϵͳ��ɡ�
	data 2016.7.21: pancystar engine��Ƴ���shadow mapϵͳ�������ɣ����״̬�༰normalmap��
	data 2016.7.26: pancystar engine��Ƴ���ssaoϵͳ��ɣ����������Ļ������������ɡ�
	data 2016.7.27: pancystar engine��Ƴ���cubemappingϵͳ��ɡ�
	data 2016.7.28: pancystar engine��Ƴ����޸���ssao��һЩ����
	data 2016.7.30: pancystar engine��Ƴ����Ż���ģ������ϵͳ���ϲ�ͬһ����������Դ���ȼ���draw call��
	created by pancy_star
*/
#include"geometry.h"
#include"pancy_d3d11_basic.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_scene_design.h"
#include"pancy_DXrenderstate.h"
#include"pancy_ssao.h"




//�̳е�d3dע����
class d3d_pancy_1 :public d3d_pancy_basic
{
	scene_root               *first_scene_test;
	shader_control           *shader_list;         //shader��
	time_count               time_need;            //ʱ�ӿ���
	pancy_input              *test_input;          //�����������
    pancy_camera             *test_camera;         //���������
	pancy_renderstate        *render_state;        //��Ⱦ��ʽ
	float                    time_game;            //��Ϸʱ��
	float                    delta_need;
	HINSTANCE                hInstance;
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
};
void d3d_pancy_1::release()
{
	render_state->release();
	shader_list->release();
	first_scene_test->release();
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
	time_need.reset();
	time_game = 0.0f;
	shader_list = new shader_control();
	hInstance = hInstance_need;
	render_state = NULL;
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
	test_camera = new pancy_camera(device_pancy, window_width, window_hight);
	test_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	render_state = new pancy_renderstate(device_pancy,contex_pancy);
	HRESULT hr;
	hr = shader_list->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"create shader failed",L"tip",MB_OK);
		return hr;
	}
	hr = render_state->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create render state failed", L"tip", MB_OK);
		return hr;
	}

	first_scene_test = new scene_engine_test(this,device_pancy,contex_pancy, render_state,test_input, test_camera, shader_list, wind_width, wind_hight);
	hr = first_scene_test->scene_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create scene failed", L"tip", MB_OK);
		return hr;
	}
	
	return S_OK;
}
void d3d_pancy_1::update()
{
	float delta_time = time_need.get_delta() * 20;
	time_game += delta_time;
	delta_need += XM_PI*0.5f*delta_time;
	time_need.refresh();
	first_scene_test->update(delta_time);
	return;
}
void d3d_pancy_1::display()
{
	//��ʼ��
	XMVECTORF32 color = { 0.75f,0.75f,0.75f,1.0f };
	contex_pancy->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	first_scene_test->display();
	//��������Ļ
	HRESULT hr = swapchain->Present(0, 0);
	int a = 0;
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


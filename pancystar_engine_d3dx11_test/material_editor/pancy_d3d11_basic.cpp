#include"pancy_d3d11_basic.h"
d3d_pancy_basic::d3d_pancy_basic(HWND hwnd_need, UINT width_need, UINT hight_need)
{
	device_pancy = NULL;
	contex_pancy = NULL;
	swapchain = NULL;
	//m_renderTargetView = NULL;
	//depthStencilView = NULL;
	//posttreatment_RTV = NULL;
	wind_hwnd  = hwnd_need;
	wind_width = width_need;
	wind_hight = hight_need;
}
HRESULT d3d_pancy_basic::init(HWND hwnd_need, UINT width_need, UINT hight_need)
{
	UINT create_flag = 0;
	bool if_use_HIGHCARD = true;
	//debug��ʽ��ѡ�񷵻ص�����Ϣ
#if defined(DEBUG) || defined(_DEBUG)
	create_flag = D3D11_CREATE_DEVICE_DEBUG;
	if_use_HIGHCARD = false;
#endif
	//~~~~~~~~~~~~~~~~~����d3d�豸�Լ�d3d�豸������
	HRESULT hr;
	if (if_use_HIGHCARD == true)
	{
		std::vector<IDXGIAdapter1*> vAdapters;
		IDXGIFactory1* factory;
		CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
		IDXGIAdapter1 * pAdapter = 0;
		DXGI_ADAPTER_DESC1 pancy_star;
		UINT i = 0;
		//HRESULT check_hardweare;
		while (factory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}
		vAdapters[1]->GetDesc1(&pancy_star);
		hr = D3D11CreateDevice(vAdapters[1], D3D_DRIVER_TYPE_UNKNOWN, NULL, create_flag, 0, 0, D3D11_SDK_VERSION, &device_pancy, &leave_need, &contex_pancy);
		int a = 0;
	}
	else
	{
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_flag, 0, 0, D3D11_SDK_VERSION, &device_pancy, &leave_need, &contex_pancy);
	}
	if (FAILED(hr))
	{
		MessageBox(hwnd_need, L"d3d�豸����ʧ��", L"��ʾ", MB_OK);
		return false;
	}
	if (leave_need != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(hwnd_need, L"�Կ���֧��d3d11", L"��ʾ", MB_OK);
		return false;
	}
	//return true;
	//~~~~~~~~~~~~~~~~~~����Ƿ�֧���ı������
	device_pancy->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &check_4x_msaa);
	if (create_flag == D3D11_CREATE_DEVICE_DEBUG)
	{
		assert(check_4x_msaa > 0);
	}
	//~~~~~~~~~~~~~~~~~~���ý������Ļ�������ʽ��Ϣ
	DXGI_SWAP_CHAIN_DESC swapchain_format;//���建�����ṹ��
	swapchain_format.BufferDesc.Width = width_need;
	swapchain_format.BufferDesc.Height = hight_need;
	swapchain_format.BufferDesc.RefreshRate.Numerator = 60;
	swapchain_format.BufferDesc.RefreshRate.Denominator = 1;
	swapchain_format.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_format.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchain_format.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//���û������Ŀ������Ϣ
	if (check_4x_msaa > 0)
	{
		swapchain_format.SampleDesc.Count = 4;
		swapchain_format.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		swapchain_format.SampleDesc.Count = 1;
		swapchain_format.SampleDesc.Quality = 0;
	}
	swapchain_format.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//��Ⱦ��ʽΪ��Ⱦ��������
	swapchain_format.BufferCount = 1;                              //��ʹ��һ����������Ϊ��̨����
	swapchain_format.OutputWindow = hwnd_need;                     //����Ĵ��ھ��
	swapchain_format.Windowed = true;                              //����ģʽ
	swapchain_format.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;        //����Ⱦ����ѡ�����Ч�ķ���
	swapchain_format.Flags = 0;                                    //�Ƿ�ȫ������
	//~~~~~~~~~~~~~~~~~~~~~~~����������
	IDXGIDevice *pDxgiDevice(NULL);
	HRESULT hr1 = device_pancy->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice));
	IDXGIAdapter *pDxgiAdapter(NULL);
	hr1 = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter));
	IDXGIFactory *pDxgiFactory(NULL);
	hr1 = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));
	hr1 = pDxgiFactory->CreateSwapChain(device_pancy, &swapchain_format, &swapchain);

	render_state = new pancy_renderstate(device_pancy, contex_pancy);
	hr = render_state->create(width_need, hight_need, swapchain);
	if (FAILED(hr))
	{
		return false;
	}
	//�ͷŽӿ�  
	pDxgiFactory->Release();
	pDxgiAdapter->Release();
	pDxgiDevice->Release();
	return true;
}
d3d_pancy_basic::~d3d_pancy_basic()
{
	//safe_release(device_pancy);
	//safe_release(contex_pancy);
	//safe_release(swapchain);
	//safe_release(m_renderTargetView);
	//safe_release(depthStencilView);
}
/*
bool d3d_pancy_basic::change_size() 
{

	//~~~~~~~~~~~~~~~~~~~~~~~������ͼ��Դ
	ID3D11Texture2D *backBuffer = NULL;
	//��ȡ�󻺳�����ַ 
	HRESULT hr;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	//������ͼ
	if (FAILED(hr))
	{
		MessageBox(wind_hwnd, L"change size error", L"tip", MB_OK);
		return false;
	}
	hr = device_pancy->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
	D3D11_TEXTURE2D_DESC check;
	backBuffer->GetDesc(&check);


	if (FAILED(hr))
	{
		MessageBox(wind_hwnd, L"change size error", L"tip", MB_OK);
		return false;
	}
	//�ͷź󻺳�������  
	backBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~������ȼ�ģ�建����
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = wind_width;
	dsDesc.Height = wind_hight;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	if (check_4x_msaa > 0)
	{
		dsDesc.SampleDesc.Count = 4;
		dsDesc.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
	}
	ID3D11Texture2D* depthStencilBuffer;
	device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	device_pancy->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~����ͼ��Ϣ����Ⱦ����
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~�����ӿڱ任��Ϣ
	viewPort.Width = static_cast<FLOAT>(wind_width);
	viewPort.Height = static_cast<FLOAT>(wind_hight);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	contex_pancy->RSSetViewports(1, &viewPort);
	return true;
}
void d3d_pancy_basic::restore_rendertarget()
{
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void d3d_pancy_basic::set_posttreatment_rendertarget() 
{
	contex_pancy->OMSetRenderTargets(1, &posttreatment_RTV, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
*/
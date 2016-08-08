#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<vector>
#include<d3dx11effect.h>
#include<directxmath.h>
#pragma comment ( lib, "D3D11.lib")
#pragma comment ( lib, "dxgi.lib")
using namespace DirectX;
#define window_width 800
#define window_hight 600
class d3d_pancy_basic
{
protected:
	HWND wind_hwnd;
	UINT wind_width;
	UINT wind_hight;
	UINT                    check_4x_msaa;       //�ͷ�֧���ı������
	ID3D11Device            *device_pancy;       //d3d�豸
	ID3D11DeviceContext     *contex_pancy;       //�豸������
	D3D_FEATURE_LEVEL       leave_need;          //�Կ�֧�ֵ�directx�ȼ�
	IDXGISwapChain          *swapchain;          //��������Ϣ	
    ID3D11RenderTargetView  *m_renderTargetView; //��ͼ����
	D3D11_VIEWPORT          viewPort;            //�ӿ���Ϣ
    ID3D11DepthStencilView  *depthStencilView;   //��������Ϣ

	ID3D11RenderTargetView  *posttreatment_RTV;  //���ں������ȾĿ��
public:
	d3d_pancy_basic(HWND wind_hwnd,UINT wind_width,UINT wind_hight);
	~d3d_pancy_basic();
	virtual void update()   = 0;
	virtual void display()  = 0;
	virtual void release()  = 0;
	bool change_size();
	void restore_rendertarget();
	void set_posttreatment_rendertarget();
protected:	
	HRESULT init(HWND wind_hwnd,UINT wind_width,UINT wind_hight);
	
	template<class T> 
	void safe_release(T t)
	{
		if(t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
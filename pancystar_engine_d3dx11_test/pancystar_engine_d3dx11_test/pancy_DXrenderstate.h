#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<vector>
#include<d3dx11effect.h>
#include<directxmath.h>
using namespace DirectX;
class pancy_renderstate 
{
	ID3D11Device           *device_pancy;     //d3d�豸
	ID3D11DeviceContext    *contex_pancy;     //�豸������
	//��Ⱦģʽ
	ID3D11RasterizerState  *CULL_front;       //��������
	ID3D11RasterizerState  *CULL_none;       //��������
	ID3D11BlendState       *blend_common;     //��׼alpha���
	ID3D11BlendState       *AlphaToCoverageBS;
	//��ȾĿ��
	ID3D11RenderTargetView  *m_renderTargetView; //��ͼ����
	D3D11_VIEWPORT          viewPort;            //�ӿ���Ϣ
	ID3D11DepthStencilView  *depthStencilView;   //��������Ϣ
	ID3D11RenderTargetView  *posttreatment_RTV;  //���ں������ȾĿ��
	ID3D11RenderTargetView  *reflectmask_RTV;    //�����ڱη������ȾĿ��
	IDXGISwapChain          *swapchain;
public:
	pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT create(int wind_width, int wind_height,IDXGISwapChain *swapchain_need);
	void release();
	ID3D11RasterizerState  *get_CULL_front_rs() { return CULL_front; };
	ID3D11RasterizerState  *get_CULL_none_rs() { return CULL_none; };
	ID3D11BlendState        *get_blend_common() { return blend_common; };
	ID3D11BlendState        *get_blend_tocoverage() { return AlphaToCoverageBS; };
	ID3D11RenderTargetView  *get_postrendertarget() { return posttreatment_RTV; };
	ID3D11RenderTargetView  *get_reflectrendertarget() { return reflectmask_RTV; };
	ID3D11RenderTargetView  *get_basicrendertarget() { return m_renderTargetView; };
	void clear_basicrendertarget();
	void clear_posttreatmentcrendertarget();
	void clear_reflectrendertarget();
	void restore_rendertarget();
	void restore_rendertarget(ID3D11DepthStencilView *depthStenci_need);
	void set_posttreatment_rendertarget();
	void set_posttreatment_rendertarget(ID3D11DepthStencilView *depthStenci_need);
	void set_posttreatment_reflect_rendertarget();
	HRESULT change_size(int wind_width,int wind_height);
private:
	HRESULT init_CULL_front();
	HRESULT init_CULL_none();
	HRESULT init_common_blend();
	HRESULT init_alpha_to_coverage();
};

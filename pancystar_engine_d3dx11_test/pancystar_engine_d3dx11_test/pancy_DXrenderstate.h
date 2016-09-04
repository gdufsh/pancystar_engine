#pragma once
#include"pancy_d3d11_basic.h"
class pancy_renderstate 
{
	ID3D11Device           *device_pancy;     //d3d�豸
	ID3D11DeviceContext    *contex_pancy;     //�豸������
	//��Ⱦģʽ
	ID3D11RasterizerState  *CULL_front;       //��������
	ID3D11RasterizerState  *CULL_none;       //��������
	ID3D11BlendState       *blend_common;     //��׼alpha���
public:
	pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT create();
	void release();
	ID3D11RasterizerState  *get_CULL_front_rs() { return CULL_front; };
	ID3D11RasterizerState  *get_CULL_none_rs() { return CULL_none; };
	ID3D11BlendState        *get_blend_common() { return blend_common; };
private:
	HRESULT init_CULL_front();
	HRESULT init_CULL_none();
	HRESULT init_common_blend();
};

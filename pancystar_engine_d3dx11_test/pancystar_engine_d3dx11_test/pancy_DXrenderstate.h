#pragma once
#include"pancy_d3d11_basic.h"
class pancy_renderstate 
{
	ID3D11Device           *device_pancy;     //d3d�豸
	ID3D11DeviceContext    *contex_pancy;     //�豸������
	//��Ⱦģʽ
	ID3D11RasterizerState  *CULL_front;       //��������
public:
	pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT create();
	void release();
	ID3D11RasterizerState  *get_CULL_front_rs() { return CULL_front; };
private:
	HRESULT init_CULL_front();
};

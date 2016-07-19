#pragma once
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
#include<directxmath.h>
#include<string.h>
#include<stdlib.h>
#include<Dinput.h>
#include<iostream>
using namespace DirectX;

#define CameraForFPS 0
#define CameraForFly 1
class pancy_camera
{
    int window_weight;
    int window_height;
	ID3D11Device *device_d3d; //ȷ�����任��d3d�豸�ӿ�
	XMFLOAT3 camera_right;    //����������ҷ�������
	XMFLOAT3 camera_look;     //������Ĺ۲췽������
	XMFLOAT3 camera_up;       //����������Ϸ�������
	XMFLOAT3 camera_position; //�����������λ������
public:
	pancy_camera(ID3D11Device *device_need,int weight,int height);
	void rotation_right(float angle);                    //��������������ת
	void rotation_up(float angle);                       //��������������ת
	void rotation_look(float angle);                     //���Ź۲�������ת
	void walk_front(float distance);                     //�������ǰƽ��
	void walk_right(float distance);                     //���������ƽ��
	void walk_up(float distance);                        //���������ƽ��
	void count_view_matrix(XMMATRIX* view_matrix);       //����ȡ������
	void get_view_position(XMFLOAT3 *view_pos);
	void get_view_direct(XMFLOAT3 *view_direct);
private:
    

};
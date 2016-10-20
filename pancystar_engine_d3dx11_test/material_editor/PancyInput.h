#pragma once
#include <windows.h>
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
#include<directxmath.h>
#include<string.h>
#include<stdlib.h>
#include<Dinput.h>
#include<iostream>
#pragma comment(lib, "Dinput8.lib") 
using namespace std;
class pancy_input
{
	ID3D11Device              *pancy_d3ddevice;                              //Direct3D���豸�ӿ�
	LPDIRECTINPUT8             pancy_dinput;                                 //DirectInput���豸�ӿ�    
	LPDIRECTINPUTDEVICE8       dinput_keyboard;                              //�����豸�ӿ�
	LPDIRECTINPUTDEVICE8       dinput_mouse;                                 //����豸�ӿ�
	char                       key_buffer[256];                              //���̰�����Ϣ�Ļ���
	DIMOUSESTATE               mouse_buffer;                                 //��������Ϣ�Ļ���
public:
	pancy_input(HWND hwnd,ID3D11Device *d3d_device,HINSTANCE hinst);         //���캯��
	~pancy_input();                                                          //��������
	void  get_input();                                                       //��ȡ��������
	bool  check_keyboard(int key_value);                                     //�������ϵ�ĳ�����������
	bool  check_mouseDown(int mouse_value);                                  //�������ϵ�ĳ�����������
	float MouseMove_X();                                                     //��ȡ�����x����ƶ���
	float MouseMove_Y();                                                     //��ȡ�����y����ƶ���
	float MouseMove_Z();                                                     //��ȡ�����z����ƶ���
private:
	void dinput_clear(HWND hwnd,DWORD keyboardCoopFlags, DWORD mouseCoopFlags);//��ʼ������
};
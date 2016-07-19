#include"PancyInput.h"
pancy_input::pancy_input(HWND hwnd, ID3D11Device *d3d_device,HINSTANCE hinst)
{
	pancy_d3ddevice = d3d_device;//��ȡD3D�豸�ӿ�
	pancy_dinput = NULL;
	DirectInput8Create(hinst,DIRECTINPUT_HEADER_VERSION,IID_IDirectInput8,(void**)&pancy_dinput,NULL);//��ȡDirectInput�豸
	dinput_clear(hwnd,(DISCL_FOREGROUND | DISCL_NONEXCLUSIVE),(DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));//�������̼����
}
pancy_input::~pancy_input()
{
	if(dinput_keyboard != NULL)
	{
		dinput_keyboard->Unacquire();
		dinput_keyboard->Release();
	}
	if(dinput_mouse != NULL)
	{
		dinput_mouse->Unacquire();
		dinput_mouse->Release();
	}
	if(pancy_dinput != NULL)
	{
		pancy_dinput->Release();
	}
}
void pancy_input::dinput_clear(HWND hwnd,DWORD keyboardCoopFlags, DWORD mouseCoopFlags)
{
	//���������豸
	pancy_dinput->CreateDevice(GUID_SysKeyboard,&dinput_keyboard,NULL);
	dinput_keyboard->SetDataFormat(&c_dfDIKeyboard);//�����豸�����ݸ�ʽ
	dinput_keyboard->SetCooperativeLevel(hwnd,keyboardCoopFlags);//�����豸�Ķ�ռ�ȼ�
	dinput_keyboard->Acquire();//��ȡ�豸�Ŀ���Ȩ
	dinput_keyboard->Poll();//������ѯ
	//��������豸
	pancy_dinput->CreateDevice(GUID_SysMouse,&dinput_mouse,NULL);
	dinput_mouse->SetDataFormat(&c_dfDIMouse);//�����豸�����ݸ�ʽ
	dinput_mouse->SetCooperativeLevel(hwnd,mouseCoopFlags);//�����豸�Ķ�ռ�ȼ�
	dinput_mouse->Acquire();//��ȡ�豸�Ŀ���Ȩ
	dinput_mouse->Poll();//������ѯ
}
void pancy_input::get_input()
{
	//��ȡ�����Ϣ
	ZeroMemory(&mouse_buffer,sizeof(mouse_buffer));
	while(true)
	{
		dinput_mouse->Poll();
		dinput_mouse->Acquire();
		HRESULT hr;
		if(SUCCEEDED(dinput_mouse->GetDeviceState(sizeof(mouse_buffer),(LPVOID)&mouse_buffer)))
		{
			break;
		}
		else
		{
			hr = dinput_mouse->GetDeviceState(sizeof(mouse_buffer),(LPVOID)&mouse_buffer);
		}
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)
		{
			break;
		};
		if (FAILED(dinput_mouse->Acquire()))
		{
			break;
		};
	}
	//��ȡ������Ϣ
	ZeroMemory(&key_buffer,sizeof(key_buffer));
	while(true)
	{
		dinput_keyboard->Poll();
		dinput_keyboard->Acquire();
		HRESULT hr;
		if(SUCCEEDED(dinput_keyboard->GetDeviceState(sizeof(key_buffer),(LPVOID)&key_buffer)))
		{
			break;
		}
		else
		{
			hr = dinput_keyboard->GetDeviceState(sizeof(key_buffer),(LPVOID)&key_buffer);
		}
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)
		{
			break;
		};
		if (FAILED(dinput_keyboard->Acquire()))
		{
			break;
		};
	}
	
}
bool pancy_input::check_keyboard(int key_value)
{
	if(key_buffer[key_value] & 0x80)
	{
		return true;
	}
	return false;
}
bool pancy_input::check_mouseDown(int mouse_value)
{
	if((mouse_buffer.rgbButtons[mouse_value]&0x80) != 0)
	{
		return true;
	}
	return false;
}
float pancy_input::MouseMove_X()
{
	return (float)mouse_buffer.lX;
}
float pancy_input::MouseMove_Y()
{
	return (float)mouse_buffer.lY;
}
float pancy_input::MouseMove_Z()
{
	return (float)mouse_buffer.lZ;
}
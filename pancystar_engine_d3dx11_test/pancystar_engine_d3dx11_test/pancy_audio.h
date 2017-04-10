#pragma once
#include <conio.h>
#include <Windows.h>
#include <vector>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <fmod.hpp>
#include <iostream>
#include <fstream>
#include<time.h>
#include<directxmath.h>
using namespace DirectX;
struct sound_list
{
	char file_name[128];
	FMOD::Sound *sounds;
	int music_ID;
};
struct sound_list_withbuffer
{
	char buffer_name[128];
	FMOD::Sound *sound_now;
	char *data_need;
	bool if_empty;
	int music_ID;
	int bufflength;
	bool if_playing;//�������б��
};
class FMOD_basic
{
	FMOD_RESULT result;                                   //��������ֵ����
	FMOD::System* system;                                 //fmod�豸ָ��
	std::vector<sound_list> sounds;                       //���ļ��������Ⱦ��Ч
	FMOD::Channel  *channels[700];                         //ͨ����Ⱦ��Чͨ��
	FMOD_VECTOR  istenerpos;                              //�趨�����ߵ�λ��
public:
	//���캯��
	FMOD_basic();
	//���º���
	HRESULT main_update();
	//���ļ��м�������
	HRESULT load_music(char music_name[], FMOD_CREATESOUNDEXINFO *exinfo, FMOD_MODE mode, int &sound_ID);
	//���ڴ��м�������
	HRESULT load_music_from_memory(char* data_mem, int datalength);
	//����������λ��
	HRESULT set_lisener(XMFLOAT3  location, XMFLOAT3 forward, XMFLOAT3 up, XMFLOAT3 speed);
	//����ĳ�����ֵ�λ��
	HRESULT set_music_place(int channel_ID, XMFLOAT3 location, XMFLOAT3 speed);
	//��ʼ����һ����ͨ����
	HRESULT play_sound(int sound_ID, int channel_ID);
	HRESULT play_sound_single(int sound_ID, int channel_ID);
	//��ͣһ��ͨ��
	HRESULT pause_sound(int channel_ID);
	//����һ��ͨ��
	HRESULT start_sound(int channel_ID);
	//����ͨ������Ҫ��
	void set_sound_priority(int channel_ID, int priority);
	//���鵱ǰ�����Ƿ����
	bool check_if_virtual(int channel_ID);
	//��������
	HRESULT set_value(int channel_ID,float value);
	//ע��ϵͳ
	HRESULT init_system();
};
#include"light_define.fx"
cbuffer perframe
{
	//pancy_light_dir    dir_light_need[10];    //�����Դ
	//pancy_light_point  point_light_need[5];   //���Դ
	//pancy_light_spot   spot_light_need[15];   //�۹�ƹ�Դ
	float3             position_view;         //�ӵ�λ��
	int num_dir;
	int num_point;
	int num_spot;
};
Texture2D  texture_normal;  //��ȷ��߼�¼
Texture2D  texture_diffuse; //���������
Texture2D  texture_specular;//���淴�����
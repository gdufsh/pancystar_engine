/*
	data 2016.7.7 ��pancystar engine��Ƴ���������Ⱦ����ǰ�����Ⱦ���ߡ�
	data 2016.7.10��pancystar engine��Ƴ���������Ⱦ����ĳ�����Ⱦ����ܹ���
	data 2016.7.12��pancystar engine��Ƴ���ǰ�����Ⱦ������ɡ�
	data 2016.7.15��pancystar engine��Ƴ���ģ������ϵͳ��ɡ�
	data 2016.7.15: pancystar engine��Ƴ���mesh�ϲ�ϵͳ��ɡ�
	data 2016.7.19: pancystar engine��Ƴ���shadow mapϵͳ��ɡ�
	data 2016.7.21: pancystar engine��Ƴ���shadow mapϵͳ�������ɣ����״̬�༰normalmap��
	data 2016.7.26: pancystar engine��Ƴ���ssaoϵͳ��ɣ����������Ļ������������ɡ�
	data 2016.7.27: pancystar engine��Ƴ���cubemappingϵͳ��ɡ�
	data 2016.7.28: pancystar engine��Ƴ����޸���ssao��һЩ����
	data 2016.7.30: pancystar engine��Ƴ����Ż���ģ������ϵͳ���ϲ�ͬһ����������Դ���ȼ���draw call��
	data 2016.8.1 : pancystar engine��Ƴ����Ż���ssao��Ϊpass1������4*MSAA����ݡ�
	data 2016.8.2 : pancystar engine��Ƴ��������HDR��pass1��Ϊ����ͼ�εõ���ƽ�����ȡ�
	data 2016.8.4 : pancystar engine��Ƴ��������HDR��pass2��Ϊ����ͼ�εõ��˸���������
	created by pancy_star
*/
#include"geometry.h"
#include"pancy_d3d11_basic.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_scene_design.h"
#include"pancy_DXrenderstate.h"
#include"pancy_ssao.h"

//�����࣬����Ⱦ��ɺ�������Ļ�ռ�ͼ��
class render_posttreatment 
{
	//ȫ���ı���
	ID3D11Buffer                *HDRMap_VB;            //aoͼƬ���㻺����
	ID3D11Buffer                *HDRMap_IB;            //aoͼƬ����������
	//����ԭ��Ⱦ���ֵĹ������
	ID3D11Device                *device_pancy;
	ID3D11DeviceContext         *contex_pancy;
	ID3D11RenderTargetView      *rendertarget_input; //ǰһ������ɵ���ȾĿ��
	shader_control              *shader_list;        //shader��
	int                         width, height;       //��Ļ���
	//�ڲ�����
	ID3D11UnorderedAccessView   *UAV_HDR_mid;        //HDR�Ļ������������м����
	ID3D11UnorderedAccessView   *UAV_HDR_final;      //HDR�Ļ����������ڴ洢���
	ID3D11ShaderResourceView    *SRV_HDR_use;        //HDR���벿�֣�Ҫ����Ļ����ת���ɷǿ���ݵ�����

	ID3D11ShaderResourceView    *SRV_HDR_save;       //HDR�߹�洢������Ⱦ��Դ�����߹���д洢��
	ID3D11RenderTargetView      *RTV_HDR_save;       //HDR�߹�洢������ȾĿ�꣬���߹���д洢��

	ID3D11ShaderResourceView    *SRV_HDR_blur1;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur1;       //HDR�߹�ģ����ȾĿ�ꡣ
	ID3D11ShaderResourceView    *SRV_HDR_blur2;       //HDR�߹�ģ����Ⱦ��Դ��
	ID3D11RenderTargetView      *RTV_HDR_blur2;       //HDR�߹�ģ����ȾĿ�ꡣ
	
	D3D11_VIEWPORT              render_viewport;      //�ӿ���Ϣ
	ID3D11Buffer*               CPU_read_buffer;
	float                       average_light;
	d3d_pancy_basic             *root_state_need;
public:
	render_posttreatment(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need,int width_need,int height_need,d3d_pancy_basic *rec_rootstate);
	HRESULT create();
	void release();
	HRESULT display();
private:
	HRESULT init_buffer();
	HRESULT init_texture();
	HRESULT CreateCPUaccessBuf(int size_need);
	HRESULT build_fullscreen_picturebuff();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);

};
HRESULT render_posttreatment::CreateCPUaccessBuf(int size_need)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = size_need*sizeof(float);        //���㻺��Ĵ�С
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	//bufferDesc.MiscFlags = 0;
	HRESULT hr = device_pancy->CreateBuffer(&bufferDesc, NULL, &CPU_read_buffer);

	if (FAILED(hr)) 
	{
		MessageBox(0,L"create CPU access read buffer error in HDR pass",L"tip",MB_OK);
		return hr;
	}
	return S_OK;
}
render_posttreatment::render_posttreatment(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, d3d_pancy_basic *rec_rootstate)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	rendertarget_input = rendertarget_need;
	shader_list = shaderlist_need;
	width = width_need;
	height = height_need;
	root_state_need = rec_rootstate;
}
HRESULT render_posttreatment::create() 
{
	HRESULT hr;
	hr = build_fullscreen_picturebuff();
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = init_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_buffer();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT render_posttreatment::init_texture() 
{
	//����������Դ
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* HDR_averagetex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_averagetex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_averagetex texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_averagetex, 0, &SRV_HDR_use);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_averagetex->Release();
	//�����߹�洢��Դ
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	ID3D11Texture2D* HDR_preblurtex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_preblurtex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_preblurtex, 0, &SRV_HDR_save);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_preblurtex,0,&RTV_HDR_save);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_preblurtex->Release();
	//������˹ģ��������Դ
	texDesc.Width = width / 4.0f;
	texDesc.Height = height / 4.0f;
	ID3D11Texture2D* HDR_blurtex1 = 0, *HDR_blurtex2 = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_blurtex1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_blurtex2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_blurtex1, 0, &SRV_HDR_blur1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_blurtex2, 0, &SRV_HDR_blur2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_blurtex1, 0, &RTV_HDR_blur1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_blurtex2, 0, &RTV_HDR_blur2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_blurtex1->Release();
	HDR_blurtex2->Release();
	//ע���ӿ���Ϣ
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(width / 4.0f);
	render_viewport.Height = static_cast<float>(height / 4.0f);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	return S_OK;
}
HRESULT render_posttreatment::init_buffer() 
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~����������~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Buffer *buffer_HDR_mid;
	ID3D11Buffer *buffer_HDR_final;
	int width_rec = width / 4;
	int height_rec = height / 4;
	//�趨�߳�����
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//�趨�߳��������
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}
	//����HDR�ĵ�һ��buffer������1/16�����²���
	D3D11_BUFFER_DESC HDR_buffer_desc;
	HDR_buffer_desc.Usage =     D3D11_USAGE_DEFAULT;            //ͨ������
	HDR_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//��������Ϊuav+srv
	HDR_buffer_desc.ByteWidth = width_rec * height_rec*sizeof(float);        //���㻺��Ĵ�С
	HDR_buffer_desc.CPUAccessFlags = 0;
	HDR_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	HDR_buffer_desc.StructureByteStride = sizeof(float);
	hr = device_pancy->CreateBuffer(&HDR_buffer_desc,NULL, &buffer_HDR_mid);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	//�����ڶ���buffer������1/256�����²���
	int buffer_num = width_rec * height_rec;
	if (buffer_num % 256 != 0) 
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else 
	{
		buffer_num = buffer_num / 256;
	}
	HDR_buffer_desc.ByteWidth = buffer_num * sizeof(float);
	hr = device_pancy->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	hr = CreateCPUaccessBuf(buffer_num);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	//������һ��UAV�������һ��buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = width_rec * height_rec;

	hr = device_pancy->CreateUnorderedAccessView(buffer_HDR_mid, &DescUAV, &UAV_HDR_mid);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR UAV", L"tip", MB_OK);
		return hr;
	}
	
	//�����ڶ���UAV������ڶ���buffer
	DescUAV.Buffer.NumElements = buffer_num;
	hr = device_pancy->CreateUnorderedAccessView(buffer_HDR_final, &DescUAV, &UAV_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR UAV", L"tip", MB_OK);
		return hr;
	}
	buffer_HDR_mid->Release();
	buffer_HDR_final->Release();

	return S_OK;
}
void render_posttreatment::release()
{
	SRV_HDR_use->Release();
	UAV_HDR_mid->Release();
	UAV_HDR_final->Release();
	CPU_read_buffer->Release();
	HDRMap_VB->Release();
	HDRMap_IB->Release();
	SRV_HDR_save->Release();
	RTV_HDR_save->Release();
	SRV_HDR_blur1->Release();
	RTV_HDR_blur1->Release();
	SRV_HDR_blur2->Release();
	RTV_HDR_blur2->Release();
}
HRESULT render_posttreatment::display() 
{
	HRESULT hr;
	//pass1����ƽ������
	auto shader_test = shader_list->get_shader_HDRaverage();
	ID3D11Resource * normalDepthTex = 0;
	ID3D11Resource * normalDepthTex_singlesample = 0;
	rendertarget_input->GetResource(&normalDepthTex);
	SRV_HDR_use->GetResource(&normalDepthTex_singlesample);
	//�����ز�������ת�����Ƕ�������
	contex_pancy->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	SRV_HDR_use->Release();
	SRV_HDR_use = NULL;
	device_pancy->CreateShaderResourceView(normalDepthTex_singlesample, 0, &SRV_HDR_use);
	normalDepthTex->Release();
	normalDepthTex_singlesample->Release();

	hr = shader_test->set_compute_tex(SRV_HDR_use);
	if (FAILED(hr)) 
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	hr = shader_test->set_compute_buffer(UAV_HDR_mid, UAV_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	int width_rec = width / 4;
	int height_rec = height / 4;
	//�����߳�����
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//�����߳��������
	if (width_rec % 16 != 0) 
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}
	
	//�����߳�����
	int buffer_num = width_rec * height_rec;
	//�����߳�������
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	hr = shader_test->set_piccturerange(width, height, width_rec * height_rec, height_rec);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	shader_test->dispatch(width_rec/16,height_rec/16, buffer_num);

	ID3D11Resource *check_rec;
	UAV_HDR_final->GetResource(&check_rec);
	ID3D11Buffer* pDebugBuffer = NULL;
	D3D11_BOX box;
	box.left = 0;
	box.right = sizeof(float) * buffer_num;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	contex_pancy->CopySubresourceRegion(CPU_read_buffer, 0, 0, 0, 0, check_rec, 0, &box);
	D3D11_MAPPED_SUBRESOURCE vertex_resource;
	hr = contex_pancy->Map(CPU_read_buffer, 0, D3D11_MAP_READ, 0, &vertex_resource);
	if (FAILED(hr))
	{
		MessageBox(0, L"get vertex buffer map error", L"tip", MB_OK);
		return hr;
	}
	float vertex[1000];
	memcpy(static_cast<void*>(vertex), vertex_resource.pData, buffer_num * sizeof(float));
	average_light = 0.0f;
	for (int i = 0; i < buffer_num; ++i) 
	{
		average_light += vertex[i];
	}
	average_light /= width * height;
	average_light = exp(average_light);
	check_rec->Release();
	contex_pancy->Unmap(CPU_read_buffer, 0);
	shader_test->set_compute_buffer(NULL,NULL);
	shader_test->set_compute_tex(NULL);
	//pass2�߹�ģ����Ԥ����
	auto shader_test2 = shader_list->get_shader_HDRpreblur();
	shader_test2->set_buffer_input(SRV_HDR_use);
	shader_test2->set_lum_message(average_light,1.0f,1.5f,0.38f);
	
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };

	ID3D11RenderTargetView* renderTargets[1] = { RTV_HDR_blur1 };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(RTV_HDR_blur1, black);
	contex_pancy->RSSetViewports(1, &render_viewport);
	
	UINT stride = sizeof(HDR_fullscreen);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	shader_test2->get_technique(&tech, "draw_preblur");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_test2->set_buffer_input(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	//pass3�߹�ģ��
	basic_blur(SRV_HDR_blur1, RTV_HDR_blur2,true);
	basic_blur(SRV_HDR_blur2, RTV_HDR_blur1,false);
	//pass4���պϳ�
	root_state_need->restore_rendertarget();
	auto shader_test3 = shader_list->get_shader_HDRfinal();
	shader_test3->set_tex_resource(SRV_HDR_use, SRV_HDR_blur1);
	shader_test3->set_lum_message(average_light, 1.0f, 1.5f, 0.38f);
	
	stride = sizeof(HDR_fullscreen);
	offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	shader_test3->get_technique(&tech, "draw_HDRfinal");
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_test3->set_tex_resource(NULL,NULL);
	return S_OK;
}
void render_posttreatment::basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output,bool if_horz) 
{
	//������ȾĿ��
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(output, black);
	contex_pancy->RSSetViewports(1, &render_viewport);
	auto shader_blur = shader_list->get_shader_HDRblur();
	shader_blur->set_image_size(1.0f / (width / 4.0f), 1.0f / (height / 4.0f));
	shader_blur->set_tex_resource(input);

	UINT stride = sizeof(HDR_fullscreen);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	if (if_horz == true) 
	{
		shader_blur->get_technique(&tech, "HorzBlur");
	}
	else 
	{
		shader_blur->get_technique(&tech, "VertBlur");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	//tech->GetPassByIndex(0)->Apply(0, contex_pancy);
}
HRESULT render_posttreatment::build_fullscreen_picturebuff()
{
	HRESULT hr;
	HDR_fullscreen v[4];
	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 1.0f);
	v[1].tex = XMFLOAT2(0.0f, 0.0f);
	v[2].tex = XMFLOAT2(1.0f, 0.0f);
	v[3].tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(HDR_fullscreen) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &HDRMap_VB);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"error when create HDR full screen buffer",L"tip",MB_OK);
		return hr;
	}
	USHORT indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	hr =device_pancy->CreateBuffer(&ibd, &iinitData, &HDRMap_IB);
	if (FAILED(hr))
	{
		MessageBox(0, L"error when create HDR full screen buffer", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}

//�̳е�d3dע����
class d3d_pancy_1 :public d3d_pancy_basic
{
	scene_root               *first_scene_test;
	shader_control           *shader_list;         //shader��
	time_count               time_need;            //ʱ�ӿ���
	pancy_input              *test_input;          //�����������
    pancy_camera             *test_camera;         //���������
	pancy_renderstate        *render_state;        //��Ⱦ��ʽ
	float                    time_game;            //��Ϸʱ��
	float                    delta_need;
	HINSTANCE                hInstance;
	render_posttreatment     *posttreat_scene;     //��������
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
};
void d3d_pancy_1::release()
{
	render_state->release();
	shader_list->release();
	first_scene_test->release();
	m_renderTargetView->Release();
	swapchain->Release();
	depthStencilView->Release();
	contex_pancy->Release();
	posttreat_scene->release();
	safe_release(posttreatment_RTV);
	//device_pancy->Release();
#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug *d3dDebug;
	HRESULT hr = device_pancy->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	if (d3dDebug != nullptr)            d3dDebug->Release();
#endif
	if (device_pancy != nullptr)            device_pancy->Release();
	
}
d3d_pancy_1::d3d_pancy_1(HWND hwnd_need, UINT width_need, UINT hight_need, HINSTANCE hInstance_need) :d3d_pancy_basic(hwnd_need,width_need,hight_need)
{
	time_need.reset();
	time_game = 0.0f;
	shader_list = new shader_control();
	posttreat_scene = NULL;
	hInstance = hInstance_need;
	render_state = NULL;
	//��Ϸʱ��
	delta_need = 0.0f;
}
HRESULT d3d_pancy_1::init_create()
{
	HRESULT hr;
	hr = init(wind_hwnd, wind_width, wind_hight);
	if (FAILED(hr)) 
	{
		MessageBox(0, L"create d3dx11 failed", L"tip", MB_OK);
		return E_FAIL;
	}
	test_camera = new pancy_camera(device_pancy, window_width, window_hight);
	test_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	render_state = new pancy_renderstate(device_pancy,contex_pancy);
	hr = shader_list->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"create shader failed",L"tip",MB_OK);
		return hr;
	}
	hr = render_state->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create render state failed", L"tip", MB_OK);
		return hr;
	}

	first_scene_test = new scene_engine_test(this,device_pancy,contex_pancy, render_state,test_input, test_camera, shader_list, wind_width, wind_hight);
	hr = first_scene_test->scene_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create scene failed", L"tip", MB_OK);
		return hr;
	}
	posttreat_scene = new render_posttreatment(device_pancy, contex_pancy, posttreatment_RTV, shader_list, wind_width, wind_hight,this);
	hr = posttreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_class failed", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void d3d_pancy_1::update()
{
	float delta_time = time_need.get_delta() * 20;
	time_game += delta_time;
	delta_need += XM_PI*0.5f*delta_time;
	time_need.refresh();
	first_scene_test->update(delta_time);
	return;
}
void d3d_pancy_1::display()
{
	//��ʼ��
	XMVECTORF32 color = { 0.75f,0.75f,0.75f,1.0f };
	contex_pancy->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	set_posttreatment_rendertarget();
	first_scene_test->display();
	restore_rendertarget();
	posttreat_scene->display();
	//��������Ļ
	HRESULT hr = swapchain->Present(0, 0);
	int a = 0;
}
//endl
class engine_windows_main
{
	HWND         hwnd;                                                  //ָ��windows��ľ����
	MSG          msg;                                                   //�洢��Ϣ�Ľṹ��
	WNDCLASS     wndclass;
	int          viewport_width;
	int          viewport_height;
	HINSTANCE    hInstance;
	HINSTANCE    hPrevInstance;
	PSTR         szCmdLine;
	int          iCmdShow;
public:
	engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height);
	HRESULT game_create();
	HRESULT game_loop();
	WPARAM game_end();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
LRESULT CALLBACK engine_windows_main::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:                // ���̰�����Ϣ
		if (wParam == VK_ESCAPE)    // ESC��
			DestroyWindow(hwnd);    // ���ٴ���, ������һ��WM_DESTROY��Ϣ
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
engine_windows_main::engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height)
{
	hwnd = NULL;
	hInstance = hInstance_need;
	hPrevInstance = hPrevInstance_need;
	szCmdLine = szCmdLine_need;
	iCmdShow = iCmdShow_need;
	viewport_width = width;
	viewport_height = height;
}
HRESULT engine_windows_main::game_create() 
{
	wndclass.style = CS_HREDRAW | CS_VREDRAW;                   //����������ͣ��˴�������ֱ��ˮƽƽ�ƻ��ߴ�С�ı�ʱʱ��ˢ�£���msdnԭ�Ľ��ܣ�Redraws the entire window if a movement or size adjustment changes the width of the client area.
	wndclass.lpfnWndProc = WndProc;                                   //ȷ�����ڵĻص������������ڻ��windows�Ļص���Ϣʱ���ڴ�����Ϣ�ĺ�����
	wndclass.cbClsExtra = 0;                                         //Ϊ������ĩβ���������ֽڡ�
	wndclass.cbWndExtra = 0;                                         //Ϊ�������ʵ��ĩβ���������ֽڡ�
	wndclass.hInstance = hInstance;                                 //�����ô�����Ĵ��ڵľ����
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);          //�������ͼ������
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);              //������Ĺ������
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);     //������ı�����ˢ�����
	wndclass.lpszMenuName = NULL;                                      //������Ĳ˵���
	wndclass.lpszClassName = TEXT("pancystar_engine");                                 //����������ơ�

	if (!RegisterClass(&wndclass))                                      //ע�ᴰ���ࡣ
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			TEXT("pancystar_engine"), MB_ICONERROR);
		return E_FAIL;
	}
	RECT R = { 0, 0, window_width, window_hight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	hwnd = CreateWindow(TEXT("pancystar_engine"), // window class name�����������õĴ���������֡�
		TEXT("pancystar_engine"), // window caption��Ҫ�����Ĵ��ڵı��⡣
		WS_OVERLAPPEDWINDOW,        // window style��Ҫ�����Ĵ��ڵ����ͣ�����ʹ�õ���һ��ӵ�б�׼������״�����ͣ������˱��⣬ϵͳ�˵��������С���ȣ���
		CW_USEDEFAULT,              // initial x position���ڵĳ�ʼλ��ˮƽ���ꡣ
		CW_USEDEFAULT,              // initial y position���ڵĳ�ʼλ�ô�ֱ���ꡣ
		width,               // initial x size���ڵ�ˮƽλ�ô�С��
		height,               // initial y size���ڵĴ�ֱλ�ô�С��
		NULL,                       // parent window handle�丸���ڵľ����
		NULL,                       // window menu handle��˵��ľ����
		hInstance,                  // program instance handle���ڳ����ʵ�������
		NULL);                     // creation parameters�������ڵ�ָ��
	if (hwnd == NULL) 
	{
		return E_FAIL;
	}
	ShowWindow(hwnd, SW_SHOW);   // ��������ʾ�������ϡ�
	UpdateWindow(hwnd);           // ˢ��һ�鴰�ڣ�ֱ��ˢ�£�����windows��Ϣѭ����������ʾ����
	return S_OK;
}
HRESULT engine_windows_main::game_loop()
{
	//��Ϸѭ��
	ZeroMemory(&msg, sizeof(msg));
	d3d_pancy_1 *d3d11_test = new d3d_pancy_1(hwnd, viewport_width, viewport_height, hInstance);
	if (d3d11_test->init_create() == S_OK)
	{
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);//��Ϣת��
				DispatchMessage(&msg);//��Ϣ���ݸ����ڹ��̺���
				d3d11_test->update();
				d3d11_test->display();
			}
			else
			{
				d3d11_test->update();
				d3d11_test->display();
			}
		}
		d3d11_test->release();
	}
	
	return S_OK;
}
WPARAM engine_windows_main::game_end()
{
	return msg.wParam;
}

//windows���������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	engine_windows_main *engine_main = new engine_windows_main(hInstance, hPrevInstance, szCmdLine, iCmdShow, window_width, window_hight);
	engine_main->game_create();
	engine_main->game_loop();
	return engine_main->game_end();
}


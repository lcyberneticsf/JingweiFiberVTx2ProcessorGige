#include "CLineaCtrl.h"

extern int numCamera ;
extern void* m_latestBuffer ;
//extern X_VIEW_HANDLE  View ;
extern aq::gigeVCamera::MY_CONTEXT m_context ;
extern pthread_t  tid;

CLineaCtrl::CLineaCtrl()
{

}
CLineaCtrl::~CLineaCtrl()
{

}

//��ʼ�������
bool CLineaCtrl::Init(int nIndex)
{
	DalsaLineaCamera.set_camera_options();
	DalsaLineaCamera.discover_camera();
	DalsaLineaCamera.choose_camera(numCamera);
	return true;
}

//���ճ�ʼ������
void CLineaCtrl::Destory()
{
	return;
}
bool CLineaCtrl::PrepareGrab()
{
	return true;
}
//��ʼ����ɼ�
bool CLineaCtrl::StartGrab()
{
	return true;
}

//ֹͣ����ɼ�
bool CLineaCtrl::StopGrab()
{
	return true;
}
//�����������״̬��1���ڴ�����2���ⴥ����
void CLineaCtrl::SetCamTrigger(int nParam)
{
	return;
}
//��������ع�ʱ��
void CLineaCtrl::SetCamExposure(int nParam)
{
	return;
}

//�����������
void CLineaCtrl::SetCamGain(int nParam)
{

}

//��������ɼ�����
void CLineaCtrl::SetCamGrabSize(int nWidth, int nHeight)
{
	return;
}

//��ȡ���״̬
void CLineaCtrl::CheckCamStatus()
{
	return;
}

//�������
bool CLineaCtrl::ConnectCamera()
{
	return true;

}

//��������豸
bool CLineaCtrl::CreateObjects()
{
	return true;
}
//��������豸
bool CLineaCtrl::DestroyObjects()
{
	return true;
}

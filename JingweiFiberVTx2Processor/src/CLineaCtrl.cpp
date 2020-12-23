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

//初始化类对象
bool CLineaCtrl::Init(int nIndex)
{
	DalsaLineaCamera.set_camera_options();
	DalsaLineaCamera.discover_camera();
	DalsaLineaCamera.choose_camera(numCamera);
	return true;
}

//回收初始化对象
void CLineaCtrl::Destory()
{
	return;
}
bool CLineaCtrl::PrepareGrab()
{
	return true;
}
//开始相机采集
bool CLineaCtrl::StartGrab()
{
	return true;
}

//停止相机采集
bool CLineaCtrl::StopGrab()
{
	return true;
}
//设置相机触发状态：1，内触发；2，外触发；
void CLineaCtrl::SetCamTrigger(int nParam)
{
	return;
}
//设置相机曝光时间
void CLineaCtrl::SetCamExposure(int nParam)
{
	return;
}

//设置相机增益
void CLineaCtrl::SetCamGain(int nParam)
{

}

//设置相机采集开窗
void CLineaCtrl::SetCamGrabSize(int nWidth, int nHeight)
{
	return;
}

//获取相机状态
void CLineaCtrl::CheckCamStatus()
{
	return;
}

//连接相机
bool CLineaCtrl::ConnectCamera()
{
	return true;

}

//创造相机设备
bool CLineaCtrl::CreateObjects()
{
	return true;
}
//销毁相机设备
bool CLineaCtrl::DestroyObjects()
{
	return true;
}

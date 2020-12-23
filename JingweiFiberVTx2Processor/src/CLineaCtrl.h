/*
* ����: CLineaCtrl
* ����:����豸��,���ڶ�������豸,��ʵ���豸�ĳ�ʼ����ͼ��ɼ����������ã�
*/
#pragma once
#include "gigevcamera.h"
using namespace std;
using namespace aq;
class CLineaCtrl
{
public:
	CLineaCtrl();
	~CLineaCtrl();
protected:
	
	//virtual bool Init();
public:
	bool Init(int nIndex);
	void Destory();
	bool PrepareGrab();
	bool StartGrab();
	bool StopGrab();

	void SetCamTrigger(int nParam);
	void SetCamExposure(int nParam);
	void SetCamGain(int nParam);
	void SetCamGrabSize(int nWidth, int nHeight);

	void CheckCamStatus();
private:
	bool ConnectCamera();
	bool CreateObjects();
	bool DestroyObjects();

public:
	gigeVCamera  DalsaLineaCamera;
	//SapAcqDevice* m_pAcqDevice;
	//SapBuffer* m_pBuffers;
	//SapTransfer* m_pXfer;
};


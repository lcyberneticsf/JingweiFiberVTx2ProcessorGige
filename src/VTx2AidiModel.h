#ifndef VSEE_TX2_AIDI_MODEL_H_
#define VSEE_TX2_AIDI_MODEL_H_
#include <opencv2/opencv.hpp>
#include "communicator.h"
#include "VTx2Session.h"
#include "aidi_vision.h"

#define Xavive_NX_MODE
//#define TX2_MODE
namespace Vsee
{
	class VCameraFrame;
	class VFrameResult;


	class VTx2AidiModel
	{
	public:
		VTx2AidiModel(const char* type,const char* model);

		VTx2AidiModel(const VTx2AidiModel&) = delete;
		VTx2AidiModel& operator=(const VTx2AidiModel&) = delete;

		VFrameResult getFrameResult(const VCameraFrame& frame);
		VFrameResult getFrameResult(char* image_data, int image_width, int image_height,int channels);
		VFrameResult getFrameResult(cv::Mat &img);
		VFrameResult getFrameResult(InferFrame infer_frame);
		//void         parse_labelio(const aq::aidi::LabelIO& labelio);
		void test_json();
		

		//static aq::Communicator communitator_;

	private:
		class Impl;
		Impl* _impl;

	};
}

#endif
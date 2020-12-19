#ifndef VSEE_CAMERA_ENUM_H_
#define VSEE_CAMERA_ENUM_H_

#include <cstdint>


namespace Vsee
{
	/// 触发模式
	/// @remark 相机支持的所有触发模式
	/// @see VCamera::setTriggerMode VCameraInterface::triggerModeResult
	namespace TriggerMode
	{
		enum : std::uint8_t
		{
			Hardware = 0,
			Software = 1
		};
	}


	/// 传输模式
	/// @remark 相机支持的所有传输模式
	/// @see VCamera::setTransMode VCameraInterface::transModeResult
	namespace TransMode
	{
		enum : std::uint8_t
		{
			Disabled = 0,
			Up       = 1,
			Down     = 2,
		};
	}


	/// 帧格式
	/// @remark 相机支持的所有图像格式
	/// @see VCamera::setFrameFormat VCameraInterface::frameFormatResult
	namespace FrameFormat
	{
		enum : std::uint8_t
		{
			Gray    = 0x10,
			R       = 0x11,
			G       = 0x12,
			B       = 0x13,
			IR      = 0x14,

			RG      = 0x21,
			RB      = 0x22,
			GB      = 0x23,

			RC      = 0x24,
			GC      = 0x25,
			BC      = 0x26,
			GrayC   = 0x27,

			RGB     = 0x31,      //RGBRGBRGB
			RGB888  = 0x32,      //RRRGGGBBB
			RGBC    = 0x41,      //RRRGGGBBBC
			RGBI    = 0x42,      //RGBRGBRGBC

		};
	}

}

#endif
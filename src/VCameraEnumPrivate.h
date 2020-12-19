#ifndef VSEE_CAMERA_ENUM_PRIVATE_H_
#define VSEE_CAMERA_ENUM_PRIVATE_H_

#include <cstdint>
#include <cstddef>


namespace Vsee
{
	/// 端口号
	namespace PortNumber
	{
		enum : std::uint16_t
		{
			UDP       = 9710,
			TCP       = 9711,
			PRC       = 9712,
		};
	}


	/// 令牌类型 \n
	/// Get为获取，Set为设置，Ret为返回
	namespace Token
	{
		enum : std::uint32_t
		{
			Invalid = 0,


			//相机相关

			GetInformation = 1,     //UDP
			SetInformation = 2,
			RetInformation = 3,

			GetServerVersion = 4,
			RetServerVersion = 5,

			GetFPGAVersion   = 6,
			RetFPGAVersion   = 7,

			GetCoreVersion   = 8,
			RetCoreVersion   = 9,

			GetSensorVersion  = 10,
			RetSensorVersion  = 11,

			GetPortVersion    = 12,
			RetPortVersion    = 13,


			//帧相关

			GetFrameWidth     = 14,
			SetFrameWidth     = 15,
			RetFrameWidth     = 16,

			GetFrameHeight    = 17,
			SetFrameHeight    = 18,
			RetFrameHeight    = 19,

			GetFrameWidthBegin  = 20,
			SetFrameWidthBegin  = 21,
			RetFrameWidthBegin  = 22,

			GetFrameHeightBegin = 23,
			SetFrameHeightBegin = 24,
			RetFrameHeightBegin = 25,

			GetFrameFormat      = 26,
			SetFrameFormat      = 27,
			RetFrameFormat      = 28,

			GetFrameExpTime     = 29,
			SetFrameExpTime     = 30,
			RetFrameExpTime     = 31,

			GetFrameExpDelay    = 32,
			SetFrameExpDelay    = 33,
			RetFrameExpDelay    = 34,

			GetFrameGain        = 35,
			SetFrameGain        = 36,
			RetFrameGain        = 37,

			GetFrameSimFormat   = 38,
			SetFrameSimFormat   = 39,
			RetFrameSimFormat   = 40,

			GetFrameSimParam    = 41,
			SetFrameSimParam    = 42,
			RetFrameSimParam    = 43,

			GetFrameInterval    = 44,
			RetFrameInterval    = 45,
			SetFrameInterval    = 46,

			GetTriggerMode      = 47,
			SetTriggerMode      = 48,
			RetTriggerMode      = 49,

			SoftwareTrigger     = 50,
			RetFrame            = 51,

			GetDebugData        = 52,
			SetDebugData        = 53,
			RetDebugData        = 54,

			GetRegisterValue    = 55,
			SetRegisterValue    = 56,
			SetRegisterValueTwo = 59,
			SetRegisterValueMore = 60,
			RetRegisterValue    = 57,

			SetRegisterValueMask    = 58,
			SetRegisterValueTwoMask = 73,
			SetRegisterValueMoreMask = 74,

			GetMultipleRegisterValue = 61,
			SetMultipleRegisterValue = 62,
			RetMultipleRegisterValue = 63,

			ReportError          = 72,


			//extension function

			GetExtensionInfo     = 64,
			RetExtensionInfo     = 65,

			GetTransMode      = 66,
			SetTransMode      = 67,
			RetTransMode      = 68,

			PushFrame         = 69,
			ProcessFrame      = 70,


			//processor

			PushProcessResult = 71,

		};
	}
}

#endif
#ifndef VSEE_CAMERA_FRAME_H_
#define VSEE_CAMERA_FRAME_H_

#include "VCameraMessage.h"

#include <utility>


namespace Vsee
{


	/// @brief 图像头
	/// @remark 仅用于指针访问
	/// @see ::None
	class VCameraCoreApi VCameraFrameHead final
	{
	public:

		/// @brief 为空 \n
		/// 判断此帧头是否为空
		/// @return bool 是否为空
		bool empty() const;


		/// @brief 时间戳 \n
		/// 读取帧时间戳
		/// @return std::uint64_t 时间值 单位ms
		std::uint64_t timestamp() const;


		/// @brief 帧序 \n
		/// 读取帧序
		/// @return std::uint64_t 帧序号数值
		std::uint64_t sequence() const;


		/// @brief 宽度 \n
		/// 读取帧宽度
		/// @return std::uint16_t 帧宽度数值
		std::uint16_t width() const;


		/// @brief 高度 \n
		/// 读取帧高度
		/// @return std::uint16_t 帧高度数值
		std::uint16_t height() const;


		/// @brief 格式 \n
		/// 读取帧数据格式
		/// @return std::uint8 图像数据格式值
		/// @see Vsee::FrameFormat
		std::uint8_t  format() const;


		/// @brief 数据量 \n
		/// 读取帧数据总量
		/// @return std::uint32_t 数据量值
		std::uint32_t payload() const;


		/// @brief 数据 \n
		/// 原始数据
		/// @return const char* 帧数据指针
		/// @remark 等同于帧指针
		const char* data() const;
		

		/// @brief 数据长度 \n
		/// 原始数据长度
		/// @return std::size_t 数据长度
		std::size_t dataSize() const;


	private:

		/// @remark 删除构造函数使此类不可被创建
		VCameraFrameHead() = delete;

	};  //class VCameraFrameHead



	/// @brief 相机图像帧 \n
	/// 通过此类访问一帧图像
	class VCameraCoreApi VCameraFrame
	{
	public:


		/// @brief 默认构造函数 \n
		/// 构造一个空图像实例
		VCameraFrame() = default;


		/// @brief 从消息构造 \n
		/// 从一个消息构造图像
		/// @remark 必须确保消息是图像消息，否则行为未定义
		VCameraFrame(VCameraMessage &&msg);


		/// @brief 生成下传图像 \n
		/// 生成可下传的图像
		/// @param [in] data 图像数据
		/// @param [in] data_len 图像数据长度
		/// @return VCameraFrame 生成的图像
		/// @remark 图像的大小必须与相机当前设置对应!!
		/// @see VCamera::pushFrame
		static VCameraFrame makePushFrame(const char* data, std::size_t data_len);


		/// @brief 为空 \n
		/// 判断此帧是否为空
		/// @return bool 是否为空
		bool empty() const;


		/// @brief 时间戳 \n
		/// 获取此帧的时间戳
		/// @return std::uint64_t 时间戳数值
		std::uint64_t timestamp() const;


		/// @brief 帧序 \n
		/// 获取此帧的帧序
		/// @return std::uint64_t 帧序数值
		std::uint64_t sequence() const;


		/// @brief 宽度 \n
		/// 获取此帧图像宽度
		/// @return std::uint16_t 宽度值
		std::uint16_t width() const;


		/// @brief 高度 \n
		/// 获取此帧图像高度
		/// @return std::uint16_t 高度值
		std::uint16_t height() const;


		/// @brief 格式 \n
		/// 获取此帧图像格式
		/// @return std::uint8_t 图像格式值
		std::uint8_t format() const;


		/// @brief 图像数据大小 \n
		/// 获取此帧图像数据大小
		/// @return std::uint32_t 图像大小值
		std::uint32_t payload() const;


		/// @brief 图像数据 \n
		/// 获取此帧图像数据
		/// @return const char* 图像数据指针
		/// @remark 获取到的为裸数据，像素排列方式依图像格式而定
		const char* data() const;


		/// @brief 获取图像附加扩展数据
		/// @param [in] template typename ExtData 扩展数据类型
		/// @return VCameraFrameExtData 扩展附加信息
		/// @remark 服务端支持的某些扩展可能会在图像帧中附加额外信息， \n
		/// 此为访问附加信息的接口，传入的类型由扩展插件提供
		template <typename ExtData>
		const ExtData* extData() const;


		/// @brief 转换为报文 \n
		/// 将此帧转换为报文，某些情况下可能有用
		/// @return const VCameraMessage& 报文常量引用
		/// @remark 返回内部成员常量引用
		const VCameraMessage& toMessage() const;


	private:

		/// 内部访问用
		const VCameraFrameHead* head() const;

	private:

		/// 原始报文
		VCameraMessage _message;

	};  //class VCameraFrame



	inline VCameraFrame::VCameraFrame(VCameraMessage &&msg) : _message(std::move(msg)) {}



	inline bool VCameraFrame::empty() const { return _message.empty(); }

	inline std::uint64_t VCameraFrame::timestamp() const { return head()->timestamp(); }
	inline std::uint64_t VCameraFrame::sequence() const { return head()->sequence(); }
	inline std::uint16_t VCameraFrame::width() const { return head()->width(); }
	inline std::uint16_t VCameraFrame::height() const { return head()->height(); }
	inline std::uint8_t VCameraFrame::format() const { return head()->format(); }
	inline std::uint32_t VCameraFrame::payload() const { return head()->payload(); }

	inline const char * VCameraFrame::data() const { return _message.data(); }
	inline const VCameraMessage & VCameraFrame::toMessage() const { return _message; }

	inline const VCameraFrameHead * VCameraFrame::head() const { return (VCameraFrameHead*)_message.data(); }

	template <typename ExtData>
	const ExtData* VCameraFrame::extData() const { return (const ExtData*)(data() + payload()); }
}

#endif
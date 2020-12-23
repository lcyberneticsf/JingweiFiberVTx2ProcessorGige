#ifndef VSEE_CAMERA_MESSAGE_H_
#define VSEE_CAMERA_MESSAGE_H_

#include "VCameraCoreApi.h"

#include <cstdint>
#include <cstddef>


namespace Vsee
{
	class VCameraMessagePrivate;



	/// @brief 相机消息 \n
	/// 相机报文类，用于读取报文或生成报文
	class VCameraCoreApi VCameraMessage final
	{
	public:


		/// @brief 默认构造函数 \n
		/// 构造一个空消息
		VCameraMessage();


		/// @brief 析构函数
		~VCameraMessage();


		/// @brief 拷贝构造函数
		VCameraMessage(const VCameraMessage& other);


		/// @brief 拷贝赋值运算符
		VCameraMessage& operator=(const VCameraMessage& rhs);


		/// @brief 移动构造函数
		VCameraMessage(VCameraMessage&&) noexcept;


		/// @brief 移动赋值运算符
		VCameraMessage& operator=(VCameraMessage&&) noexcept;


		/// @brief 从字符串构造 \n
		/// 与指定字符串构造消息
		/// @param [in] raw 指定字符串
		/// @param [in] size 指定字符串长度
		/// @remark 必须确保字符串符合格式，否则行为未定义!!
		VCameraMessage(const char* raw, std::size_t size);


		/// @brief 根据字段构造 \n
		/// 根据提供的字段构造消息
		/// @param [in] token 令牌
		/// @param [in] data 数据指针
		/// @param [in] data_len 数据长度
		/// @param [in] gen_crc 是否校验
		VCameraMessage(std::uint32_t token, const char *data = nullptr, std::size_t len = 0, bool gen_crc = true);


		/// @brief 判断缓冲区内是否含有前导码
		/// @param [in] buf 指定缓冲区
		/// @param [in] len 指定缓冲区长度
		/// @return const char* 前导码第一次出现的地方
		static const char* findLeadCode(const char* buf, std::size_t len);


		/// @brief 检查下次读取数量
		/// @param [in] buf 已读取到的数据
		/// @param [in] buf_len 已读取到的数据长度
		/// @return std::size_t 下次读取数量，已有一包报文时返回0
		/// @todo 重构此部分内容，太不清晰且容易误用
		/// @see VCameraMessage::load
		static std::size_t nextLoad(const char* buf, std::size_t buf_len);


		/// @brief 从缓冲区读取报文
		/// @param [in] buffer 指定缓冲区
		/// @return VCameraMessage 读取到的报文，可能为空
		/// @todo 重构此部分内容，太不清晰且容易误用 
		static VCameraMessage load(const char* buf, std::size_t size);


		/// @brief 类型转换为bool \n
		/// 转换为bool判断是否为空
		/// @see VCameraMessage::empty
		operator bool() const;


		/// @brief 检测是否为空 \n
		/// 检测此消息是否为空
		/// @return bool 是否为空
		bool empty() const;


		/// @brief 检查CRC \n
		/// 检查此消息内的crc是否合法
		/// @return bool 检测结果
		bool checkCrc() const;


		/// @brief 获取长度 \n
		/// 获取长度字段
		/// @return std::uint32_t 长度字段
		/// @remark 注意，此为长度字段而不是消息长度! 若报文为空则行为未定义
		/// @see VCameraMessage::toString
		std::uint32_t length() const;


		/// @brief 设置长度字段 \n
		/// 设置消息长度字段
		/// @param [in] len 新的长度
		/// @param [in] up 是否更新crc
		/// @return void
		/// @remark 由于更新crc比较耗时，所以可以选择是否更新，可以多次设置操作一次更新。若报文为空则行为未定义
		/// @see VCameraMessage::setToken VCameraMessage::setData VCameraMessage::setCrc
		void setLength(std::uint32_t len, bool up = true);


		/// @brief 获取令牌字段 \n
		/// 获取令牌字段
		/// @return std::uint32_t 当前报文内的令牌
		/// @remark 若报文为空则行为未定义
		/// @see Vsee::Token
		std::uint32_t token() const;


		/// @brief 设置令牌字段 \n
		/// 设置消息令牌字段
		/// @param [in] tok 令牌
		/// @param [in] up 是否更新crc
		/// @return void
		/// @remark 若报文为空则行为未定义
		void setToken(std::uint32_t tok, bool up = true);


		/// @brief 获取数据字段 \n
		/// 获取消息数据字段
		/// @return const char* 数据指针
		/// @remark 若报文为空则行为未定义
		const char* data() const;


		/// @brief 设置数据字段 \n
		/// 设置消息数据字段内容
		/// @param [in] d 数据
		/// @param [in] len 数据长度
		/// @param [in] up 是否更新crc
		/// @return void
		/// @remark 若报文为空则行为未定义
		void setData(const char* d, std::size_t len, bool up = true);


		/// @brief 获取数据字段内容长度 \n
		/// 获取消息数据字段内容的长度
		/// @return std::size_t 报文长度
		/// @remark 若报文为空则行为未定义
		std::size_t dataSize() const;


		/// @brief 获取CRC字段 \n
		/// 获取消息CRC字段数值
		/// @return std::uint32_t CRC32值
		/// @remark 若报文为空则行为未定义
		std::uint32_t crc() const;


		/// @brief 设置CRC字段 \n
		/// 设置消息CRC字段值
		/// @param [in] crc 新的crc值
		/// @return void
		/// @remark 若报文为空则行为未定义
		void setCrc(std::uint32_t crc);


		/// @brief 更新CRC字段 \n
		/// 根据消息现有各字段数据重新生成CRC字段
		/// @return void
		/// @remark 若报文为空则行为未定义
		void updateCrc();


		/// @brief 获取原始字节 \n
		/// 获取原始字节指针
		/// @return const char* 原始字节指针
		const char* bytes() const;


		/// @brief 获取原始字节数
		/// @return std::size_t 字节数
		std::size_t byteSize() const;

//	private:

		VCameraMessage(VCameraMessagePrivate* pri);
		VCameraMessagePrivate* _private;

	};  //class VCameraMessage



	inline VCameraMessage::VCameraMessage() : _private(nullptr) {}
	inline VCameraMessage::VCameraMessage(VCameraMessagePrivate* pri) : _private(pri) {}

	inline VCameraMessage::operator bool() const { return !empty(); }

}

#endif

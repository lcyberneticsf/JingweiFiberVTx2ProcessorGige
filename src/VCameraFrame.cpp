#include "VCameraFrame.h"
#include "VCameraEnum.h"

#include "VCameraEnumPrivate.h"


namespace Vsee
{
	namespace
	{
		template <typename T>
		inline T toValue(const char *p) { return *(T*)p; }

		inline std::size_t getHeadCount(const VCameraFrameHead *head)
		{
			using namespace FrameFormat;
			switch (head->format())
			{
			case RGB:
			case RGBC:
				return 3;
			case RGB888:
				return 1;
			}

			return 0;
		}

	}

	template <std::size_t N>
	struct HeadData
	{
		struct Width
		{
			std::uint8_t _low[N], _high[N];

			operator std::uint16_t() const
			{
				union { std::uint8_t _bytes[2]; std::uint16_t _value; } helper;
				helper._bytes[0] = _low[0];
				helper._bytes[1] = _high[0];
				return helper._value;
			}

		};

		using Height = Width;

		struct Timestamp
		{
			std::uint8_t _0[N], _1[N], _2[N], _3[N], _4[N], _5[N], _6[N], _7[N];

			operator std::uint64_t() const
			{
				union { std::uint8_t _bytes[8]; std::uint64_t _value; } helper;
				helper._bytes[0] = _0[0];
				helper._bytes[1] = _1[0];
				helper._bytes[2] = _2[0];
				helper._bytes[3] = _3[0];
				helper._bytes[4] = _4[0];
				helper._bytes[5] = _5[0];
				helper._bytes[6] = _6[0];
				helper._bytes[7] = _7[0];
				return helper._value;
			}
		};


		using Sequence = Timestamp;


		std::uint8_t _format;
		std::uint8_t _unused[N * 3 + N - 1];

		Width _width;
		Height _height;

		Timestamp _timestamp;
		Sequence  _sequence;

	};

	bool VCameraFrameHead::empty() const { return !this; }
	const char * VCameraFrameHead::data() const { return reinterpret_cast<const char*>(this); }


	std::uint64_t VCameraFrameHead::timestamp() const
	{
		if (empty())
			return 0;

		switch (getHeadCount(this))
		{
		case 0:
			return 0;
		case 1:
			return ((const HeadData<1>*)this)->_timestamp;
		case 2:
			return ((const HeadData<2>*)this)->_timestamp;
		case 3:
			return ((const HeadData<3>*)this)->_timestamp;
		}

		return 0;
	}

	std::uint64_t VCameraFrameHead::sequence() const
	{
		if (empty())
			return 0;

		switch (getHeadCount(this))
		{
		case 0:
			return 0;
		case 1:
			return ((const HeadData<1>*)this)->_sequence;
		case 2:
			return ((const HeadData<2>*)this)->_sequence;
		case 3:
			return ((const HeadData<3>*)this)->_sequence;
		}

		return 0;
	}

	std::uint16_t VCameraFrameHead::width() const
	{
		if (empty())
			return 0;

		switch (getHeadCount(this))
		{
		case 0:
			return 0;
		case 1:
			return ((const HeadData<1>*)this)->_width;
		case 2:
			return ((const HeadData<2>*)this)->_width;
		case 3:
			return ((const HeadData<3>*)this)->_width;
		}

		return 0;
	}

	std::uint16_t VCameraFrameHead::height() const
	{
		if (empty())
			return 0;

		switch (getHeadCount(this))
		{
		case 0:
			return 0;
		case 1:
			return ((const HeadData<1>*)this)->_height;
		case 2:
			return ((const HeadData<2>*)this)->_height;
		case 3:
			return ((const HeadData<3>*)this)->_height;
		}

		return 0;
	}

	std::uint32_t VCameraFrameHead::payload() const
	{
		std::uint32_t res = width() * height();

		using namespace FrameFormat;
		switch (format())
		{
		case RGB:
		case RGB888:
		case RGBC:
			return res * 3;
		}

		return res;
	}

	std::uint8_t VCameraFrameHead::format() const
	{
		if (empty())
			return 0;

		return ((const HeadData<1>*)this)->_format;
	}

	std::size_t VCameraFrameHead::dataSize() const
	{
		std::uint32_t res = width() * height();

		using namespace FrameFormat;
		switch (format())
		{
		case RGB:
		case RGB888:
			return res * 3;
		case RGBC:
			return res * 3 + 2048;
		}

		return res;
	}

	VCameraFrame VCameraFrame::makePushFrame(const char * data, std::size_t data_len)
	{
		return VCameraFrame(VCameraMessage(Token::PushFrame, data, data_len, false));
	}

}

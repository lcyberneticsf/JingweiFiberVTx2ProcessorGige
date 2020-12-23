#include "VCameraProcessorTypes.h"


namespace Vsee
{
	class VFrameResult::Private
	{
	public:
		Private
		(
			std::uint32_t sequence,
			std::uint8_t v1,
			std::uint8_t v2,
			std::uint8_t v3,
			std::uint8_t v4,
			std::uint8_t v5,
			std::uint8_t v6,
			std::uint8_t v7,
			std::uint8_t v8
		) 
		:
			_sequece(sequence),
			_1(v1),
			_2(v2),
			_3(v3),
			_4(v4),
			_5(v5),
			_6(v6),
			_7(v7),
			_8(v8)
		{

		}

		std::uint32_t sequence() const { return _sequece; }
		void setSequece(std::uint32_t seq) { _sequece = seq; }

		std::uint8_t operator[](std::size_t index) const
		{
			std::uint8_t tab[] = { _1, _2, _3, _4, _5, _6, _7, _8 };
			return tab[index];
		}

		void set(std::size_t index, std::uint8_t value)
		{
			switch (index)
			{
			case 0:
				_1 = value;
				break;
			case 1:
				_2 = value;
				break;
			case 2:
				_3 = value;
				break;
			case 3:
				_4 = value;
				break;
			case 4:
				_5 = value;
				break;
			case 5:
				_6 = value;
				break;
			case 6:
				_7 = value;
				break;
			case 7:
				_8 = value;
				break;
			}
		}

	private:
		std::uint32_t _sequece;
		std::uint32_t _1 : 4;
		std::uint32_t _2 : 4;
		std::uint32_t _3 : 4;
		std::uint32_t _4 : 4;
		std::uint32_t _5 : 4;
		std::uint32_t _6 : 4;
		std::uint32_t _7 : 4;
		std::uint32_t _8 : 4;

	};

	VFrameResult::VFrameResult(std::uint32_t sequence, std::uint8_t v1, std::uint8_t v2, std::uint8_t v3, std::uint8_t v4, std::uint8_t v5, std::uint8_t v6, std::uint8_t v7, std::uint8_t v8)
	{
		*pimpl() = Private(sequence, v1, v2, v3, v4, v5, v6, v7, v8);
	}

	std::uint32_t VFrameResult::sequence() const
	{
		return pimpl()->sequence();
	}

	void VFrameResult::setSequence(std::uint32_t seq)
	{
		pimpl()->setSequece(seq);
	}

	std::uint8_t VFrameResult::operator[](std::size_t index) const
	{
		return (*pimpl())[index];
	}

	void VFrameResult::set(std::size_t index, std::uint8_t value)
	{
		return pimpl()->set(index, value);
	}

	const char * VFrameResult::bytes() const
	{
		return (const char*)&_unused;
	}

	std::size_t VFrameResult::byteSize() const
	{
		return sizeof(_unused);
	}

	auto Vsee::VFrameResult::pimpl() const -> Private * { return (Private*)&_unused; }
}


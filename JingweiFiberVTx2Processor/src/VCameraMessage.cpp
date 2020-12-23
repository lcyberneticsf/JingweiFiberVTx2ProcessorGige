#include "VCameraMessage.h"
#include "VCameraEnumPrivate.h"
#include "VCameraMessagePrivate.h"


namespace Vsee
{
	VCameraMessage::~VCameraMessage()
	{
		if (_private)
			delete _private;
	}

	VCameraMessage::VCameraMessage(const VCameraMessage & other) :
		_private(new VCameraMessagePrivate(*other._private))
	{
	}

	VCameraMessage & VCameraMessage::operator=(const VCameraMessage & rhs)
	{
		if (this != &rhs)
		{
			if (rhs._private)
			{
				if (_private)
					*_private = *rhs._private;
				else
					_private = new VCameraMessagePrivate(*rhs._private);
			}
			else
			{
				delete _private;
				_private = nullptr;
			}
		}

		return *this;
	}

	VCameraMessage::VCameraMessage(VCameraMessage&& other) noexcept :
		_private(other._private)
	{
		other._private = nullptr;
	}

	VCameraMessage & VCameraMessage::operator=(VCameraMessage&& rhs) noexcept
	{
		if (this != &rhs)
		{
			_private = rhs._private;
			rhs._private = nullptr;
		}

		return *this;
	}

	VCameraMessage::VCameraMessage(const char * raw, std::size_t size) :
		_private(new VCameraMessagePrivate(raw, size))
	{

	}

	VCameraMessage::VCameraMessage(std::uint32_t token, const char * data, size_t len, bool gen_crc) :
		_private(new VCameraMessagePrivate(token, data, len, gen_crc))
	{
	
	}

	const char * VCameraMessage::findLeadCode(const char * buf, std::size_t len)
	{
		return VCameraMessagePrivate::findLeadCode(buf, len);
	}

	bool VCameraMessage::empty() const
	{
		if (!_private)
			return true;
		return _private->empty();
	}

	std::size_t VCameraMessage::nextLoad(const char* buf, std::size_t buf_len)
	{
		return VCameraMessagePrivate::nextLoad(buf, buf_len);
	}

	VCameraMessage VCameraMessage::load(const char* buf, std::size_t size)
	{
		return VCameraMessage(VCameraMessagePrivate::load(buf, size));
	}

	bool VCameraMessage::checkCrc() const
	{
		if (!_private)
			return false;
		return _private->checkCrc();
	}

	std::uint32_t VCameraMessage::length() const 
	{
		if (!_private)
			return 0;

		return _private->length();
	}

	void VCameraMessage::setLength(std::uint32_t len, bool up)
	{
		if (_private)
			_private->setLength(len, up);
	}

	std::uint32_t VCameraMessage::token() const 
	{
		if (!_private)
			return 0;
		return _private->token();
	}

	void VCameraMessage::setToken(std::uint32_t tok, bool up)
	{
		if (_private)
			_private->setToken(tok, up);
	}

	const char * VCameraMessage::data() const 
	{
		if (!_private)
			return nullptr;
		return _private->data();
	}

	void VCameraMessage::setData(const char * d, std::size_t len, bool up)
	{
		if (_private)
			_private->setData(d, len, up);
	}

	std::size_t VCameraMessage::dataSize() const
	{
		if (!_private)
			return 0;
		return _private->dataSize();
	}

	std::uint32_t VCameraMessage::crc() const
	{
		if (!_private)
			return 0;
		return _private->crc();
	}

	void VCameraMessage::setCrc(std::uint32_t crc)
	{
		if (_private)
			_private->setCrc(crc);
	}

	void VCameraMessage::updateCrc()
	{
		if (_private)
			_private->updateCrc();
	}

	const char * VCameraMessage::bytes() const
	{
		if(!_private)
			return nullptr;
		return _private->bytes();
	}

	std::size_t VCameraMessage::byteSize() const
	{
		if (!_private)
			return 0;
		return _private->byteSize();
	}
}

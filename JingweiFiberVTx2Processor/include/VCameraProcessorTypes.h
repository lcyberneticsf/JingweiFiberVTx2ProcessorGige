#ifndef VSEE_CAMERA_PROCESSOR_TYPES_H_
#define VSEE_CAMERA_PROCESSOR_TYPES_H_

#include "VCameraProcessorApi.h"

#include <cstdint>
#include <cstddef>


namespace Vsee
{
	class VCameraProcessorApi VFrameResult
	{
	public:
		VFrameResult
		(
			std::uint32_t sequence = 0,
			std::uint8_t v1 = 0,
			std::uint8_t v2 = 0,
			std::uint8_t v3 = 0,
			std::uint8_t v4 = 0,
			std::uint8_t v5 = 0,
			std::uint8_t v6 = 0,
			std::uint8_t v7 = 0,
			std::uint8_t v8 = 0
		);

		enum ResType
		{
			None                = 0,
			Good                = 1,
			Unmat               = 2,
			MiddleUnmat         = 3,
			LightUnmat          = 4,
			FlatSunk            = 5,
			MiddleFlatSunk      = 6,
			LightFlatSunk       = 7,
		};

		std::uint32_t sequence() const;
		void setSequence(std::uint32_t seq);

		std::uint8_t operator[](std::size_t index) const;
		void set(std::size_t index, std::uint8_t value);

		const char* bytes() const;
		std::size_t byteSize() const;

	private:
		class Private;
		Private* pimpl() const;

	private:
		std::uint64_t _unused;

	};
}

#endif
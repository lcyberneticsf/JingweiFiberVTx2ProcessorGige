#pragma once

#ifndef VSEE_ASYNC_IO_H_
#define VSEE_ASYNC_IO_H_

#ifndef ASIO_STANDALONE
#	define ASIO_STANDALONE
#endif


#ifdef _MSC_VER
#	include <sdkddkver.h>
#endif

#include "asio/asio.hpp"


namespace Vsee
{
	using namespace asio;
	using namespace asio::ip;

	namespace Asio = asio;
	namespace Ip = asio::ip;
}

#endif
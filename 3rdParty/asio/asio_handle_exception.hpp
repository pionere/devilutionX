#pragma once

#include <asio/detail/throw_exception.hpp>

#define ErrAsio(message) dvl::app_fatal("ASIO Error: %s", message)

namespace dvl {

extern void app_fatal(const char* pszFmt, ...);

} // namespace dvl

namespace asio::detail {

template <typename Exception>
void throw_exception(
	const Exception &e
	ASIO_SOURCE_LOCATION_PARAM)
{
  ErrAsio(e.what());
}

} // namespace asio::detail

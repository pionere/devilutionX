#define ErrAsio(message) dvl::app_fatal("ASIO Error: %s", message)

namespace dvl {

extern void app_fatal(const char* pszFmt, ...);

} // namespace dvl

namespace asio::detail {

void fatal_exception(const char* message)
{
	ErrAsio(message);
}

} // namespace asio::detail

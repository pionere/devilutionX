#ifdef TCPIP
#include <errno.h>

int pause(void)
{
	errno = ENOSYS;
	return -1;
}
#endif // TCPIP
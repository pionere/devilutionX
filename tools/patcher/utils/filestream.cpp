#include "filestream.h"

DEVILUTION_BEGIN_NAMESPACE

static void LogErrorFFFF(const char* msg, ...)
{
	char tmp[256];

	FILE* f0 = NULL;
	while (f0 == NULL) {
		f0 = fopen("/storage/0403-0201/Android/data/org.diasurgical.devilx/files/logdebug0.txt", "a+");
	}

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	fputs(tmp, f0);

	fputc('\n', f0);

	fclose(f0);
}

// See https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
std::istream& safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			break;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			break;
		// case std::streambuf::traits_type::eof():
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty()) {
				is.setstate(std::ios::failbit);
			}
			break;
		default:
			t += (char)c;
			continue;
		}
		break;
	}
	return is;
}

DEVILUTION_END_NAMESPACE

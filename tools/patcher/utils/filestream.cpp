#include "filestream.h"

DEVILUTION_BEGIN_NAMESPACE

// See https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
/*std::istream&*/bool safeGetline(std::istream& is, std::string& t)
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
	// return is;
	return !is.fail();
}

DEVILUTION_END_NAMESPACE

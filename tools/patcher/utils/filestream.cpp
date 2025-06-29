#include "filestream.h"

DEVILUTION_BEGIN_NAMESPACE

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

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			break;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			break;
		case std::streambuf::traits_type::eof():
			// Also handle the case when the last line has no line ending
			if (t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
			continue;
		}
		break;
	}
	// Ignore last empty line
	if (t.empty() && sb->sgetc() == std::streambuf::traits_type::eof())
		is.setstate(std::ios::eofbit);
	return is;
}

DEVILUTION_END_NAMESPACE

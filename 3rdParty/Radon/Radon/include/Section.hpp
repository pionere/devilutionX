// Copyright Dmitro bjornus Szewczuk 2017 under zlib license

#pragma once

#include <string>
#include <vector>

#include "Named.hpp"

namespace radon
{
	class Key;

	class Section
		: public Named
	{
	public:
#ifdef FULL
		Section();
#endif
		Section(const std::string & name);

		Key *getKey(const std::string & name);

		void addKey(const std::string & name, const std::string & value);

	private:
		std::vector<Key> keys;

		friend class File;
	};
}
// Copyright Dmitro bjornus Szewczuk 2017

#include "../include/Radon.hpp"

#include <assert.h>

namespace radon
{
#ifdef FULL
	Section::Section()
		: Named()
	{
	}
#endif

	Section::Section(const std::string & name)
		: Named(name)
	{
	}


	Key *Section::getKey(const std::string & name)
	{
		for (size_t i = 0; i < keys.size(); i++)
		{
			if (keys[i].getName() == name)
				return &keys[i];
		}

		return NULL;
	}


	void Section::addKey(const std::string & name, const std::string & value)
	{
		keys.emplace_back(name, value);
	}
}
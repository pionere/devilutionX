// Copyright Dmitro bjornus Szewczuk 2017 under zlib license

#pragma once

#include <string>

#include "Named.hpp"

namespace radon
{
	class Key
		: public Named
	{
	public:
#ifdef FULL
		Key();
#endif
		Key(const std::string & name, const std::string & value);
#ifdef FULL
		Key(const std::string & name, const float & value);
#endif
		const std::string &getStringValue() const;
#ifdef FULL
		float getFloatValue();

		void setValue(float & value);
#endif
		void setValue(const std::string & value);

	private:
		std::string value;

		friend class File;
	};
}
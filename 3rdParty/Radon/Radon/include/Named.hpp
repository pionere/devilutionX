// Copyright Dmitro bjornus Szewczuk 2017 under zlib license

#pragma once

#include <string>

namespace radon
{
	class Named
	{
	public:
#ifdef FULL
		Named();
#endif
		Named(const std::string & name);

		void setName(const std::string & name);

		const std::string &getName() const;

	protected:
		std::string name;
	};
}
// Copyright Dmitro bjornus Szewczuk 2017

#include "../include/Radon.hpp"
#ifdef FULL
#include <sstream>

std::string Float2String(float fVal)
{
	std::ostringstream ss;
	ss << fVal;
	std::string s(ss.str());
	return s;
}
#endif

namespace radon
{
#ifdef FULL
	Key::Key()
		: Named()
	{
	}
#endif

	Key::Key(const std::string & name, const std::string & value)
		: Named(name), value(value)
	{
	}

#ifdef FULL
	Key::Key(const std::string & name, const float & value)
		: Named(name), value(Float2String(value))
	{
	}
#endif

	const std::string& Key::getStringValue() const
	{
		return value;
	}

#ifdef FULL
	float Key::getFloatValue()
	{
		return (float)(atof(value.data()));
	}


	void Key::setValue(float & value)
	{
		this->value = Float2String(value);
	}
#endif

	void Key::setValue(const std::string & value)
	{
		this->value = value;
	}
}
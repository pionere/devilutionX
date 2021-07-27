// Copyright Dmitro bjornus Szewczuk 2017

#include "../include/Radon.hpp"

namespace radon
{
#ifdef FULL
	Named::Named()
	{
		setName("You need to set name!");
	}
#endif

	Named::Named(const std::string & name)
	{
		setName(name);
	}


	void Named::setName(const std::string & name)
	{
		this->name = name;
	}


	const std::string& Named::getName() const
	{
		return name;
	}
}
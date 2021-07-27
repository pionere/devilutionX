// Copyright Dmitro bjornus Szewczuk 2017

#include "../include/Radon.hpp"

#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <assert.h>

namespace radon
{
	File::File(const std::string & path)
	{
		this->path = path;
		std::ifstream stream(path.c_str());

		if (stream.is_open()) {
			std::string buffer;
			std::string nameOfCurrent;

			while (std::getline(stream, buffer)) {
				if (buffer[0] == ';' || buffer[0] == '#') continue;
				if (buffer[0] == '[') {
					auto endPos = buffer.find("]");
					if (endPos == std::string::npos)
						continue;
					nameOfCurrent = buffer.substr(1, endPos - 1);
					sections.emplace_back(nameOfCurrent);
				} else if (!sections.empty()) {
					auto equalsPosition = buffer.find('=');
					if (equalsPosition == std::string::npos)
						continue;

					std::string nameOfElement = buffer.substr(0, equalsPosition);
					std::string valueOfElement = buffer.substr(equalsPosition + 1, buffer.size());

					sections.back().addKey(nameOfElement, valueOfElement);
				}
			}
		}
	}


	Section *File::getSection(const std::string & name)
	{
		for (size_t i = 0; i < sections.size(); i++)
		{
			if (sections[i].getName() == name)
			{
				return &sections[i];
			}
		}

		return NULL;
	}


	Section* File::addSection(const std::string & name)
	{
		return &sections.emplace_back(name);
		//sections.emplace_back(name);
		//return &sections.back();
	}


	void File::saveToFile()
	{
		std::ofstream file(path.data(), std::ios::out | std::ios::trunc);

		for (size_t i = 0; i < sections.size(); i++) {
			file << "[" << sections[i].getName() << "]\n";
			for (size_t j = 0; j < sections[i].keys.size(); j++) {
				file << sections[i].keys[j].getName() << "=" << sections[i].keys[j].getStringValue() << "\n";
			}
		}
		file.close();
	}
}
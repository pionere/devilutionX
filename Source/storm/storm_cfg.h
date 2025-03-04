#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

void InitConfig();
void FreeConfig();

bool getIniBool(const char* sectionName, const char* keyName, bool defaultValue);
const char* getIniStr(const char* sectionName, const char* keyName);
int getIniValue(const char* sectionName, const char* keyName, char* string, int stringSize);
void setIniValue(const char* sectionName, const char* keyName, const char* value);
bool getIniInt(const char* sectionName, const char* keyName, int* value);
void setIniInt(const char* sectionName, const char* keyName, int value);
void delIniValue(const char* sectionName, const char* keyName);

void SLoadKeyMap(BYTE (&map)[256]);

DEVILUTION_END_NAMESPACE

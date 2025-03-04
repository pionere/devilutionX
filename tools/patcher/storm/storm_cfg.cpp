#include "storm_cfg.h"

#include <string>
#include <vector>

#include "utils/file_util.h"
#include "utils/paths.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAX_LINE_LEN 1024

typedef struct ConfigEntry {
	const char* key;
	const char* value;
	ConfigEntry(const char* k, const char* v) {
		key = k;
		value = v;
	}
} ConfigEntry;

typedef struct ConfigSection {
	const char* name;
	std::vector<ConfigEntry> entries;
	ConfigSection(const char* n) {
		name = n;
	}
	ConfigEntry* getEntry(const char* name);
	ConfigEntry* addEntry(const char* name, const char* value);
} ConfigSection;

typedef struct DiabloConfig {
	bool modified;
	std::vector<ConfigSection> sections;
} DiabloConfig;

DiabloConfig config;

ConfigEntry* ConfigSection::getEntry(const char* name)
{
	for (ConfigEntry &entry : entries) {
		if (SDL_strcmp(name, entry.key) == 0) {
			return &entry;
		}
	}
	return NULL;
}

ConfigEntry* ConfigSection::addEntry(const char* key, const char* value)
{
	entries.push_back(ConfigEntry(SDL_strdup(key), SDL_strdup(value)));
	return &entries.back();
}

static FILE* getConfigFile(const char* mode)
{
	std::string path = GetPrefPath();
	path += "diablo.ini";
	return FileOpen(path.c_str(), mode);
}

static ConfigSection* getSection(const char* name)
{
	for (ConfigSection &section : config.sections) {
		if (SDL_strcmp(name, section.name) == 0) {
			return &section;
		}
	}
	return NULL;
}

static ConfigSection* addSection(const char* name)
{
	config.sections.push_back(ConfigSection(SDL_strdup(name)));

	return &config.sections.back();
}

void InitConfig()
{
	FILE* f = getConfigFile("rt");
	if (f == NULL) {
		return;
	}
	char tmp[MAX_LINE_LEN];
	unsigned cursor = 0;
	ConfigSection* section = NULL;
	while (true) {
		unsigned len = fread(&tmp[cursor], 1, sizeof(tmp) - cursor, f);
		len += cursor;
		if (len == 0) {
			break;
		}
		if (len < sizeof(tmp)) {
			tmp[len] = '\r';
			len++;
		}
		cursor = 0;
		while (true) {
			// find the end of the line
			unsigned sp = cursor;
			while (tmp[cursor] != '\r' && tmp[cursor] != '\n') {
				cursor++;
				if (cursor == len) {
					cursor = sp;
					goto done;
				}
			}

			if (tmp[sp] == '[') {
				// add a section
				sp++;
				unsigned ep = sp;
				while (tmp[ep] != ']') {
					ep++;
					if (ep == cursor) {
						goto eol;
					}
				}
				tmp[ep] = '\0';
				section = addSection(&tmp[sp]);
				goto eol;
			}
			if (tmp[sp] != ';' && section != NULL && sp != cursor) {
				// add an entry
				unsigned ep = sp;
				while (tmp[ep] != '=') {
					ep++;
					if (ep == cursor) {
						goto eol;
					}
				}
				char ch = tmp[cursor];
				tmp[ep] = '\0';
				tmp[cursor] = '\0';
				section->addEntry(&tmp[sp], &tmp[ep + 1]);
				tmp[cursor] = ch;
			}
eol:
			// skip the line-end
			// assert(tmp[cursor] == '\r' || tmp[cursor] == '\n');
			cursor++;
			if (cursor == len) {
				goto done;
			}
			if (tmp[cursor] == '\n') {
				cursor++;
				if (cursor == len) {
					goto done;
				}
			}
		}
done:
		if (cursor == 0) {
			DoLog("Failed to parse diablo.ini due to a long line.");
			break;
		}
		memmove(tmp, &tmp[cursor], sizeof(tmp) - cursor);
		len -= cursor;
		cursor = len;
	}

	fclose(f);
}

void FreeConfig()
{
	if (config.modified) {
		config.modified = false;

		FILE* f = getConfigFile("wt");
		if (f != NULL) {
			char tmp[MAX_LINE_LEN];
			for (const ConfigSection &section : config.sections) {
				unsigned len = snprintf(tmp, sizeof(tmp), "[%s]\n", section.name);
				if (!WriteFile(tmp, len, f)) {
					DoLog("Failed to add section '%s' to the config.", section.name);
				}
				for (const ConfigEntry &entry : section.entries) {
					unsigned len = snprintf(tmp, sizeof(tmp), "%s=%s\n", entry.key, entry.value);
					if (!WriteFile(tmp, len, f)) {
						DoLog("Failed to add entry '%s' (with value '%s') to the config.", entry.key, entry.value);
					}
				}
			}

			fclose(f);
		} else {
			DoLog("Failed to store the config (diablo.ini).");
		}
	}

	for (ConfigSection &section : config.sections) {
		for (ConfigEntry &entry : section.entries) {
			free(const_cast<char*>(entry.key));
			free(const_cast<char*>(entry.value));
		}
	}

	config.sections.clear();
}

bool getIniBool(const char* sectionName, const char* keyName, bool defaultValue)
{
	int value = defaultValue;

	getIniInt(sectionName, keyName, &value);

	return value != 0;
}

const char* getIniStr(const char* sectionName, const char* keyName)
{
	ConfigSection* section = getSection(sectionName);
	if (section == NULL) {
		return NULL;
	}

	ConfigEntry* entry = section->getEntry(keyName);
	if (entry == NULL) {
		return NULL;
	}
	return entry->value;
}

int getIniValue(const char* sectionName, const char* keyName, char* string, int stringSize)
{
	const char* valueStr = getIniStr(sectionName, keyName);
	if (valueStr == NULL)
		return -1;

	return SStrCopy(string, valueStr, stringSize);
}

void setIniValue(const char* sectionName, const char* keyName, const char* value)
{
	ConfigSection* section = getSection(sectionName);
	if (section == NULL) {
		section = addSection(sectionName);
	}

	ConfigEntry* entry = section->getEntry(keyName);
	if (entry == NULL) {
		section->addEntry(keyName, value);
	} else {
		if (SDL_strcmp(value, entry->value) == 0)
			return;
		free(const_cast<char*>(entry->value));
		entry->value = SDL_strdup(value);
	}
	config.modified = true;
}

bool getIniInt(const char* sectionName, const char* keyName, int* value)
{
	const char* valueStr = getIniStr(sectionName, keyName);
	if (valueStr != NULL) {
		*value = SDL_strtol(valueStr, NULL, 10);
		return true;
	}

	return false;
}

void setIniInt(const char* sectionName, const char* keyName, int value)
{
	char str[10];
	snprintf(str, sizeof(str), "%d", value);
	setIniValue(sectionName, keyName, str);
}

DEVILUTION_END_NAMESPACE

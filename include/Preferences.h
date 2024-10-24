#pragma once

#include "Framework.h"
#include <queue>
#include <filesystem>
#include <map>

class Preferences {
public:
	Preferences(const Preferences&) = delete;
	Preferences(Preferences&&) = delete;
	Preferences& operator=(const Preferences&) = delete;
	Preferences& operator=(Preferences&&) = delete;

	static Preferences* Get();

	Preferences();
	~Preferences();

	void Load();
	void Save();

	void SaveParameter(const tstring& key, const tstring& value);
	tstring LoadParameter(const tstring& key);

	int _tile_resolution;
	int _lazy_save_count;
	std::uint32_t _tile_default_color;

	int _file_recents_max;	
	std::queue<std::filesystem::path> _file_recents;
	std::filesystem::path _file_last_openned;
	float _brush_step;
};


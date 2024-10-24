#include "Preferences.h"

#include "Log.h"
#include "App.h"

Preferences* g_preferences;

Preferences* Preferences::Get() {
	return g_preferences;
}

Preferences::Preferences() {
	_tile_resolution = 256;
	_lazy_save_count = 4;
	_tile_default_color = 0x00FFFFFF;

	_file_recents_max;
	_file_recents;
	_file_last_openned;
	_brush_step = 0.5f;

	g_preferences = this;
}

Preferences::~Preferences() {
	Save();
}

void Preferences::Load() {
}

void Preferences::Save() {}

void Preferences::SaveParameter(const tstring& key, const tstring& value) {}

tstring Preferences::LoadParameter(const tstring& key) {
	return tstring();
}
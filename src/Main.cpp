#include <stdexcept>
#include "Framework.h"
#include "App.h"
#include "Log.h"

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd) {
	try {
		Log::Info(lpCmdLine);
		App app(hInstance, nShowCmd);
		app.Run();
	} catch (const std::runtime_error& e) {
		tstring err = ConvertString(e.what());
		Log::Info(err);
		MessageBox(nullptr, err.c_str(), TEXT("Mashiro"), MB_ICONERROR | MB_OK);
		return -1;
	}

	return 0;
}


int main() {
	return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_NORMAL);
}
#pragma once

#include "Framework.h"
#include <vector>

class Tool {};
class Preferences {};
class Inputs {};

// Handle the window
// Handle the runnning of the application
class App {
  public:
    App(const App &) = delete;
    App(App &&) = delete;
    App &operator=(const App &) = delete;
    App &operator=(App &&) = delete;

    static App Get();

    App(HINSTANCE hInstance, std::vector<tstring> args);
    ~App();
    void Run();

    void Update();
    void Render();

    void New();
    void Open();
    void Exit();
    void Save();
    void SaveAs();

  protected:
    void InitPreferences();
    void InitWindow();
    void InitRenderer();
    void InitInputs();

  private:
    HINSTANCE _instance;
    std::vector<tstring> _args;

    Preferences _preferences;
};

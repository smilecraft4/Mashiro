#pragma once

#include <wx/wx.h>

namespace Mashiro
{

enum {
    ID_HELLO = 1,
    ID_FULLSCREEN = 2,
    ID_WINDOW = 3,
};

class App : public wxApp
{
  public:
    virtual bool OnInit();
};

class Frame : public wxFrame
{
  public:
    Frame();

  private:
    void OnHello(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);

    void OnFullscreen(wxCommandEvent &event);
    void OnWindow(wxCommandEvent &event);
};

} // namespace Mashiro

wxIMPLEMENT_APP(Mashiro::App);
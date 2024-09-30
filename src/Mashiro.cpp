#include "Mashiro.h"

#include <wx/menu.h>

bool Mashiro::App::OnInit()
{
  auto frame = new Frame();
  frame->Show();
  return true;
}

Mashiro::Frame::Frame() : wxFrame(NULL, wxID_ANY, "Mashiro")
{
  auto menuFile = new wxMenu();
  menuFile->Append(ID_HELLO, "&Hello...\tCtrl-h", "Help string shown in status bar for this menu item");
  menuFile->Append(ID_FULLSCREEN, "&Fullscreen...\tCtrl-h", "");
  menuFile->Append(ID_WINDOW, "&Window...\tCtrl-h", "");

  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  auto menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText("Welcom to wxWidgets!");

  Bind(wxEVT_MENU, &Frame::OnHello, this, ID_HELLO);

  Bind(wxEVT_MENU, &Frame::OnFullscreen, this, ID_FULLSCREEN);
  Bind(wxEVT_MENU, &Frame::OnWindow, this, ID_WINDOW);

  Bind(wxEVT_MENU, &Frame::OnExit, this, wxID_EXIT);
  Bind(wxEVT_MENU, &Frame::OnAbout, this, wxID_ABOUT);
}

void Mashiro::Frame::OnHello(wxCommandEvent &event)
{
  wxLogMessage("Hello world from Mashiro!");
}

void Mashiro::Frame::OnExit(wxCommandEvent &event)
{
  Close(true);
}

void Mashiro::Frame::OnAbout(wxCommandEvent &event)
{
  wxMessageBox("This is a wxWidgets, Hello from Mashiro", "About Mashiro", wxOK | wxICON_INFORMATION);
}

void Mashiro::Frame::OnFullscreen(wxCommandEvent &event)
{
  ShowFullScreen(true);
}

void Mashiro::Frame::OnWindow(wxCommandEvent &event)
{
  ShowFullScreen(false);
}

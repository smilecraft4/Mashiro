#include <catch2/catch_test_macros.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#include <format>
#include <shobjidl.h>
#include <string>
#include <windowsx.h>

static COMDLG_FILTERSPEC rgSpec[] = {{TEXT("Mashiro files"), TEXT("*.msh")}};

class App {
  public:
    App(std::filesystem::path filename, bool saved, bool openned, bool named)
        : filename(filename), saved(saved), openned(openned), named(named) {
    }

    void SaveAs() {
        if (openned) {
            // SaveAsDialog

            // File::Save(filename)
            MessageBox(hwnd, std::format(TEXT("Saved \"{}\""), filename.wstring()).c_str(), TEXT("SaveAs test"), MB_OK);
            filename = "./unnamed.msh"; // TODO
            saved = true;
            named = true;
            openned = true;
        }
    }

    void Save() {
        if (openned) {
            if (!saved) {
                if (!named) {
                    SaveAs();
                } else {
                    // File::Save(filename)
                    MessageBox(hwnd, std::format(TEXT("Saved \"{}\""), filename.wstring()).c_str(), TEXT("Save test"),
                               MB_OK);
                    saved = true;
                    named = false;
                    openned = true;
                }
            }
        }
    }

    void New() {
        if (openned) {
            if (!saved) {
                if (MessageBox(hwnd, std::format(TEXT("Do you want to save \"{}\" ?"), filename.wstring()).c_str(),
                               TEXT("New test"), MB_YESNO) == MB_OK) {
                    Save();
                }
            }

            // File::Release()
            MessageBox(hwnd, std::format(TEXT("Releasing openned file \"{}\""), filename.wstring()).c_str(),
                       TEXT("New test"), MB_OK);
            openned = false;
            saved = false;
            named = false;
            filename = "";
        }

        // File::New()
        filename = "./unnamed.msh";
        MessageBox(hwnd, std::format(TEXT("Create new file \"{}\""), filename.wstring()).c_str(), TEXT("New test"),
                   MB_OK);
        openned = true;
        saved = false;
        named = false;
    }

    void Open() {
        if (openned) {
            if (!saved) {
                if (MessageBox(hwnd, std::format(TEXT("Do you want to save \"{}\""), filename.wstring()).c_str(),
                               TEXT("Open test"), MB_YESNO) == MB_OK) {
                    Save();
                }
            }

            // File::Release()
            MessageBox(hwnd, std::format(TEXT("Releasing openned file \"{}\""), filename.wstring()).c_str(),
                       TEXT("New test"), MB_OK);
            openned = false;
            saved = false;
            named = false;
            filename = "";
        }

        const auto path = OpenDialog();

        // File::Open(path)
        filename = path;
        MessageBox(hwnd, std::format(TEXT("Openned file \"{}\""), filename.wstring()).c_str(), TEXT("Open test"), MB_OK);
        openned = true;
        saved = false;
        named = true;
    }

    void Exit() {
        Save();

        // File::Release()
        MessageBox(hwnd, std::format(TEXT("Releasing openned file \"{}\""), filename.wstring()).c_str(),
                   TEXT("New test"), MB_OK);
        openned = false;
        saved = false;
        named = false;
        filename = "";

        MessageBox(hwnd, TEXT("Exited app"), TEXT("Open test"), MB_OK);
    }

  private:
    std::filesystem::path SaveAsDialog() {
        return "";
    }

    std::filesystem::path OpenDialog() {
        return "";
    }

    HWND hwnd;
    std::filesystem::path filename;
    bool openned;
    bool saved;
    bool named;
};

// TODO: All the scenerios with expected outcomes !

SCENARIO("Prompt save") {

    GIVEN("An app with no file") {
        App app("", false, false, false);

        WHEN("Saving") {
            app.Save();
        }

        WHEN("Saving As") {
            app.SaveAs();
        }

        WHEN("Openning file") {
            app.Open();
        }

        WHEN("Creating a new file") {
            app.New();
        }
    }

    GIVEN("An app with an new file") {
        App app("unnamed.msh", false, true, false);

        WHEN("Saving") {
            app.Save();
        }

        WHEN("Saving As") {
            app.SaveAs();
        }

        WHEN("Openning file") {
            app.Open();
        }

        WHEN("Creating a new file") {
            app.New();
        }
    }

    GIVEN("An app with a unsaved file") {
        App app("unnamed.msh", false, true, false);

        WHEN("Saving") {
            app.Save();
        }

        WHEN("Saving As") {
            app.SaveAs();
        }

        WHEN("Openning file") {
            app.Open();
        }

        WHEN("Creating a new file") {
            app.New();
        }
    }

    GIVEN("An app with a saved file") {
        App app("super.msh", false, true, true);

        WHEN("Saving") {
            app.Save();
        }

        WHEN("Saving As") {
            app.SaveAs();
        }

        WHEN("Openning file") {
            app.Open();
        }

        WHEN("Creating a new file") {
            app.New();
        }
    }

    // Should be impossible
    GIVEN("An app with a saved unnamed file") {
        App app("unnamed.msh", true, true, false);

        WHEN("Saving") {
            app.Save();
        }

        WHEN("Saving As") {
            app.SaveAs();
        }

        WHEN("Openning file") {
            app.Open();
        }

        WHEN("Creating a new file") {
            app.New();
        }
    }
}
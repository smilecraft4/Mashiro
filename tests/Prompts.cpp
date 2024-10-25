#include <catch2/catch_test_macros.hpp>

#define WIN32_LEAN_AND_MEAN
#include <ShObjIdl.h>
#include <Windows.h>
#include <filesystem>
#include <format>
#include <string>
#include <windowsx.h>

static COMDLG_FILTERSPEC rgSpec[] = {{TEXT("Mashiro files"), TEXT("*.msh")}};

static std::filesystem::path SaveAsDialog(std::filesystem::path directory = "./",
                                          std::filesystem::path filename = "unsaved.msh");
static std::filesystem::path OpenDialog(std::filesystem::path directory = "./");

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
        MessageBox(hwnd, std::format(TEXT("Openned file \"{}\""), filename.wstring()).c_str(), TEXT("Open test"),
                   MB_OK);
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
    HWND hwnd;
    std::filesystem::path filename;
    bool openned;
    bool saved;
    bool named;
};

std::filesystem::path SaveAsDialog(std::filesystem::path directory, std::filesystem::path filename) {
    const auto filename = directory / filename;
    return "";
}

std::filesystem::path OpenDialog(std::filesystem::path directory) {
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw new std::runtime_error("Directory provided as default path for OpenDialog does not exists");
    }

    // Make sure the path is absolute
    if (directory.is_absolute()) {
        directory = std::filesystem::absolute(directory);
    }

    std::filesystem::path filename;

    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

    // Set the options on the dialog.
    DWORD dwFlags;

    // Before setting, always get the options first in order
    // not to override existing options.
    hr = pfd->GetOptions(&dwFlags);

    // In this case, get shell items only for file system items.
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);

    // Set the file types to display only.
    // Notice that this is a 1-based array.
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

    // Set the selected file type index to Word Docs for this example.
    hr = pfd->SetFileTypeIndex(0);

    // Set the default extension to be ".doc" file.
    hr = pfd->SetDefaultExtension(L"msh");
    IShellItem *psiResult;

    IShellItem *folder;
    hr = SHCreateItemFromParsingName(directory.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
    hr = pfd->SetDefaultFolder(folder);

    hr = pfd->Show(NULL);

    IShellItem *psiResult;
    hr = pfd->GetResult(&psiResult);

    PWSTR pszFilePath = NULL;
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    filename = std::filesystem::path(pszFilePath);

    psiResult->Release();

    pfd->Release();

    return filename;
}

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
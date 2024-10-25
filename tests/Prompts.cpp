#include <catch2/catch_test_macros.hpp>

#define WIN32_LEAN_AND_MEAN
#include <ShObjIdl.h>
#include <Windows.h>
#include <filesystem>
#include <format>
#include <string>
#include <windowsx.h>

static COMDLG_FILTERSPEC rgSpec[] = {{TEXT("Mashiro files"), TEXT("*.msh")}, {TEXT("All files"), TEXT("*.*")}};

static std::filesystem::path SaveAsDialog(std::filesystem::path directory = "./",
                                          std::filesystem::path filename = "unsaved.msh");
static std::filesystem::path OpenDialog(std::filesystem::path directory = "./");

class App {
  public:
    App(std::filesystem::path filename, bool saved, bool openned, bool named)
        : filename(filename), saved(saved), openned(openned), named(named), hwnd(NULL) {
    }

    void SaveAs() {
        if (openned) {
            // File::Save(filename)
            filename = SaveAsDialog();
            MessageBox(hwnd, std::format(TEXT("Saved \"{}\""), filename.wstring()).c_str(), TEXT("SaveAs test"), MB_OK);
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

        // File::Open(path)
        filename = OpenDialog();
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
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw new std::runtime_error("Directory provided as default path for SaveAsDialog does not exists");
    }
    directory = std::filesystem::absolute(directory);

    IFileDialog *pfd{};
    IShellItem *psiResult{};
    IShellItem *folder{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));
    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetFileName(filename.wstring().c_str());
    hr = pfd->SetDefaultExtension(L"msh");
    hr = SHCreateItemFromParsingName(directory.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
    hr = pfd->SetDefaultFolder(folder);

    hr = pfd->Show(NULL);
    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    folder->Release();
    psiResult->Release();
    pfd->Release();

    return path;
}

std::filesystem::path OpenDialog(std::filesystem::path directory) {
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw new std::runtime_error("Directory provided as default path for OpenDialog does not exists");
    }

    directory = std::filesystem::absolute(directory);

    IFileDialog *pfd{};
    IShellItem *folder{};
    IShellItem *psiResult{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetDefaultExtension(L"msh");
    hr = SHCreateItemFromParsingName(directory.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
    hr = pfd->SetDefaultFolder(folder);

    hr = pfd->Show(NULL);
    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    folder->Release();
    psiResult->Release();
    pfd->Release();

    return path;
}

// TODO: Add the expected result ! (check for the bools, names, etc...)
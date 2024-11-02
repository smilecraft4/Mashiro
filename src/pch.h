#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <initializer_list>
#include <istream>
#include <map>
#include <memory>
#include <queue>
#include <span>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <ShObjIdl.h>
#include <comdef.h>
#include <commctrl.h>
// #include <gl/GL.h>
#include <windows.h>
#include <windowsx.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <png.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Opengl32.lib")

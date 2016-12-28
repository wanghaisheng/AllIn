#include <string>
#include <windows.h>
#include "common_definitions.h"

bool MC::GetMoudulePath(std::string& path)
{
    char file_path[_MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, file_path, _MAX_FNAME) == 0)
        return false;

    std::string file_path_str = file_path;
    size_t last_slash = file_path_str.find_last_of(PATHSPLIT_CHAR);
    if (last_slash == std::string::npos)
        return false;

    path = file_path_str.substr(0, last_slash + 1);
    return true;
}

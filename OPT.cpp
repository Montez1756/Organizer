#include "OPT.h"
#ifdef _WIN32
#include <windows.h>
#undef Rectangle
#undef CloseWindow
#else
#include <unistd.h>
#endif


OPT::OPT(int argc, char **argv) : num_args(argc), clParams(argv) {}

void OPT::add_argument(const std::string &arg)
{
    argsVec.push_back(arg);
}

void OPT::parse_args()
{
    int i = 0;
    while(i < num_args)
    {
        if(contains(argsVec, std::string(clParams[i])))
        {
            std::string key = clParams[i];
            std::vector<std::string> params;
            i++;
            while(i < num_args && !contains(argsVec, std::string(clParams[i])))
            {
                params.emplace_back(clParams[i]);
                i++;
            }
            paramsMap[key] = params;
        }
        else {
            i++;
        }
    }
}

std::vector<std::string> OPT::operator[](const std::string &key)
{
    auto it = paramsMap.find(key);
    if (it != paramsMap.end()) {
        return it->second;
    }
    return {}; // return empty vector if key not found
}

std::string get_username() {
    #ifdef _WIN32
    char username[256 + 1];
    DWORD username_len = 256 + 1;
    GetUserNameA(username, &username_len);
    return std::string(username);
    #else
    passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_name);
    } else {
        return "";
    }
    #endif
}

std::string get_downloads_path() {
    #ifdef _WIN32
    std::string downloads_path = getenv("USERPROFILE");
    downloads_path += "\\Downloads";
    return downloads_path;
    #else
    std::string downloads_path = "/home/";
    downloads_path += get_username();
    downloads_path += "/Downloads";
    return downloads_path;
    #endif
}

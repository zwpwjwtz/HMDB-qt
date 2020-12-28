#include <list>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#endif
 #include "filesystem.h"


std::vector<const char*> utils_listDirectoryFiles(const char* dir)
{
    if (!dir)
        return std::vector<const char*>();

    std::list<const char*> fileList;
#ifdef _WIN32
    char* pattern = new char[strlen(dir) + 4];
    strcpy(pattern, dir);
    strcat(pattern, "\\*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern, &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            fileList.push_back(data.cFileName);
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
    delete[] pattern;
#else
    DIR* d = opendir(dir);
    struct dirent* dEntry;
    while ((dEntry = readdir(d)) != nullptr)
        fileList.push_back(dEntry->d_name);
    closedir(d);
#endif

    return std::vector<const char*>{ fileList.begin(), fileList.end() };
}

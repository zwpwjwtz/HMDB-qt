#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "filesystem.h"


std::list<std::string> utils_listDirectoryFiles(const char* dir)
{
    std::list<std::string> fileList;
    if (!dir)
        return fileList;

#ifdef _WIN32
    char* pattern = new char[strlen(dir) + 4];
    strcpy(pattern, dir);
    strcat(pattern, "\\*");
    WIN32_FIND_DATAA data;
    HANDLE hFind;
    if ((hFind = FindFirstFileA(pattern, &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            fileList.push_back(data.cFileName);
        } while (FindNextFileA(hFind, &data) != 0);
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

    return fileList;
}

bool utils_isDirectory(const char* path)
{
    struct stat fileInfo;
    return stat(path, &fileInfo) == 0 && S_ISDIR(fileInfo.st_mode);
}

bool utils_isFile(const char* path)
{
    struct stat fileInfo;
    return stat(path, &fileInfo) == 0 && S_ISREG(fileInfo.st_mode);
}

long utils_fileLength(const char* filename)
{
    struct stat fileInfo;
    if (stat(filename, &fileInfo) == 0)
        return fileInfo.st_size;
    else
        return 0;
}

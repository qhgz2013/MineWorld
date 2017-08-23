#include "util.h"
#include <Windows.h>
#include <ctime>
#include <list>
#include <io.h>
using namespace std;
uint64_t GetCurrentTimestamp()
{
	const int64_t time_unix = 116444736000000000i64;
	FILETIME ft;
	LARGE_INTEGER li;
	int64_t tt = 0;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	tt = (li.QuadPart - time_unix);

	return tt;
}


void RecursiveDelete(const char * path)
{
	string origin = path;
	list<string> dir_list;
	dir_list.push_back(origin);
	list<string> file_list;
	auto iter_dir = dir_list.begin();
	for (; iter_dir != dir_list.end(); iter_dir++)
	{
		//bfs mode
		string dir = *dir_list.begin();

		char  szTempDir[MAX_PATH + 1];
		strcpy(szTempDir, dir.c_str());
		strcat(szTempDir, "*");
		HANDLE  hFindFile;
		WIN32_FIND_DATAA  FileData;

		hFindFile = FindFirstFileA(szTempDir, &FileData);

		if (hFindFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		else
		{
			do
			{
				if (strcmp(FileData.cFileName, ".") == 0 || strcmp(FileData.cFileName, "..") == 0) continue;

				char szNewDir[MAX_PATH + 1];
				strcpy(szNewDir, dir.c_str());
				strcat(szNewDir, FileData.cFileName);
				if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					strcat(szNewDir, "\\");
					dir_list.push_back(string(szNewDir));
				}
				else
					file_list.push_back(string(szNewDir));
			} while (FindNextFileA(hFindFile, &FileData));

			FindClose(hFindFile);
		}
	}

	for (auto i = file_list.begin(); i != file_list.end(); i++)
	{
		remove(i->c_str());
	}
	for (auto i = dir_list.rbegin(); i != dir_list.rend(); i++)
	{
		remove(i->c_str());
	}
}

double fGetCurrentTimestamp()
{
	return GetCurrentTimestamp() * 0.0000001;
}
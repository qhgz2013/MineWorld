#include "util.h"
#include <Windows.h>
#include <ctime>
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
double fGetCurrentTimestamp()
{
	return GetCurrentTimestamp() * 0.0000001;
}
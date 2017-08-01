#include "chunkloader.h"
#include <cstdio>
#include <io.h>
#include <direct.h>
#include <fstream>
#include "pointer_check.h"

using namespace std;

Tag * ChunkLoader::_load_chunk_from_file(int chunk_x, int chunk_y)
{
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, 6) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_FMT, chunk_x, chunk_y);
	ifstream ifs(filename, ios::binary | ios::in);
	TagString* tag_version = (TagString*)Tag::ReadTagFromStream(ifs);
	TagByteArray* tag_data = (TagByteArray*)Tag::ReadTagFromStream(ifs);

	ifs.close();
	return tag_data;
}

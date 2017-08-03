#include "mineworld.h"
#include <QtWidgets/QApplication>
#include "pointer_check.h"
#include <string>
#include "chunkloader.h"
using namespace std;

int __main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	MineWorld w;
	w.show();
	return a.exec();
}
Tag* callback(int chunk_x, int chunk_y)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;
	char* data = debug_new char[size*size];
	memset(data, 0, size*size);

	Tag* ret = debug_new TagByteArray(string("chunkdata"), data, size*size);
	debug_delete[] data;
	return ret;
}
int main(int argc, char *argv[])
{
	int result = __main(argc, argv);
	{
		ChunkLoader cl(callback);
		char data = cl.GetBlockData(2, 2);
		cl.SetBlockData(2, 2, '\n');
		data = cl.GetBlockData(2, 2);
		data = cl.GetBlockData(4, 4);

		char** data2 = debug_new char*[1 << DEFAULT_CHUNK_SIZE];
		cl.GetChunkData(0, 0, data2);

		data2[4][4] = 'a';
		cl.SetChunkData(0, 0, (const char**)data2);

		data = cl.GetBlockData(4, 4);
		for (int i = 0; i < (1 << DEFAULT_CHUNK_SIZE); i++)
		{
			debug_delete[] data2[i];
		}
		debug_delete[] data2;
	}
	auto ptr_result = get_leak_memory();
	
	return result;
}

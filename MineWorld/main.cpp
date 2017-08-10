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
	size *= size;
	char* data = debug_new char[size];
	for (int i = 0; i < size; i++)
	{
		int pos = rand() % 100;
		if (pos >= DEFAULT_GEN_MINE_POSSIBILITY)
			data[i] = 0x00;
		else
			data[i] = 0x10;
	}

	Tag* ret = debug_new TagByteArray(string("chunkdata"), data, size);
	debug_delete[] data;
	return ret;
}
int main(int argc, char *argv[])
{
	srand(time(nullptr));

	int result = __main(argc, argv);
	auto ptr_result = get_leak_memory();
	
	return result;
}

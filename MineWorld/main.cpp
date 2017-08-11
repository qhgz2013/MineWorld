#include "mineworld.h"
#include <QtWidgets/QApplication>
#include "pointer_check.h"
#include <string>
#include "chunkloader.h"
#include <QtConcurrent\qtconcurrentmap.h>
using namespace std;

int __main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	MineWorld w;
	w.show();
	return a.exec();
}
int main(int argc, char *argv[])
{
	srand(time(nullptr));

	int result = __main(argc, argv);
	auto ptr_result = get_leak_memory();

	return result;
}

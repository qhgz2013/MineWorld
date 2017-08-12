#include "mineworld.h"
#include <QtWidgets/QApplication>
#include "pointer_check.h"
#include <string>
#include <Windows.h>
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

	uint64_t before_pointer = allocated_block;
	int result = __main(argc, argv);
	uint64_t after_pointer = allocated_block;
	if (after_pointer > before_pointer)
	{
		return 1; //memory leak
	}

	return result;
}

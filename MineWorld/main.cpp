#include "mineworld.h"
#include <QtWidgets/QApplication>
#include "pointer_check.h"
#include "nbt.h"
#include <string>

int __main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	MineWorld w;
	w.show();
	return a.exec();
}
int main(int argc, char *argv[])
{
	int result = __main(argc, argv);

	//test for debug new
	Tag* test = debug_new TagInt("hello", 233);
	{
		TagInt t2(*(TagInt*)test);
	}
	debug_delete test;
	auto ptr_result = get_leak_memory();
	return result;
}

#include "mineworld.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MineWorld w;
	w.show();
	return a.exec();
}

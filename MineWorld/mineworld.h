#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mineworld.h"

class MineWorld : public QMainWindow
{
	Q_OBJECT

public:
	MineWorld(QWidget *parent = Q_NULLPTR);

private:
	Ui::MineWorldClass ui;
};

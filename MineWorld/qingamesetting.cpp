#include "qingamesetting.h"
#include <qpainter.h>
#include <qtimer.h>
using namespace std;
QInGameSetting::QInGameSetting(QWidget* parent, MapLoader* loader): QLabel(parent)
{
	_parent = parent;
	_loader = loader;
	_menu_title = new QLabel("Option", this);
	_menu_title->setVisible(false);
	_menu_title->setFont(QFont("Trajan Pro 3", 20));
	_menu_title->setGeometry(QRect(300, 20, 200, 30));
	_menu_title->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	QPalette p;
	p.setColor(QPalette::Foreground, Qt::white);
	_menu_title->setPalette(p);
	_back_to_screen = new QClickableLabel("Back To Screen", this, false);
	_back_to_screen->setFont(QFont("Trajan Pro 3", 16));
	_back_to_screen->setHoverColor(QColor(255, 165, 0));
	_back_to_screen->setGeometry(QRect(300, 70, 200, 30));
	_back_to_screen->setPalette(p);
	_exit = new QClickableLabel("Exit Game", this, false);
	_exit->setFont(QFont("Trajan Pro 3", 16));
	_exit->setHoverColor(QColor(255, 165, 0));
	_exit->setGeometry(QRect(300, 110, 200, 30));
	_exit->setPalette(p);
	_mini_map = nullptr;
	_position_input = new QLineEdit(this);
	_position_input->setGeometry(QRect(300, 150, 200, 25));
	_position_input->setVisible(false);

	_statistics = new QLabel(this);
	_statistics->setPalette(p);
	_statistics->setGeometry(QRect(300, 190, 400, 90));
	//_statistics->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

	connect(_back_to_screen, SIGNAL(clicked()), this, SLOT(_on_back_to_screen_clicked()));
	connect(_exit, SIGNAL(clicked()), this, SLOT(_on_exit_clicked()));
	connect(_position_input, SIGNAL(returnPressed()), this, SLOT(_on_position_input_changed()));

	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(_on_timer_tick()));
	_timer->start(25);
	_visible = false;
	QLabel::setVisible(false);
}

QInGameSetting::~QInGameSetting()
{
#define _dispose(x) if (x) delete x; x = nullptr
	_dispose(_back_to_screen);
	_dispose(_exit);
	_dispose(_mini_map);
	_dispose(_position_input);
	_dispose(_statistics);
	_dispose(_menu_title);
#undef _dispose

	if (_timer)
	{
		_timer->stop();
		delete _timer;
		_timer = nullptr;
	}
}

void QInGameSetting::show()
{
	_back_to_screen->show();
	_exit->show();
	char str_pos[400];
	sprintf_s(str_pos, "%d,%d", (int)_loader->location().x(), (int)_loader->location().y());
	_position_input->setText(str_pos);
	_position_input->show();
	char str_time_play[50];
	uint64_t time = (uint64_t)_loader->timePlayed();
	sprintf_s(str_time_play, "%-20s: %d:%02d:%02d", "time played", (int)time / 3600, (int)(time % 3600) / 60, (int)time % 60);
	char str_mine_clicked[50];
	sprintf_s(str_mine_clicked, "%-20s: %lld", "mine clicked", _loader->mineClicked());
	char str_block_cleared[50];
	sprintf_s(str_block_cleared, "%-20s: %lld", "block cleared", _loader->clearedBlock());
	char str_flag_made[50];
	sprintf_s(str_flag_made, "%-20s: %lld", "flag made", _loader->flagMade());
	char str_unknown_made[50];
	sprintf_s(str_unknown_made, "%-20s: %lld", "unknown made", _loader->unknownMade());
	sprintf_s(str_pos, "Statistics:\r\n%-30s\r\n%-30s %-30s\r\n%-30s %-30s", str_time_play, str_block_cleared, str_flag_made, str_mine_clicked, str_unknown_made);
	if (_mini_map) delete _mini_map;
	_mini_map = new QImage(_loader->renderMiniMap(100, 100));
	_statistics->setText(str_pos);
	_statistics->show();
	_menu_title->show();
}

void QInGameSetting::hide()
{
	_back_to_screen->hide();
	_exit->hide();
	_statistics->hide();
	_menu_title->hide();
	_position_input->hide();
}

void QInGameSetting::paintEvent(QPaintEvent * event)
{
	QPainter p(this);
	p.setOpacity(0.5);
	p.fillRect(rect(), Qt::black);
	p.setOpacity(1);
	if (_mini_map)
		p.drawImage(QRect((width() - 100) >> 1, 280, 100, 100), *_mini_map);
}

void QInGameSetting::resizeEvent(QResizeEvent * event)
{
#define _center(x) x->setGeometry((width() - x->width()) >> 1, x->geometry().y(), x->width(), x->height())
	_center(_back_to_screen);
	_center(_exit);
	_center(_position_input);
	_center(_statistics);
	_center(_menu_title);
#undef _center

	if (_mini_map) delete _mini_map;
	_mini_map = new QImage(_loader->renderMiniMap(100, 100));

	update();
}

void QInGameSetting::setVisible(bool visible)
{
	if (_visible == visible) return;
	_visible = visible;
	if (_visible)
		show();
	else
		hide();
	QLabel::setVisible(_visible);
}

void QInGameSetting::_on_back_to_screen_clicked()
{
	setVisible(false);
	emit backToScreenClicked();
}
void QInGameSetting::_on_exit_clicked()
{
	emit exitClicked();
}
void QInGameSetting::_on_position_input_changed()
{
	QRegExp ex("(-?\\d+)(,|\\s+)(-?\\d+)");
	if (ex.exactMatch(_position_input->text()))
	{
		int x, y;
		x = ex.cap(1).toInt();
		y = ex.cap(3).toInt();

		if (_mini_map) delete _mini_map;
		_loader->setLocation(QPointF(x, y));
		_mini_map = new QImage(_loader->renderMiniMap(100, 100));

		emit positionUpdated(x, y);
	}
}
void QInGameSetting::_on_timer_tick()
{
	if (isVisible())
	{
		char str[300];
		char str_time_play[50];
		uint64_t time = (uint64_t)_loader->timePlayed();
		sprintf_s(str_time_play, "%-20s: %d:%02d:%02d", "time played", (int)time / 3600, (int)(time % 3600) / 60, (int)time % 60);
		char str_mine_clicked[50];
		sprintf_s(str_mine_clicked, "%-20s: %lld", "mine clicked", _loader->mineClicked());
		char str_block_cleared[50];
		sprintf_s(str_block_cleared, "%-20s: %lld", "block cleared", _loader->clearedBlock());
		char str_flag_made[50];
		sprintf_s(str_flag_made, "%-20s: %lld", "flag made", _loader->flagMade());
		char str_unknown_made[50];
		sprintf_s(str_unknown_made, "%-20s: %lld", "unknown made", _loader->unknownMade());
		sprintf_s(str, "Statistics:\r\n%-30s\r\n%-30s %-30s\r\n%-30s %-30s", str_time_play, str_block_cleared, str_flag_made, str_mine_clicked, str_unknown_made);
		if (_mini_map) delete _mini_map;
		_mini_map = new QImage(_loader->renderMiniMap(100, 100));
		_statistics->setText(str);

		//update();
		_statistics->update();
		_back_to_screen->update();
		_exit->update();
	}
}
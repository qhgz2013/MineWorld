#include "mineworld.h"
#include <qpainter.h>
#include <qtimer.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "util.h"
#include <qevent.h>
#include <Windows.h>
using namespace std;

//static definations
int MineWorld::_ani_particle_count = 100;
int MineWorld::_ani_duration = 6;

MineWorld::MineWorld(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_status = 0;
	_timer = new QTimer(this);

	connect(_timer, SIGNAL(timeout()), this, SLOT(_background_color_change()));
	_timer->start(16);

	_to_gray = (unsigned char)(rand() % 80 + 175);
	_from_gray = 255;

	_next_update = fGetCurrentTimestamp() + 5;
	_ico = new QImage(":/MineWorld/icon.png");
	_is_mouse_down = false;

	_init_ani_data();

	_title = new QClickableLabel("Mine World", this);
	_title->setGeometry(QRect(40, 30, 270, 90));
	_title->setFont(QFont("Kristen ITC", 26));
	_title->setObjectName("_title");
	_title->setCursor(Qt::CursorShape::ArrowCursor);
	_title->setEnableHoverAnimation(false);
	_version = new QClickableLabel("Version: v1.00", this);
	_version->setGeometry(QRect(40, 110, 270, 15));
	_version->setFont(QFont("SimSun", 9));
	_version->setObjectName("_version");
	_version->setCursor(Qt::CursorShape::ArrowCursor);
	QPalette q = _version->palette();
	q.setColor(QPalette::Foreground, Qt::gray);
	_version->setPalette(q);
	_version->setEnableHoverAnimation(false);
	_version->setHoverColor(QColor(Qt::gray));

	_exist_svd = access(DEFAULT_SAVEDATA_PATH, IO_MODE_EXIST) != -1;
	if (_exist_svd)
	{
		_start_game = new QClickableLabel("Start Game", this);
		_start_game->setGeometry(QRect(330, 180, 160, 40));
	}
	else
	{
		//save path not exist, no start game shown
		_start_game = new QClickableLabel("Start Game", this, false);
		_start_game->setGeometry(QRect(330, 180 - 70, 160, 40));
	}
	_start_game->setFont(QFont("Trajan Pro 3", 18));
	_start_game->setObjectName("_start_game");
	_start_game->setHoverColor(QColor(255, 165, 0)); //orange
	_new_game = new QClickableLabel("New Game", this);
	_new_game->setFont(QFont("Trajan Pro 3", 18));
	_new_game->setObjectName("_new_game");
	_new_game->setHoverColor(QColor(255, 165, 0)); //orange
	_new_game->setGeometry(QRect(330, _start_game->geometry().top() + 70, 160, 40));
	_exit = new QClickableLabel("Exit", this);
	_exit->setFont(QFont("Trajan Pro 3", 18));
	_exit->setObjectName("_exit");
	_exit->setHoverColor(QColor(255, 165, 0)); //orange
	_exit->setGeometry(QRect(330, _new_game->geometry().top() + 70, 160, 40));

	//select game mode
	_game_mode_label = new QClickableLabel("Select Difficulty", this, false);
	_game_mode_label->setFont(QFont("Trajan Pro 3", 20));
	_game_mode_label->setEnableHoverAnimation(false);
	_game_mode_label->setGeometry(QRect(330, 160, 300, 40));
	_game_mode_label->setObjectName("_game_mode_label");
	_game_mode_label->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
	_easy = new QClickableLabel("Easy", this, false);
	_easy->setFont(QFont("Trajan Pro 3", 14));
	_easy->setObjectName("_easy");
	_easy->setHoverColor(QColor(255, 165, 0));
	_easy->setGeometry(QRect(330, _game_mode_label->geometry().top() + 60, 150, 30));
	_normal = new QClickableLabel("Normal", this, false);
	_normal->setFont(QFont("Trajan Pro 3", 14));
	_normal->setObjectName("_normal");
	_normal->setHoverColor(QColor(255, 165, 0));
	_normal->setGeometry(QRect(330, _easy->geometry().top() + 40, 150, 30));
	_hard = new QClickableLabel("Hard", this, false);
	_hard->setFont(QFont("Trajan Pro 3", 14));
	_hard->setObjectName("_hard");
	_hard->setHoverColor(QColor(255, 165, 0));
	_hard->setGeometry(QRect(330, _normal->geometry().top() + 40, 150, 30));
	_game_mode_back = new QClickableLabel("Back", this, false);
	_game_mode_back->setFont(QFont("Trajan Pro 3", 18));
	_game_mode_back->setHoverColor(QColor(255, 165, 0));
	_game_mode_back->setObjectName("_game_mode_back");
	_game_mode_back->setGeometry(QRect(330, _hard->geometry().top() + 50, 150, 40));

	connect(_exit, SIGNAL(clicked()), this, SLOT(_on_exit_clicked()));
	connect(_start_game, SIGNAL(clicked()), this, SLOT(_on_start_game_clicked()));
	connect(_new_game, SIGNAL(clicked()), this, SLOT(_on_new_game_clicked()));
	connect(_game_mode_back, SIGNAL(clicked()), this, SLOT(_on_game_mode_back_clicked()));
	connect(_easy, SIGNAL(clicked()), this, SLOT(_on_easy_clicked()));
	connect(_normal, SIGNAL(clicked()), this, SLOT(_on_normal_clicked()));
	connect(_hard, SIGNAL(clicked()), this, SLOT(_on_hard_clicked()));

	_loader = new MapLoader();
	_setting = new QInGameSetting(this, _loader);

	connect(_setting, SIGNAL(backToScreenClicked()), this, SLOT(_on_ingame_back_to_screen_clicked()));
	connect(_setting, SIGNAL(exitClicked()), this, SLOT(_on_ingame_exit_clicked()));
	connect(_setting, SIGNAL(positionUpdated(int, int)), this, SLOT(_on_position_updated(int, int)));
}

MineWorld::~MineWorld()
{
	if (_timer && _timer->isActive())
		_timer->stop();

#define _dispose(x) if (x) delete x; x = nullptr
	_dispose(_timer);
	_dispose(_ico);
	_dispose(_start_game);
	_dispose(_new_game);
	_dispose(_exit);
	_dispose(_title);
	_dispose(_version);
	_dispose(_game_mode_label);
	_dispose(_easy);
	_dispose(_normal);
	_dispose(_hard);
	_dispose(_game_mode_back);
	_dispose(_setting);
#undef _dispose

	_dispose_ani_data();
	if (_loader)
	{
		_loader->setWidth(0);
		_loader->setHeight(0); //restore left-top point
		delete _loader;
	}
	_loader = nullptr;
}

void MineWorld::_init_ani_data()
{
	_ani_start_time = new double[_ani_particle_count];
	_ani_from_pos = new QPoint[_ani_particle_count];
	_ani_to_pos = new QPoint[_ani_particle_count];
	_ani_omega = new double[_ani_particle_count];
	_ani_opacity = new double[_ani_particle_count];
	_ani_size = new QSize[_ani_particle_count];

	double cur_time = fGetCurrentTimestamp() + 3;
	for (int i = 0; i < _ani_particle_count; i++)
	{
		_refresh_particle(i);
	}
}

void MineWorld::_dispose_ani_data()
{
	if (_ani_omega) delete[] _ani_omega;
	_ani_omega = nullptr;
	if (_ani_from_pos) delete[] _ani_from_pos;
	_ani_from_pos = nullptr;
	if (_ani_to_pos) delete[] _ani_to_pos;
	_ani_to_pos = nullptr;
	if (_ani_start_time) delete[] _ani_start_time;
	_ani_start_time = nullptr;
	if (_ani_opacity) delete[] _ani_opacity;
	_ani_opacity = nullptr;
	if (_ani_size) delete[] _ani_size;
	_ani_size = nullptr;
}

void MineWorld::_update_particle(QPainter & p)
{
	double cur_time = fGetCurrentTimestamp();
	for (int i = 0; i < _ani_particle_count; i++)
	{
		if (cur_time > _ani_start_time[i] + _ani_duration)
			_refresh_particle(i);

		cur_time = fGetCurrentTimestamp();
		if (cur_time < _ani_start_time[i]) continue;
		double stat = (cur_time - _ani_start_time[i]) / _ani_duration;
		if (stat < 0 || stat > 1) continue;
		double posx = _ani_from_pos[i].x() + (_ani_to_pos[i].x() - _ani_from_pos[i].x()) * stat;
		double posy = _ani_from_pos[i].y() + (_ani_to_pos[i].y() - _ani_from_pos[i].y()) * stat;

		double alpha;

		if (stat < 0.3)
			alpha = (stat / 0.3)*_ani_opacity[i];
		else
			alpha = (1 - (stat - 0.3) / 0.7)*_ani_opacity[i];

		p.setOpacity(alpha);
		QTransform tr;

		double mposx = posx + _ani_size[i].width() / 2.0;
		double mposy = posy + _ani_size[i].height() / 2.0;
		tr.translate(mposx, mposy);
		double angle = fmod(stat * _ani_duration * _ani_omega[i], 360);
		tr.rotate(angle);
		tr.translate(-mposx, -mposy);

		p.setTransform(tr);
		p.drawImage(QRectF(QPointF(posx, posy), _ani_size[i]), *_ico);
	}
	p.resetTransform();
	p.setOpacity(1);
}

void MineWorld::_refresh_particle(int index)
{
	double cur_time = fGetCurrentTimestamp();
	cur_time += (rand() % 100) * 0.01 * 5;
	_ani_start_time[index] = cur_time;
	_ani_from_pos[index] = QPoint(rand() % width(), (rand() % height()) - 200 - rand() % 100);
	_ani_to_pos[index] = QPoint(_ani_from_pos[index].x(), _ani_from_pos[index].y() + 200 + rand() % 500);
	_ani_omega[index] = ((rand() % 100)*0.01 * 200 + 30);
	int size = rand() % 55 + 5;
	_ani_size[index] = QSize(size, size);
	_ani_opacity[index] = size / 60.0;
}

void MineWorld::paintEvent(QPaintEvent * event)
{
	QPainter p(this);
	double cur_time = fGetCurrentTimestamp();
	//main title
	if (_status == 0 || _status == 4)
	{

		//background
		if (cur_time >= _next_update)
		{
			_next_update += 5;
			_from_gray = _to_gray;
			_to_gray = (unsigned char)(rand() % 80 + 175);
		}
		double stat = (cur_time - _next_update + 5) / 5;
		int gray = (int)(_from_gray + (_to_gray - _from_gray) * stat);
		p.setBackground(QBrush(QColor(gray, gray, gray)));

		_update_particle(p);
	}
	//entering game
	else if (_status == 1)
	{
		const double ani_duration = 0.3;
		double window_angle = atan((width() - 1)*1.0 / (height() - 1));

		//using _ani_start_time[0]
		if (_ani_start_time[0] + ani_duration + 0.1 < cur_time)
		{
			p.fillRect(rect(), Qt::darkGray);
			_ani_start_time[0] = cur_time;
			_status = 2;
			_loader->setWidth(width());
			_loader->setHeight(height());
			return;
		}
		double stat = (cur_time - _ani_start_time[0]) / ani_duration;
		double angle = M_PI_2 * stat;

		QPainterPath path;
		path.moveTo(0, height() - 1);
		path.lineTo(0, 0);
		if (angle <= window_angle)
			path.lineTo((height() - 1) * tan(angle), 0);
		else
		{
			path.lineTo(width() - 1, 0);
			path.lineTo(width() - 1, (height() - 1) - (width() - 1) / tan(angle));
		}
		path.lineTo(0, height() - 1);

		p.fillPath(path, Qt::darkGray);
	}
	//game pre-paint
	else if (_status == 2)
	{
		_status = 3;
		update();
		return;
		//test loading animation
		p.fillRect(rect(), Qt::darkGray);
		const double ani_duration = 3.0;

		double stat = fmod(cur_time - _ani_start_time[0], ani_duration) / ani_duration;
		double alpha = abs(sin(stat * M_PI * 2));

		p.setOpacity(alpha);
		p.setPen(Qt::white);
		p.setFont(QFont("Jokerman", 30));
		p.drawText(rect(), "Loading...", QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
	}
	else if (_status == 3)
	{
		//全屏重绘函数
		//if (event->rect().width() == width() && event->rect().height() == height())
		_loader->renderMap(p, this);
	}
}

void MineWorld::resizeEvent(QResizeEvent * event)
{
#define _center(x) x->setGeometry((width() - x->width()) >> 1, x->geometry().y(), x->width(), x->height())
	_center(_start_game);
	_center(_new_game);
	_center(_exit);
	_center(_version);
	_center(_title);
	_center(_game_mode_back);
	_center(_game_mode_label);
	_center(_easy);
	_center(_normal);
	_center(_hard);
#undef _center

	if (_loader) _loader->setWidth(width());
	if (_loader) _loader->setHeight(height());
	if (_setting) _setting->setGeometry(rect());
}

void MineWorld::mousePressEvent(QMouseEvent * event)
{
	_is_mouse_down = true;
	_mouse_down_pos = event->pos();
	_mouse_last_pos = event->pos();
	_mouse_down_time = fGetCurrentTimestamp();
}

void MineWorld::mouseReleaseEvent(QMouseEvent * event)
{
	_is_mouse_down = false;
	double mouse_up_time = fGetCurrentTimestamp();
	//0.1s内无视鼠标移动事件，直接判定为点击
	if (_status == 3 && !_setting->isVisible() && ((_mouse_down_pos == event->pos() && _mouse_last_pos == event->pos()) || mouse_up_time - _mouse_down_time <= 0.1))
	{
		QPointF fblock = _loader->blockAt(event->pos());
		//char buf[100];
		//sprintf_s(buf, "mouse at (%d,%d) block: (%f,%f)\r\n", event->pos().x(), event->pos().y(), fblock.x(), fblock.y());
		//OutputDebugStringA(buf);

		QPoint block(floor(fblock.x()), floor(fblock.y()));
		if (event->button() & Qt::MouseButton::LeftButton)
		{
			_loader->clickBlock(block);
		}
		else if (event->button() & Qt::MouseButton::RightButton)
		{
			_loader->rightClickBlock(block);
		}
		update();
	}
	_mouse_last_pos = event->pos();
}

void MineWorld::mouseMoveEvent(QMouseEvent * event)
{
	double cur_time = fGetCurrentTimestamp();

	if (!_setting->isVisible() && ((cur_time - _mouse_down_time > 0.1) || abs(event->pos().x() - _mouse_last_pos.x()) + abs(event->pos().y() - _mouse_last_pos.y()) > 1))
	{
		if (_is_mouse_down && _status == 3 && _mouse_last_pos != event->pos())
		{
			_loader->setDeltaLocation(QPointF(_mouse_last_pos - event->pos()) / _loader->blockSize());
			update();
		}

		_mouse_last_pos = event->pos();
	}
}

void MineWorld::wheelEvent(QWheelEvent * event)
{
}

void MineWorld::keyPressEvent(QKeyEvent * event)
{
	if (_status == 3 && event->key() == Qt::Key_Escape)
	{
		_setting->toggleVisible();
	}
	else
		QWidget::keyPressEvent(event);
}


void MineWorld::_on_exit_clicked()
{
	QApplication::exit();
}

void MineWorld::_on_start_game_clicked()
{
	_start_game->hide();
	_new_game->hide();
	_exit->hide();
	_version->hide();
	_title->hide();

	_ani_start_time[0] = fGetCurrentTimestamp();
	_status = 1;
}

void MineWorld::_on_new_game_clicked()
{

	_start_game->hide();
	_new_game->hide();
	_exit->hide();
	_game_mode_back->show();
	_game_mode_label->show();
	_easy->show();
	_normal->show();
	_hard->show();
	_loader->clearStatistics();
	_status = 4;
}

void MineWorld::_on_easy_clicked()
{
	delete _setting;
	delete _loader;
	if (access(DEFAULT_SAVEDATA_PATH, IO_MODE_EXIST) != -1)
	{
		RecursiveDelete(DEFAULT_SAVEDATA_PATH);
	}
	_loader = new MapLoader();
	_setting = new QInGameSetting(this, _loader);
	_setting->setGeometry(rect());
	connect(_setting, SIGNAL(backToScreenClicked()), this, SLOT(_on_ingame_back_to_screen_clicked()));
	connect(_setting, SIGNAL(exitClicked()), this, SLOT(_on_ingame_exit_clicked()));
	connect(_setting, SIGNAL(positionUpdated(int, int)), this, SLOT(_on_position_updated(int, int)));
	GEN_MINE_POSSIBILITY = DEFAULT_GEN_MINE_POSSIBILITY_EASY;
	_loader->setPossibility(GEN_MINE_POSSIBILITY);
	_easy->hide();
	_normal->hide();
	_hard->hide();
	_game_mode_back->hide();
	_game_mode_label->hide();
	_version->hide();
	_title->hide();

	_ani_start_time[0] = fGetCurrentTimestamp();
	_status = 1;
}
void MineWorld::_on_normal_clicked()
{
	delete _setting;
	delete _loader;
	if (access(DEFAULT_SAVEDATA_PATH, IO_MODE_EXIST) != -1)
	{
		RecursiveDelete(DEFAULT_SAVEDATA_PATH);
	}
	_loader = new MapLoader();
	_setting = new QInGameSetting(this, _loader);
	_setting->setGeometry(rect());
	connect(_setting, SIGNAL(backToScreenClicked()), this, SLOT(_on_ingame_back_to_screen_clicked()));
	connect(_setting, SIGNAL(exitClicked()), this, SLOT(_on_ingame_exit_clicked()));
	connect(_setting, SIGNAL(positionUpdated(int, int)), this, SLOT(_on_position_updated(int, int)));
	GEN_MINE_POSSIBILITY = DEFAULT_GEN_MINE_POSSIBILITY_NORMAL;
	_loader->setPossibility(GEN_MINE_POSSIBILITY);
	_easy->hide();
	_normal->hide();
	_hard->hide();
	_game_mode_back->hide();
	_game_mode_label->hide();
	_version->hide();
	_title->hide();

	_ani_start_time[0] = fGetCurrentTimestamp();
	_status = 1;
}
void MineWorld::_on_hard_clicked()
{
	delete _setting;
	delete _loader;
	if (access(DEFAULT_SAVEDATA_PATH, IO_MODE_EXIST) != -1)
	{
		RecursiveDelete(DEFAULT_SAVEDATA_PATH);
	}
	_loader = new MapLoader();
	_setting = new QInGameSetting(this, _loader);
	_setting->setGeometry(rect());
	connect(_setting, SIGNAL(backToScreenClicked()), this, SLOT(_on_ingame_back_to_screen_clicked()));
	connect(_setting, SIGNAL(exitClicked()), this, SLOT(_on_ingame_exit_clicked()));
	connect(_setting, SIGNAL(positionUpdated(int, int)), this, SLOT(_on_position_updated(int, int)));
	GEN_MINE_POSSIBILITY = DEFAULT_GEN_MINE_POSSIBILITY_HARD;
	_loader->setPossibility(GEN_MINE_POSSIBILITY);
	_easy->hide();
	_normal->hide();
	_hard->hide();
	_game_mode_back->hide();
	_game_mode_label->hide();
	_version->hide();
	_title->hide();

	_ani_start_time[0] = fGetCurrentTimestamp();
	_status = 1;
}
void MineWorld::_on_game_mode_back_clicked()
{
	if (_exist_svd) _start_game->show();
	_new_game->show();
	_exit->show();
	_easy->hide();
	_normal->hide();
	_hard->hide();
	_game_mode_back->hide();
	_game_mode_label->hide();
	_status = 0;
}

void MineWorld::_background_color_change()
{
	//触发paint event
	if (_status >= 0 && _status < 3 || _status == 4)
		update();
}

void MineWorld::_on_ingame_back_to_screen_clicked()
{
	_dispose_ani_data();
	_init_ani_data();

	_start_game->show();
	_new_game->show();
	_exit->show();
	_version->show();
	_title->show();

	_status = 0;
}

void MineWorld::_on_ingame_exit_clicked()
{
	QApplication::exit(0);
}

void MineWorld::_on_position_updated(int new_x, int new_y)
{
	//_loader->setLocation(QPointF(new_x, new_y));
	update();
}
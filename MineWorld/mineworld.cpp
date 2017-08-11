#include "mineworld.h"
#include <qpainter.h>
#include <qtimer.h>
#include <qpropertyanimation.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "util.h"
#include <qevent.h>
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
	_loader = new MapLoader();

	_init_ani_data();

	_title = new QClickableLabel("Mine World", this);
	_title->setGeometry(QRect(40, 30, 270, 90));
	_title->setFont(QFont("Kristen ITC", 26));
	_title->setObjectName("_title");
	_title->setCursor(Qt::CursorShape::ArrowCursor);
	_title->setEnableHoverAnimation(false);
	_version = new QClickableLabel("Version: v0.01 Pre-Alpha", this);
	_version->setGeometry(QRect(40, 110, 270, 15));
	_version->setFont(QFont("SimSun", 9));
	_version->setObjectName("_version");
	_version->setCursor(Qt::CursorShape::ArrowCursor);
	QPalette q = _version->palette();
	q.setColor(QPalette::Foreground, Qt::gray);
	_version->setPalette(q);
	_version->setEnableHoverAnimation(false);
	_version->setHoverColor(QColor(Qt::gray));

	_start_game = new QClickableLabel("Start Game", this);
	_start_game->setGeometry(QRect(330, 180, 160, 40));
	_start_game->setFont(QFont("Trajan Pro 3", 18));
	_start_game->setObjectName("_start_game");
	_start_game->setHoverColor(QColor(255, 165, 0)); //orange
	_exit = new QClickableLabel("Exit", this);
	_exit->setGeometry(QRect(330, 250, 160, 40));
	_exit->setFont(QFont("Trajan Pro 3", 18));
	_exit->setObjectName("_exit");
	_exit->setHoverColor(QColor(255, 165, 0)); //orange

	connect(_exit, SIGNAL(clicked()), this, SLOT(_on_exit_clicked()));
	connect(_start_game, SIGNAL(clicked()), this, SLOT(_on_start_game_clicked()));

}

MineWorld::~MineWorld()
{
	if (_timer && _timer->isActive())
		_timer->stop();
	if (_timer) delete _timer;
	_timer = nullptr;
	if (_ico) delete _ico;
	_ico = nullptr;

	_dispose_ani_data();

	if (_start_game) delete _start_game;
	_start_game = nullptr;
	if (_exit) delete _exit;
	_exit = nullptr;
	if (_title) delete _title;
	_title = nullptr;
	if (_version) delete _version;
	_version = nullptr;

	if (_loader) delete _loader;
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
	if (_status == 0)
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
		if (_ani_start_time[0] + ani_duration < cur_time)
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
		if (event->rect().width() == width() && event->rect().height() == height())
			_loader->renderMap(p, this);
	}
}

void MineWorld::resizeEvent(QResizeEvent * event)
{
	_start_game->setGeometry((width() - _start_game->width()) >> 1, _start_game->geometry().y(), _start_game->width(), _start_game->height());
	_exit->setGeometry((width() - _exit->width()) >> 1, _exit->geometry().y(), _exit->width(), _exit->height());
	_version->setGeometry((width() - _version->width()) >> 1, _version->geometry().y(), _version->width(), _version->height());
	_title->setGeometry((width() - _title->width()) >> 1, _title->geometry().y(), _title->width(), _title->height());

	if (_loader) _loader->setWidth(width());
	if (_loader) _loader->setHeight(height());
}

void MineWorld::mousePressEvent(QMouseEvent * event)
{
}

void MineWorld::mouseReleaseEvent(QMouseEvent * event)
{
}

void MineWorld::mouseMoveEvent(QMouseEvent * event)
{
	QPointF pos = _loader->blockAt(event->pos());
	QPoint ipos = QPoint(floor(pos.x()), floor(pos.y()));

}

void MineWorld::wheelEvent(QWheelEvent * event)
{
}

void MineWorld::_on_exit_clicked()
{
	QApplication::exit();
}

void MineWorld::_on_start_game_clicked()
{
	_start_game->hide();
	_exit->hide();
	_version->hide();
	_title->hide();

	_ani_start_time[0] = fGetCurrentTimestamp();
	_status = 1;
}

void MineWorld::_background_color_change()
{
	//触发paint event
	if (_status >= 0 && _status < 3)
		update();
}
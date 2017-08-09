#include "mineworld.h"
#include <qpainter.h>
#include <qtimer.h>
#include <qpropertyanimation.h>
#include "util.h"
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

	connect(_timer, SIGNAL(timeout()), this, SLOT(background_color_change()));
	_timer->start(16);

	_to_gray = (unsigned char)(rand() % 80 + 175);
	_from_gray = 255;

	_next_update = fGetCurrentTimestamp() + 5;
	_ico = new QImage(":/MineWorld/icon.png");

	ui.graphicsView->hide();
	_init_ani_data();

}

MineWorld::~MineWorld()
{
	if (_timer && _timer->isActive())
		_timer->stop();
	if (_timer) delete _timer;
	_timer = nullptr;
	if (_ico) delete _ico;
	_ico = nullptr;

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

void MineWorld::background_color_change()
{
	//´¥·¢paint event
	update();
}
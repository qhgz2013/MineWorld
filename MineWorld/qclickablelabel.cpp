#include "qclickablelabel.h"
#include <QMouseEvent>
#include <qpainter.h>
#include "util.h"
using namespace std;

QClickableLabel::QClickableLabel(const QString& text, QWidget *parent, bool visible)
	: QLabel(parent)
{
	setText(text);
	_ctor_time = fGetCurrentTimestamp();

	_fade_duration = 0.3;
	_color_change_duration = 0.3;

	_visible = false;
	_entered = false;
	_leave_time = fGetCurrentTimestamp() - _color_change_duration;
	_enter_time = 0;
	setCursor(Qt::CursorShape::PointingHandCursor);

	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_visible = visible;
	QLabel::setVisible(_visible);
}

QClickableLabel::~QClickableLabel()
{
}

void QClickableLabel::enterEvent(QEvent* e)
{
	_entered = true;
	_enter_time = fGetCurrentTimestamp();
}

void QClickableLabel::leaveEvent(QEvent* e)
{
	_entered = false;
	_leave_time = fGetCurrentTimestamp();
}

void QClickableLabel::setVisible(bool visible)
{
	if (visible == _visible) return;
	_ctor_time = fGetCurrentTimestamp();
	if (visible)
		QLabel::setVisible(true);
	_visible = visible;
}

void QClickableLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
		emit clicked();
	QLabel::mousePressEvent(event);
}

void QClickableLabel::paintEvent(QPaintEvent * event)
{
	QPainter p(this);
	double curtime = fGetCurrentTimestamp();
	if (_visible)
	{
		if (_ctor_time + _fade_duration > curtime)
			p.setOpacity(min((curtime - _ctor_time) / _fade_duration, 1.0));
	}
	else
	{
		if (_ctor_time + _fade_duration > curtime)
			p.setOpacity(max(1 - (curtime - _ctor_time) / _fade_duration, 0.0));
		else
		{
			QLabel::setVisible(false);
			return;
		}
	}
	p.setFont(font());

	QColor fore = palette().foreground().color();
	QColor from_color, to_color;

	if (_entered)
	{
		to_color = _hover_color;
		from_color = fore;
	}
	else
	{
		from_color = _hover_color;
		to_color = fore;
	}

	//线性变换
	double stat = (curtime - (_entered ? _enter_time : _leave_time)) / _color_change_duration;
	if (stat > 1.0) stat = 1.0;
	else if (stat < 0.0)stat = 0.0;
	QColor current(
		(int)(from_color.red() + (to_color.red() - from_color.red()) * stat),
		(int)(from_color.green() + (to_color.green() - from_color.green()) * stat),
		(int)(from_color.blue() + (to_color.blue() - from_color.blue()) * stat)
	);
	
	if (_enable_hover_animation)
	{
		const int border_size = 2;
		double wh_stat = width() * 1.0 / (width() + height());
		if (width() > border_size && height() > border_size)
		{
			QPen pen(_hover_color, 1);
			pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
			p.setPen(pen);
			QPainterPath path;

			if (_entered)
			{
				if (stat <= wh_stat)
					path.moveTo(stat / wh_stat * (width() - 1), 0);
				else
				{
					path.moveTo(width() - 1, (stat - wh_stat) / (1 - wh_stat) * (height() - 1));
					path.lineTo(width() - 1, 0);
				}
				path.lineTo(0, 0);
				if (stat <= 1 - wh_stat)
					path.lineTo(0, stat / (1 - wh_stat) * (height() - 1));
				else
				{
					path.lineTo(0, height() - 1);
					path.lineTo((stat + wh_stat - 1) / wh_stat * (width() - 1), height() - 1);
				}
			}
			else
			{
				if (stat <= wh_stat)
				{
					path.moveTo(stat / wh_stat * (width() - 1), 0);
					path.lineTo(width() - 1, 0);
				}
				else
					path.moveTo(width(), (stat - wh_stat) / (1 - wh_stat)*(height() - 1));
				path.lineTo(width() - 1, height() - 1);
				if (stat <= 1 - wh_stat)
				{
					path.lineTo(0, height() - 1);
					path.lineTo(0, stat / (1 - wh_stat)*(height() - 1));
				}
				else
					path.lineTo((stat + wh_stat - 1) / wh_stat*(width() - 1), height() - 1);
			}
			p.drawPath(path);
		}
	}
	p.setPen(QPen(current));
	p.drawText(rect(), text(), QTextOption(alignment()));
}

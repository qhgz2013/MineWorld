#pragma once
#include <qobject.h>
#include <qevent.h>
#include <qwidget.h>
#include <qlineedit.h>
#include "qclickablelabel.h"
#include "maploader.h"

class QInGameSetting : public QLabel
{
	Q_OBJECT

private:
	QWidget* _parent;
	MapLoader* _loader;
	QClickableLabel* _back_to_screen;
	QClickableLabel* _exit;
	QImage* _mini_map;
	QLineEdit* _position_input;
	QLabel* _statistics;
	QLabel* _menu_title;
	QTimer* _timer;
	//static double _animation_duration;
	bool _visible;
public:
	QInGameSetting(QWidget* parent, MapLoader* loader);
	~QInGameSetting();
	void show();
	void hide();
	inline void toggleVisible() { setVisible(!isVisible()); }
protected:
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
	void setVisible(bool visible);

signals:
	void backToScreenClicked();
	void exitClicked();
	void positionUpdated(int new_x, int new_y);

private slots:
	void _on_back_to_screen_clicked();
	void _on_exit_clicked();
	void _on_position_input_changed();
	void _on_timer_tick();
};

#pragma once

#include <QLabel>

class QClickableLabel : public QLabel
{
	Q_OBJECT

public:
	QClickableLabel(const QString& text = "", QWidget *parent = nullptr);
	~QClickableLabel();
	
	inline QColor hoverColor() const { return _hover_color; }
	inline void setHoverColor(QColor& color) { _hover_color = color; }

private:
	bool _visible;
	double _fade_duration;
	double _ctor_time;
	double _color_change_duration;
	double _enter_time;
	bool _entered;
	double _leave_time;
	QColor _hover_color;
	bool _enable_hover_animation;
signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);
	void enterEvent(QEvent* e);
	void leaveEvent(QEvent* e);
	void setVisible(bool visible);

	//overloads functions
public:
	inline void show() { setVisible(true); }
	inline void hide() { setVisible(false); }
	inline double fadeDuration() const { return _fade_duration; }
	inline void setFadeDuration(double duration) { _fade_duration = duration; }
	inline double colorChangeDuration() const { return _color_change_duration; }
	inline void setColorChangeDuration(double duration) { _color_change_duration = duration; }
	inline bool enableHoverAnimation() const { return _enable_hover_animation; }
	inline void setEnableHoverAnimation(bool enable) { _enable_hover_animation = enable; }
};

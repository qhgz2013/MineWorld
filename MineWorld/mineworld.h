#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mineworld.h"
#include "qclickablelabel.h"
#include "maploader.h"

class MineWorld : public QMainWindow
{
	Q_OBJECT

public:
	MineWorld(QWidget *parent = Q_NULLPTR);
	~MineWorld();

private:
	Ui::MineWorldClass ui;
	char _status;
	QTimer* _timer;
	//背景动画: 渐变测试
	//目标灰度
	unsigned char _to_gray;
	//原始灰度
	unsigned char _from_gray;
	//下一次更新时间
	double _next_update;

	//背景动画: 图片旋转+随机测试
	QImage* _ico;
	//数量
	static int _ani_particle_count;
	//开始时间
	double* _ani_start_time;
	//持续时间
	static int _ani_duration;
	//位置
	QPoint* _ani_from_pos;
	QPoint* _ani_to_pos;
	QSize* _ani_size;
	//旋转角速度(°/s)
	double* _ani_omega;
	//不透明度
	double* _ani_opacity;
	//创建数据
	void _init_ani_data();
	void _dispose_ani_data();
	//绘制
	void _update_particle(QPainter& p);
	//重新生成
	void _refresh_particle(int index);

	//UI
	QClickableLabel* _title;
	QClickableLabel* _version;
	QClickableLabel* _start_game;
	QClickableLabel* _exit;

	//地图加载
	MapLoader* _loader;
protected:
	//窗体绘制
	void paintEvent(QPaintEvent* event);
	//大小调整
	void resizeEvent(QResizeEvent* event);
	//鼠标事件
	void mousePressEvent(QMouseEvent* event); //单击
	void mouseReleaseEvent(QMouseEvent* event); //释放
	void mouseMoveEvent(QMouseEvent* event); //移动
	void wheelEvent(QWheelEvent* event); //滑轮

private slots:
	//定时器回调函数
	void _background_color_change();
	void _on_exit_clicked();
	void _on_start_game_clicked();
};

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
	//��������: �������
	//Ŀ��Ҷ�
	unsigned char _to_gray;
	//ԭʼ�Ҷ�
	unsigned char _from_gray;
	//��һ�θ���ʱ��
	double _next_update;

	//��������: ͼƬ��ת+�������
	QImage* _ico;
	//����
	static int _ani_particle_count;
	//��ʼʱ��
	double* _ani_start_time;
	//����ʱ��
	static int _ani_duration;
	//λ��
	QPoint* _ani_from_pos;
	QPoint* _ani_to_pos;
	QSize* _ani_size;
	//��ת���ٶ�(��/s)
	double* _ani_omega;
	//��͸����
	double* _ani_opacity;
	//��������
	void _init_ani_data();
	void _dispose_ani_data();
	//����
	void _update_particle(QPainter& p);
	//��������
	void _refresh_particle(int index);

	//UI
	QClickableLabel* _title;
	QClickableLabel* _version;
	QClickableLabel* _start_game;
	QClickableLabel* _exit;

	//��ͼ����
	MapLoader* _loader;
protected:
	//�������
	void paintEvent(QPaintEvent* event);
	//��С����
	void resizeEvent(QResizeEvent* event);
	//����¼�
	void mousePressEvent(QMouseEvent* event); //����
	void mouseReleaseEvent(QMouseEvent* event); //�ͷ�
	void mouseMoveEvent(QMouseEvent* event); //�ƶ�
	void wheelEvent(QWheelEvent* event); //����

private slots:
	//��ʱ���ص�����
	void _background_color_change();
	void _on_exit_clicked();
	void _on_start_game_clicked();
};

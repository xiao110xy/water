#pragma once
#include <QLabel>
class MyQLabel : public QLabel
{
	Q_OBJECT;
public:
	MyQLabel(QWidget *parent = 0);
	~MyQLabel();
protected slots:
	void change_value(QPointF data);
};


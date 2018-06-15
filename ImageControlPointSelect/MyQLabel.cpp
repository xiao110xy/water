#include "MyQLabel.h"



MyQLabel::MyQLabel(QWidget *parent)
	: QLabel(parent)
{
}

MyQLabel::~MyQLabel()
{
}

void MyQLabel::change_value(QPointF data)
{
	QString temp = QString::number(data.x(), 'f', 2)+","+QString::number(data.y(), 'f', 2);
	this->setText(temp);
}
#ifndef WIDGET_H
#define WIDGET_H

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
using namespace std;


#include <QWidget>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTextStream>
#include <QTranslator>
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
	Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
protected:
	//! Setup the various windows which can be brought up using the main window
	void setupWindows();
	//! Establish the myriad of signal and slot connections
	void setupConnections();
private:
    Ui::Widget *ui;
	QGraphicsScene *m_graphicsScene1;
	QGraphicsScene *m_graphicsScene2;
	QImage left_qimage;
	QImage right_qimage;
signals:
	void draw_roi(QRect rect);

private slots:
	void pushbotton_load_base_image();
	void pushbotton_load_wrap_image();
	void pushbotton_get_roi();
	void pushbotton_savePts();
	void pushbotton_reloadPts();
};

#endif // WIDGET_H

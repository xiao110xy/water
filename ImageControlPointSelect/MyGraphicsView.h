#pragma once
#include <vector>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPoint>
#include <QGraphicsLineItem>

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	MyGraphicsView(QWidget *parent = 0);
	~MyGraphicsView();
	void viewFit();
    void zoomIn();
    void zoomOut();

private:
    void scaleView(qreal scaleFactor);
    bool isResized;
	bool isLandscape;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
public:
	void refresh_view();
signals:
	//! Signal is emitted when a point changed;
	void point_changed(QPointF point);
public:
	QPointF base_point;
	QGraphicsItemGroup *green_circleGroup;
	std::vector<QGraphicsItemGroup*> point_list;
	std::vector<QGraphicsTextItem*> text_list;
protected slots:
	void addDrawPoint();
	void addDrawPoint(QPointF temp_point);
	void removeDrawPoint(int row);
};

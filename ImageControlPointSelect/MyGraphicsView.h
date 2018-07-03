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
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
public:
	void refresh_view();
signals:
	//! Signal is emitted when a point changed;
	void point_changed(QPointF point);
public:
	QPointF base_point;
	QGraphicsItemGroup *green_circleGroup;
	QGraphicsRectItem *roi_Group;
	QPoint roi_left_top;
	bool m_dragged1;
	QPoint roi_right_bottom;
	bool m_dragged2;
	std::vector<QGraphicsItemGroup*> point_list;
	std::vector<QGraphicsTextItem*> text_list;

protected slots:
	void addDrawPoint();
	void addDrawPoint(QPointF temp_point);
	void removeDrawPoint(int row);
	void DrawRoi(QRect roi_rect);
	void DrawRoi();
};

#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QMenu>
#include <QMouseEvent>
//! This class inherits QMenu and provides extra functionality.
class TablePopUp : public QMenu
{
	Q_OBJECT

public:
	//! Constructor
	TablePopUp();
	//! Destructor
	virtual ~TablePopUp();

protected:
	//! Initialization of menu items
	void populateMenu();

protected slots:
	//! Menu option to remove the items that the user has selected
	void removethis();
	//! Menu option to remove all of the items that the user has not selected
	void addthis();

signals:
	//! Signal emitted to remove items from the list (if flag is true)
	void removeItems();
	void addItems();

private:
	QAction *remove;
	QAction *add;
};

class MyQTableWidget : public QTableWidget
{
	Q_OBJECT;
public:
	MyQTableWidget(QWidget *parent = 0);
	~MyQTableWidget();
protected:
	//! Re-implementation of this function from QListWidget to control right-click action
	void mouseReleaseEvent(QMouseEvent *event);
protected slots:
	void addFiles();
	void removeFiles();
	void base_point_changed(QPointF data);
	void wrap_point_changed(QPointF data);
signals:
	void add_index();
	void add_base_index(QPointF data);
	void add_wrap_index(QPointF data);
	void remove_index(int data);
public:
	TablePopUp * PopUp;
	QPointF base_point, wrap_point;
};

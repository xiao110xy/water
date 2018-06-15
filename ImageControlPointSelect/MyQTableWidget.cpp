#include "MyQTableWidget.h"

TablePopUp::TablePopUp() : QMenu()
{
	populateMenu();
}

TablePopUp::~TablePopUp()
{

}

void TablePopUp::populateMenu()
{
	/* These allow the user to exclude some of the available images from being added
	to the image lists. */
    add = new QAction(tr("add new item"), this);
    remove = new QAction(tr("Remove selected items"), this);
	connect(remove, SIGNAL(triggered(bool)), this, SLOT(removethis()));
	connect(add, SIGNAL(triggered(bool)), this, SLOT(addthis()));
	this->addAction(add);
	this->addAction(remove);
}

void TablePopUp::removethis()
{
	emit(removeItems());
}

void TablePopUp::addthis()
{
	emit(addItems());
}


MyQTableWidget::MyQTableWidget(QWidget * parent) : QTableWidget(parent)
{
	PopUp = new TablePopUp;
	connect(PopUp, SIGNAL(removeItems()), this, SLOT(removeFiles()));
	connect(PopUp, SIGNAL(addItems()), this, SLOT(addFiles()));

	base_point.setX(-999);
	base_point.setY(-999);
	wrap_point.setX(-999);
	wrap_point.setY(-999);

}

MyQTableWidget::~MyQTableWidget()
{
}

void MyQTableWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->button() == Qt::RightButton) {
		PopUp->popup(event->globalPos());

	}
}
void MyQTableWidget::addFiles()
{
	if (base_point.x() < 0 && base_point.y() < 0 &&
		wrap_point.x() < 0 && wrap_point.y() < 0) {
		return;
	}
	int row = this->rowCount();//获取表格中当前总行数
	setRowCount(row + 1);//添加一行
	setItem(row, 0, new QTableWidgetItem(QString::number(base_point.x(), 'f', 2)));
	setItem(row, 1, new QTableWidgetItem(QString::number(base_point.y(), 'f', 2)));
	setItem(row, 2, new QTableWidgetItem(QString::number(wrap_point.x(), 'f', 2)));
	setItem(row, 3, new QTableWidgetItem(QString::number(wrap_point.y(), 'f', 2)));
	emit(add_index());
}

void MyQTableWidget::removeFiles()
{
	bool focus = isItemSelected(currentItem());
	if (!focus)
		return;
	int row = this->currentItem()->row();//当前选中行
	emit(remove_index(row));
	removeRow(row);
}

void MyQTableWidget::base_point_changed(QPointF data)
{
	base_point = data;
}

void MyQTableWidget::wrap_point_changed(QPointF data)
{
	wrap_point = data;
}



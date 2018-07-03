#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
	QTranslator* translator = new QTranslator;
	QString runPath = QCoreApplication::applicationDirPath();       //获取文件运行路径
	if (translator->load(runPath + "/cn.qm")) {
		qApp->installTranslator(translator);
	}

    ui->setupUi(this);
	setupWindows();
	setupConnections();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupWindows()
{
	QImage bground(50, 50, QImage::Format_RGB888);
	for (int y = 0; y < 25; y++)
	{
		for (int x = 0; x < 25; x++)
		{
			bground.setPixel(x, y, qRgb(0xCA, 0xCA, 0xCA));
			bground.setPixel(x + 25, y, qRgb(0xFF, 0xFF, 0xFF));
			bground.setPixel(x, y + 25, qRgb(0xFF, 0xFF, 0xFF));
			bground.setPixel(x + 25, y + 25, qRgb(0xCA, 0xCA, 0xCA));
		}
	}
	//
	m_graphicsScene1 = new QGraphicsScene();
	m_graphicsScene1->setItemIndexMethod(QGraphicsScene::NoIndex);
	m_graphicsScene1->setBackgroundBrush(QPixmap::fromImage(bground));
	ui->graphicsView1->setScene(m_graphicsScene1);
	//
	m_graphicsScene2 = new QGraphicsScene();
	m_graphicsScene2->setItemIndexMethod(QGraphicsScene::NoIndex);
	m_graphicsScene2->setBackgroundBrush(QPixmap::fromImage(bground));
	ui->graphicsView2->setScene(m_graphicsScene2);
	// table 
	ui->tableWidget->setColumnCount(4);
	ui->tableWidget->setRowCount(0);
	QStringList header;
	header <<"Base X" << "Base Y" << "Wrap X" << "Wrap Y";
	ui->tableWidget->setHorizontalHeaderLabels(header);
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
}

void Widget::setupConnections()
{
	connect(this, SIGNAL(draw_roi(QRect)), ui->graphicsView2, SLOT(DrawRoi(QRect)));

	connect(ui->load_base_image, SIGNAL(clicked()), this, SLOT(pushbotton_load_base_image()));
	connect(ui->load_wrap_image, SIGNAL(clicked()), this, SLOT(pushbotton_load_wrap_image()));
	connect(ui->get_roi, SIGNAL(clicked()), this, SLOT(pushbotton_get_roi()));
    connect(ui->savePts, SIGNAL(clicked()), this, SLOT(pushbotton_savePts()));
	connect(ui->reloadPts, SIGNAL(clicked()), this, SLOT(pushbotton_reloadPts()));

	connect(ui->graphicsView1, SIGNAL(point_changed(QPointF)), ui->base_point, SLOT(change_value(QPointF)));
	connect(ui->graphicsView2, SIGNAL(point_changed(QPointF)), ui->wrap_point, SLOT(change_value(QPointF)));
	
	connect(ui->graphicsView1, SIGNAL(point_changed(QPointF)), ui->tableWidget, SLOT(base_point_changed(QPointF)));
	connect(ui->graphicsView2, SIGNAL(point_changed(QPointF)), ui->tableWidget, SLOT(wrap_point_changed(QPointF)));
	
	connect(ui->tableWidget, SIGNAL(add_index()),ui->graphicsView1, SLOT(addDrawPoint()));
	connect(ui->tableWidget, SIGNAL(add_index()),ui->graphicsView2, SLOT(addDrawPoint()));
	connect(ui->tableWidget, SIGNAL(add_base_index(QPointF)),ui->graphicsView1, SLOT(addDrawPoint(QPointF)));
	connect(ui->tableWidget, SIGNAL(add_wrap_index(QPointF)),ui->graphicsView2, SLOT(addDrawPoint(QPointF)));
	connect(ui->tableWidget, SIGNAL(remove_index(int)),ui->graphicsView1, SLOT(removeDrawPoint(int)));
	connect(ui->tableWidget, SIGNAL(remove_index(int)),ui->graphicsView2, SLOT(removeDrawPoint(int)));

}

void Widget::pushbotton_load_base_image()
{
	QString qStrFilePath = QFileDialog::getOpenFileName(this,
		tr("Open Image"),
		QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
		tr("Image Files (*.png *.jpg *.bmp)"));

	if (qStrFilePath.isEmpty())
		return;
	// 刷新
	ui->graphicsView1->refresh_view();
	ui->graphicsView2->refresh_view();
    ui->tableWidget->clearContents();//只清除表中数据，不清除表头内容
    ui->tableWidget->setRowCount(0);
	//
	//QString qStrFilePath = "C:/Users/Alexia/Desktop/template_1.bmp";
	QImageReader reader(qStrFilePath);
	if (!reader.canRead())
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Cannot read file"));
		msgBox.exec();
		return;
	}
	if (!m_graphicsScene1->sceneRect().isEmpty())
	{
		m_graphicsScene1->clear();
	}
	left_qimage = reader.read();
	m_graphicsScene1->setSceneRect(left_qimage.rect());
	m_graphicsScene1->addPixmap(QPixmap::fromImage(left_qimage));
	ui->graphicsView1->viewFit();
}

void Widget::pushbotton_load_wrap_image()
{
	QString qStrFilePath = QFileDialog::getOpenFileName(this,
		tr("Open Image"),
		QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
		tr("Image Files (*.png *.jpg *.bmp)"));
	if (qStrFilePath.isEmpty())
		return;
	//QString qStrFilePath="C:/Users/xy/Pictures/bing/20180701.jpg";
	// 刷新
    ui->graphicsView1->refresh_view();
    ui->graphicsView2->refresh_view();
    ui->tableWidget->clearContents();//只清除表中数据，不清除表头内容
    ui->tableWidget->setRowCount(0);
	//
	//QString qStrFilePath = "C:/Users/Alexia/Desktop/1.jpg";
	QImageReader reader(qStrFilePath);
	if (!reader.canRead())
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Cannot read file"));
		msgBox.exec();
		return;
	}
	if (!m_graphicsScene2->sceneRect().isEmpty())
	{
		m_graphicsScene2->clear();
	}
	right_qimage = reader.read();
	m_graphicsScene2->setSceneRect(right_qimage.rect());
	m_graphicsScene2->addPixmap(QPixmap::fromImage(right_qimage));
	ui->graphicsView2->viewFit();
}

void Widget::pushbotton_get_roi()
{
	if (ui->graphicsView2->sceneRect().isEmpty())
		return;
	QPoint roi_left_top, roi_right_bottom;
	roi_left_top.setX(0 + ui->graphicsView2->sceneRect().width() / 3);
	roi_left_top.setY(0 + ui->graphicsView2->sceneRect().height() / 3);
	roi_right_bottom.setX(2 * ui->graphicsView2->sceneRect().width() / 3);
	roi_right_bottom.setY(2 * ui->graphicsView2->sceneRect().height() / 3);
	QRect roi_rect;
	roi_rect.setTopLeft(roi_left_top);
	roi_rect.setBottomRight(roi_right_bottom);
	emit(draw_roi(roi_rect));
}

void Widget::pushbotton_savePts()
{
	QString qStrFilePath = QFileDialog::getSaveFileName(this,
		tr("save assit txt"),
		QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
		tr("Assit  Files (*.txt)"));
	//QString qStrFilePath = "C:/Users/Alexia/Desktop/assist_3.txt";
	QFile f(qStrFilePath);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		cout << "Open failed." << endl;
		return;
	}

	QTextStream Out(&f);
	int row = ui->tableWidget->rowCount();
	if (row > 0) {
		Out << "0,0,0," << QString::number(row, 10) <<";"<<endl;
		QRect temp = ui->graphicsView2->roi_Group->rect().toRect();
		Out << temp.x()<<","<<temp.y() << "," <<
			temp.width() << "," <<temp.height()<<";" << endl;
		for (int i = 0; i < row; ++i) {
			Out << ui->tableWidget->item(i, 0)->text() << ",";
			Out << ui->tableWidget->item(i, 1)->text() << ","; 
			Out << ui->tableWidget->item(i, 2)->text() << ","; 
			Out << ui->tableWidget->item(i, 3)->text() << ";"<< endl;
		}
		QMessageBox::information(this, tr("notice"),tr("file has writed"));
	}
	f.close();
}

void Widget::pushbotton_reloadPts()
{
	QString qStrFilePath = QFileDialog::getOpenFileName(this,
		tr("Open assit txt"),
		QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
		tr("Assit  Files (*.txt)"));
	//QString qStrFilePath = "C:/Users/Alexia/Desktop/assist_1.txt";
	QFile f(qStrFilePath);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)){
		cout << "Open failed." << endl;
		return;
	}
	// 刷新
    ui->graphicsView1->refresh_view();
    ui->graphicsView2->refresh_view();
    ui->tableWidget->clearContents();//只清除表中数据，不清除表头内容
    ui->tableWidget->setRowCount(0);
	//
	QTextStream txtInput(&f);
	QString LineStr= txtInput.readLine();
	vector<int> temp;
	string temp_name;
	temp_name = LineStr.toLocal8Bit();
	temp.clear();
	for (int i = 0; i < temp_name.size(); ++i) {
		int temp_value = 0;
		int j = 0;
		for (j = i; j < temp_name.size(); ++j) {
			if (temp_name[j] >= 48 && temp_name[j] <= 57) {
				temp_value = temp_value * 10 + temp_name[j] - 48;
			}
			else {
				break;
			}
		}
		i = j;
		temp.push_back(temp_value);
	}
	int nLine = temp[3];
	ui->tableWidget->clearContents();//只清除表中数据，不清除表头内容
	ui->tableWidget->setRowCount(nLine);
	LineStr = txtInput.readLine();
	temp_name = LineStr.toLocal8Bit();
	temp.clear();
	for (int i = 0; i < temp_name.size(); ++i) {
		if (temp_name[i] >= 48 && temp_name[i] <= 57) {
			double temp_value = 0;
			int j = i;
			int n = -1;
			for (; j < temp_name.size(); ++j) {
				if (temp_name[j] == 44 || temp_name[j] == 59) {
					temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
					temp.push_back(temp_value);
					break;
				}
				if (temp_name[j] >= 48 && temp_name[j] <= 57) {
					temp_value = temp_value * 10 + temp_name[j] - 48;
				}
				if (temp_name[j] == 46)
					n = j;
			}
			i = j;
		}
	}
	if (temp.size() == 4) {
		if (temp[2] > 0 && temp[3] > 0) {
			QRect temp_rect(temp[0],temp[1],temp[2],temp[3]);
			emit(draw_roi(temp_rect));
		}
	}
	for (int n = 0; n < nLine; ++n) {
		vector<double> temp;
		LineStr = txtInput.readLine();
		temp_name = LineStr.toLocal8Bit();
		for (int i = 0; i < temp_name.size(); ++i) {
			if (temp_name[i] >= 48 && temp_name[i] <= 57) {
				double temp_value = 0;
				int j = i;
				int n = -1;
				for (; j < temp_name.size(); ++j) {
					if (temp_name[j] == 44 || temp_name[j] == 59) {
						temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
						temp.push_back(temp_value);
						break;
					}
					if (temp_name[j] >= 48 && temp_name[j] <= 57) {
						temp_value = temp_value * 10 + temp_name[j] - 48;
					}
					if (temp_name[j] == 46)
						n = j;
				}
				i = j;
			}
		}
		ui->tableWidget->base_point.setX(temp[0]);
		ui->tableWidget->base_point.setY(temp[1]);
		ui->tableWidget->wrap_point.setX(temp[2]);
		ui->tableWidget->wrap_point.setY(temp[3]);
		ui->tableWidget->setItem(n, 0, new QTableWidgetItem(QString::number(temp[0], 'f', 2)));
		ui->tableWidget->setItem(n, 1, new QTableWidgetItem(QString::number(temp[1], 'f', 2)));
		ui->tableWidget->setItem(n, 2, new QTableWidgetItem(QString::number(temp[2], 'f', 2)));
		ui->tableWidget->setItem(n, 3, new QTableWidgetItem(QString::number(temp[3], 'f', 2)));
		emit(ui->tableWidget->add_base_index(ui->tableWidget->base_point));
		emit(ui->tableWidget->add_wrap_index(ui->tableWidget->wrap_point));
	}

	f.close();
}

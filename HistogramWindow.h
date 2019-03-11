#ifndef _HistogramWindow_H
#define _HistogramWindow_H

#include <QtWidgets/QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QVector>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

#include "PCX.h"

QT_CHARTS_USE_NAMESPACE

class HistogramWindow : public QWidget
{
	Q_OBJECT

public :
	HistogramWindow(img::Histogram &&h, QWidget *parent = Q_NULLPTR) : histogram{ std::move(h) }, QWidget(parent)
	{
		this->setWindowFlag(Qt::FramelessWindowHint);

		color_planes_num = histogram.size();

		createBarSets();

		createBarSeries();

		createAxis();

		createChart();

		createChartView();

		createCheckBoxes();

		createLayout();

		createGroupBox();
	}

	void resizeEvent(QResizeEvent *ev)
	{
		group_box->setGeometry(QRect{ 0, height() - check_box_height, width(), check_box_height });

		chart_view->setGeometry(0, 0, width(), height() - check_box_height);
	}

public slots:

	void box0Change_slot(int state)
	{
		if (state == Qt::CheckState::Checked)
		{
			sets[0]->setColor(QColor{ color_weight,0,0 });
			sets[0]->setBorderColor(QColor{ color_weight,0,0 });
		}
		else
		{
			sets[0]->setColor(QColor{ 0,0,0,0 });
			sets[0]->setBorderColor(QColor{ 0,0,0,0 });
		}

		update();
	}

	void box1Change_slot(int state)
	{
		if (state == Qt::CheckState::Checked)
		{
			sets[1]->setColor(QColor{ 0,color_weight,0 });
			sets[1]->setBorderColor(QColor{ 0,color_weight,0 });
		}
		else
		{
			sets[1]->setColor(QColor{ 0,0,0,0 });
			sets[1]->setBorderColor(QColor{ 0,0,0,0 });
		}

		update();
	}

	void box2Change_slot(int state)
	{
		if (state == Qt::CheckState::Checked)
		{
			sets[2]->setColor(QColor{ 0,0,color_weight });
			sets[2]->setBorderColor(QColor{ 0,0,color_weight });
		}
		else
		{
			sets[2]->setColor(QColor{ 0,0,0,0 });
			sets[2]->setBorderColor(QColor{ 0,0,0,0 });
		}

		update();
	}


private:

	const int color_weight{ 220 };

	int color_planes_num;
	img::Histogram histogram;

	QChartView *chart_view;
	QChart *chart;
	QVector<QBarSet*> sets;
	QBarSeries *series;
	QBarCategoryAxis *axis;

	QGroupBox *group_box;
	QHBoxLayout *bottom_layout;
	QVector<QCheckBox*> check_boxes;

	const int check_box_height = 35;


private:

	void createBarSets(void)
	{
		switch (color_planes_num)
		{
		case 1:
			sets.push_back(new QBarSet{ "Grey Level" ,this });

			sets[0]->setColor(QColor{ color_weight,0,0 });
			sets[0]->setBorderColor(QColor{ color_weight,0,0 });
			break;

		case 3:
			sets.push_back(new QBarSet{ "R" ,this });
			sets.push_back(new QBarSet{ "G" ,this });
			sets.push_back(new QBarSet{ "B" ,this });

			sets[0]->setColor(QColor{ color_weight,0,0 });
			sets[0]->setBorderColor(QColor{ color_weight,0,0 });

			sets[1]->setColor(QColor{ 0,color_weight,0 });
			sets[1]->setBorderColor(QColor{ 0,color_weight,0 });

			sets[2]->setColor(QColor{ 0,0,color_weight });
			sets[2]->setBorderColor(QColor{ 0,0,color_weight });
		}

		for (int c = 0; c < color_planes_num; ++c)
		{
			auto &set = sets[c];

			for (int k = 0; k < 256; ++k)
				set->append(histogram[c][k].second);
		}
	}

	void createBarSeries(void)
	{
		series = new QBarSeries{ this };

		for (int i = 0; i < color_planes_num; ++i)
		{
			series->append(sets[i]);
		}
	}

	void createAxis(void)
	{
		axis = new QBarCategoryAxis{ this };
	}

	void createChart(void)
	{
		chart = new QChart;

		chart->setTitle("Histogram");
		chart->setAnimationOptions(QChart::SeriesAnimations);
		chart->addSeries(series);
		chart->createDefaultAxes();
		chart->setAxisX(axis, series);

		chart->legend()->setVisible(false);
	}

	void createChartView(void)
	{
		chart_view = new QChartView{ chart, this };
		chart_view->setRenderHint(QPainter::Antialiasing);

		
	}

	void createCheckBoxes(void)
	{
		switch (color_planes_num)
		{
		case 1:
			check_boxes.push_back(new QCheckBox{ "Gray Level",this });

			check_boxes[0]->setChecked(true);

			QObject::connect(check_boxes[0], SIGNAL(stateChanged(int)), this, SLOT(box0Change_slot(int)));
			break;

		case 3:
			check_boxes.push_back(new QCheckBox{ "R",this });
			check_boxes.push_back(new QCheckBox{ "G",this });
			check_boxes.push_back(new QCheckBox{ "B",this });

			check_boxes[0]->setChecked(true);
			check_boxes[1]->setChecked(true);
			check_boxes[2]->setChecked(true);

			QObject::connect(check_boxes[0], SIGNAL(stateChanged(int)), this, SLOT(box0Change_slot(int)));
			QObject::connect(check_boxes[1], SIGNAL(stateChanged(int)), this, SLOT(box1Change_slot(int)));
			QObject::connect(check_boxes[2], SIGNAL(stateChanged(int)), this, SLOT(box2Change_slot(int)));

			break;
		}
	}

	void createLayout(void)
	{
		bottom_layout = new QHBoxLayout{ this };

		for (int i = 0; i < color_planes_num; ++i)
			bottom_layout->addWidget(check_boxes[i]);
	}

	void createGroupBox(void)
	{
		group_box = new QGroupBox{ this };

		group_box->resize(300, check_box_height);

		group_box->setLayout(bottom_layout);
	}
};



#endif	// end _HistogramWindow_H

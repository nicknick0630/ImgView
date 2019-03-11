#ifndef _ThresholdWindow_H
#define _ThresholdWindow_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QVector>
#include "ui_ThresholdWindow.h"

#include <vector>
#include <string>

class ThresholdWindow : public QWidget
{
	Q_OBJECT

public:
	ThresholdWindow(QWidget *parent = Q_NULLPTR) : QWidget(parent)
	{
		ui.setupUi(this);

		initialButtom();

		add();

		QObject::connect(ui.sentData, SIGNAL(clicked()), this, SIGNAL(storeDataToOrgData()));
		QObject::connect(ui.undo, SIGNAL(clicked()), this, SIGNAL(undoData()));
	}

	~ThresholdWindow() 
	{
		for (int i = 0; i < slider_sets.size(); ++i)
		{
			delete slider_sets[i];
			delete label_sets[i];
		}
	}

	const std::vector<int>& getThresholds(void) const
	{
		return sliders_value;
	}

signals:

	void clickSentThresholdsBottum(void);

	void storeDataToOrgData(void);

	void undoData(void);

public slots:

	void add(void)
	{
		slider_sets.push_back(new QSlider{ Qt::Horizontal,this });
		slider_sets[slider_sets.size() - 1]->setRange(0, 255);
		QObject::connect(slider_sets[slider_sets.size() - 1], SIGNAL(valueChanged(int)), this, SLOT(showText(int)));
		QObject::connect(slider_sets[slider_sets.size() - 1], SIGNAL(valueChanged(int)), this, SIGNAL(clickSentThresholdsBottum()));

		label_sets.push_back(new QLabel{ this });
		sliders_value.push_back(0);

		calculateSlidersValue();



		int cnt = sliders_value.size() - 1;

		ui.top_layout->addWidget(slider_sets[cnt], cnt, 0);
		ui.top_layout->addWidget(label_sets[cnt], cnt, 1);

		this->update();
	}

	void eliminate(void)
	{
		if (slider_sets.size() == 1)
			return;

		int cnt = slider_sets.size() - 1;

		ui.top_layout->removeWidget(slider_sets[cnt]);
		ui.top_layout->removeWidget(label_sets[cnt]);

		delete slider_sets[cnt];
		delete label_sets[cnt];

		slider_sets.pop_back();
		label_sets.pop_back();
		sliders_value.pop_back();

		calculateSlidersValue();
	}

	void showText(int v)
	{
		int idx;
		bool flag = false;

		for (idx = 0; idx < slider_sets.size(); ++idx)
			if (slider_sets[idx]->hasFocus())
			{
				flag = true;
				break;
			}


		if (flag)
		{
			if (idx > 0 && slider_sets[idx]->value() <= slider_sets[idx - 1]->value())
			{
				slider_sets[idx]->setValue(slider_sets[idx - 1]->value() + 1);
				return;
			}

			if (idx < slider_sets.size() - 1 && slider_sets[idx]->value() >= slider_sets[idx + 1]->value())
			{
				slider_sets[idx]->setValue(slider_sets[idx + 1]->value() - 1);
				return;
			}



			label_sets[idx]->setText(QString{ std::to_string(v).c_str() });
			sliders_value[idx] = v;
		}
	}

private:
	Ui::ThresholdWindow ui;

	QVector<QSlider*> slider_sets;
	QVector<QLabel*> label_sets;
	
	std::vector<int> sliders_value;

private:
	void initialButtom(void)
	{
		ui.addSlider->setText(QStringLiteral("增加"));
		ui.deleteSlider->setText(QStringLiteral("刪除"));
		ui.sentData->setText(QStringLiteral("確定"));
		ui.undo->setText(QStringLiteral("回復"));
	}

	void calculateSlidersValue(void)
	{
		int dif = 255 / (slider_sets.size() + 1);
		int num;

		for (unsigned i = 0; i < slider_sets.size(); ++i)
		{
			QObject::disconnect(slider_sets[i], SIGNAL(valueChanged(int)), this, SIGNAL(clickSentThresholdsBottum()));

			num = (i + 1) * dif;

			slider_sets[i]->setValue(num);
			label_sets[i]->setText(QString{ std::to_string(num).c_str() });
			sliders_value[i] = num;

			QObject::connect(slider_sets[i], SIGNAL(valueChanged(int)), this, SIGNAL(clickSentThresholdsBottum()));
		}
	}
};


#endif	// end _ThresholdWindow_H
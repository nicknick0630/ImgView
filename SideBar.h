#ifndef _SideBar_H
#define _SideBar_H

#include <QWidget>
#include <QLabel>
#include <QSplitter>
#include <QPaintEvent>
#include <QTabWidget>
#include "ui_SideBar.h"
#include "PCX.h"

class SideBar : public QWidget
{
	Q_OBJECT

public:
	SideBar(QWidget *parent = Q_NULLPTR) : QWidget(parent)
	{
		ui.setupUi(this);

		this->setWindowFlag(Qt::FramelessWindowHint);

		this->setStyleSheet("background-color:white");

		createTabWindowws();

		createSplitter();
	}

	~SideBar() {}

public:

	void resizeEvent(QResizeEvent *ev)
	{
		calculateCoordinate();

		splitter->setGeometry(0, 0, width(), height());

		upper_tab_window->setGeometry(0, 0, width(), upper_height);
		lower_tab_window->setGeometry(0, upper_height, width(), lower_height);
	}

public: /*-------- public method --------*/

	void addUpperBlock(QWidget *widget, const QString &tag_name)
	{
		upper_tab_window->addTab(widget, tag_name);
	}

	void addLowerBlock(QWidget *widget, const QString &tag_name)
	{
		lower_tab_window->addTab(widget, tag_name);
	}

	void removeFromUpperBlock(QWidget *widget)
	{
		removeFromUpperBlock(upper_tab_window->indexOf(widget));
	}

	void removeFromUpperBlock(int idx)
	{
		if (idx < 0 || idx >= upper_tab_window->count())
			throw "no this widget in upper window of side bar";

		/*auto ptr = upper_tab_window->widget(idx);

		if (ptr->objectName() == "ThresholdWindow")
		{
			upper_tab_window->removeTab(idx);
			delete ptr;
		}*/

		upper_tab_window->removeTab(idx);
	}

	void removeFromLowerBlock(QWidget *widget)
	{
		removeFromLowerBlock(lower_tab_window->indexOf(widget));
	}

	void removeFromLowerBlock(int idx)
	{
		if (idx < 0 || idx >= lower_tab_window->count())
			throw "no this widget in lower window of side bar";

		lower_tab_window->removeTab(idx);
	}

	void setToHide(void)
	{
		is_show = false;
		this->hide();
	}

	void setToShow(void)
	{
		is_show = true;
		this->show();
	}

public slots:

	void upperWindowClose_slot(int index)
	{
		removeFromUpperBlock(index);
	}

	void lowerWindowClose_slot(int index)
	{
		removeFromLowerBlock(index);
	}

	void changeShowAndHide_slot(void)
	{
		if (is_show)
		{
			this->hide();
			is_show = false;
		}
		else
		{
			this->show();
			is_show = true;
		}
	}

private:
	Ui::SideBar ui;

	QSplitter *splitter;

	QTabWidget *upper_tab_window;
	QTabWidget *lower_tab_window;

	QLabel *img_header_imformation_block{ nullptr };
	QLabel *palette_block{ nullptr };

	int upper_height;
	int lower_height;

	bool is_show{ false };


private: /*-------- initializion --------*/

	void createSplitter(void)
	{
		splitter = new QSplitter{ Qt::Orientation::Vertical,this };

		splitter->addWidget(upper_tab_window);
		splitter->addWidget(lower_tab_window);


		splitter->setCollapsible(0, false);
		splitter->setCollapsible(1, false);
	}

	void createTabWindowws(void)
	{
		upper_tab_window = new QTabWidget{ this };
		upper_tab_window->setTabPosition(QTabWidget::South);
		upper_tab_window->setMovable(false);
		upper_tab_window->setTabBarAutoHide(true);
		upper_tab_window->setTabsClosable(true);

		upper_tab_window->setMinimumHeight(425);

		QObject::connect(upper_tab_window, SIGNAL(tabCloseRequested(int)), this, SLOT(upperWindowClose_slot(int)));


		lower_tab_window = new QTabWidget{ this };
		lower_tab_window->setTabPosition(QTabWidget::South);
		lower_tab_window->setMovable(true);
		lower_tab_window->setTabsClosable(true);

		lower_tab_window->setMinimumHeight(325);

		QObject::connect(lower_tab_window, SIGNAL(tabCloseRequested(int)), this, SLOT(lowerWindowClose_slot(int)));
	}

private: /*-------- --------*/

	void calculateCoordinate(void)
	{
		upper_height = this->height() * 4 / 7;
		lower_height = this->height() - upper_height;
	}
};

#endif	// end _SideBar_H
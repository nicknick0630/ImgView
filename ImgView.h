#pragma once

#include <QtWidgets/QMainWindow>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QFileDialog>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QCombobox>

#include <string>

#include <QDebug>

#include "ui_ImgView.h"
#include "Page.h"
#include "SideBar.h"
#include "ThresholdWindow.h"
#include "FractalWindow.h"

class ImgView : public QMainWindow
{

	Q_OBJECT

public:
	ImgView(QWidget *parent = Q_NULLPTR) : QMainWindow(parent)
	{
		ui.setupUi(this);

		createActions();

		createMenus();

		createToolBar();

		createStatusBar();

		createTabWidget();

		createSideBar();

		createOther();

		// global setting
		QObject::connect(open_file_action, SIGNAL(triggered()), this, SLOT(openFile_slot()));
		QObject::connect(fractal_pannel_open_action, SIGNAL(triggered()), this, SLOT(openFractalPannel_slot()));
	}

	virtual ~ImgView() { }

	void paintEvent(QPaintEvent *ev)
	{
		// check all action should be disabled or enabled
		checkMenuAndToolIsValid();

		// set height
		_setLayoutData();

		// set pages coordinate
		pages->setGeometry(0, menu_and_tool_bar_height, pages_width, pages_height);

		// set side bar coordinate
		sidebar->setGeometry(pages_width, menu_and_tool_bar_height, max_sideBar_width, pages_height);	// there use side_bar_width not max_sideBar_width

	}

	bool eventFilter(QObject *obj, QEvent *ev)
	{
		if (obj == now_page && ev->type() == QEvent::MouseMove)
		{
			QMouseEvent *e = reinterpret_cast<QMouseEvent*>(ev);

			QPoint point = now_page->transformPageXYToImgXY(e->pos());

			updataStastusBar(point);

			this->update();
		}

		return false;
	}

	ThresholdWindow* getThresholdWindow(void)
	{
		return threshold_window;
	}

public: /*-------- creation function --------*/

	void createActions(void)
	{
		_fileMenuActionCreation();

		_fractalMenuActionCreation();

		_menuImageSettingActionCreation();

		_toolBarActionsCreation();
	}

	void createMenus(void)
	{
		// file menu
		file_menu = menuBar()->addMenu(QStringLiteral("檔案"));
		file_menu->addAction(open_file_action);
		file_menu->addAction(store_to_old_action);
		file_menu->addAction(store_to_new_action);
		file_menu->addSeparator();
		file_menu->addAction(exit_action);

		// rotate setting menu
		rotate_setting_menu = menuBar()->addMenu(QStringLiteral("旋轉設定"));
		rotate_setting_menu->addAction(inverseRotate_setting_action);
		rotate_setting_menu->addAction(directRotate_setting_action);

		// approximation setting menu
		approximation_setting_menu = menuBar()->addMenu(QStringLiteral("近似設定"));
		approximation_setting_menu->addAction(BilinearInterpolation_setting_action);
		approximation_setting_menu->addAction(NearestNeighbor_setting_action);

		// fractal option
		fractal_option_menu = menuBar()->addMenu(QStringLiteral("碎形操作"));
		fractal_option_menu->addAction(fractal_pannel_open_action);
	}

	void createToolBar(void)
	{
		// color operation
		color_tool_bar = addToolBar(QStringLiteral("color operation"));
		color_tool_bar->setMovable(false);

		color_tool_bar->addAction(complement_action);
		color_tool_bar->addAction(gray_level_action);
		color_tool_bar->addAction(bit_planes_action);
		color_tool_bar->addAction(gray_code_bit_planes_action);
		color_tool_bar->addAction(color_planes_action);
		color_tool_bar->addAction(histogram_equalization_action);
		color_tool_bar->addAction(set_threshold_action);
		color_tool_bar->addAction(otsu_threshold_action);
		color_tool_bar->addAction(add_watermark_action);
		color_tool_bar->addAction(image_overlay_action);

		// rotate operation
		rotate_tool_bar = addToolBar(QStringLiteral("rotate and flip"));
		rotate_tool_bar->setMovable(false);

		rotate_tool_bar->addAction(r_rotate_action);
		rotate_tool_bar->addAction(l_rotate_action);
		rotate_tool_bar->addAction(set_org_angle_action);
		rotate_tool_bar->addSeparator();
		rotate_tool_bar->addAction(horizontal_flip_action);
		rotate_tool_bar->addAction(vertical_flip_action);


		// zoom operation
		zoom_tool_bar = addToolBar(QStringLiteral("zooming"));
		zoom_tool_bar->setMovable(false);

		zoom_tool_bar->addAction(large_action);
		zoom_tool_bar->addAction(small_action);
		zoom_tool_bar->addAction(set_org_size_action);


		// filter operation
		filter_tool_bar = addToolBar(QStringLiteral("filter"));
		filter_tool_bar->setMovable(false);
	}

	void createStatusBar(void)
	{
		_statusBarElementsCreation();

		statusBar()->addWidget(coordinate_status_bar_element);
		statusBar()->addWidget(color_block_status_bar_element);
		statusBar()->addWidget(rgb_status_bar_element);
		statusBar()->addWidget(zoom_size_status_bar_element);
		statusBar()->addWidget(rotate_degree_status_bar_element);

		statusBar()->addPermanentWidget(sideBar_control_status_bar_element);
	}

	void createTabWidget(void)
	{
		pages = new QTabWidget{ this };

		pages->setWindowFlag(Qt::FramelessWindowHint);

		pages->setTabsClosable(true);


		// connect
		QObject::connect(pages, SIGNAL(tabCloseRequested(int)), this, SLOT(removePage_slot(int)));
		QObject::connect(pages, SIGNAL(currentChanged(int)), this, SLOT(connectActionsToCurrentPage_slot(int)));
	}

	void createSideBar(void)
	{
		sidebar = new SideBar{ this };

		// at beginning, set to hide
		sidebar->setToHide();
	}

	void createOther(void)
	{
		threshold_window = new ThresholdWindow{ this };
		threshold_window->hide();
		QObject::connect(threshold_window, SIGNAL(clickSentThresholdsBottum()), this, SLOT(recieveThresholdWindowSignal_slot()));
		QObject::connect(threshold_window, SIGNAL(storeDataToOrgData()), this, SLOT(storeThresholdDataToOrgImg_slot()));
		QObject::connect(threshold_window, SIGNAL(undoData()), this, SLOT(undoTresholdData_slot()));
	}

public slots :

	// connect to file menu open
	void openFile_slot(void)
	{
		QString file_name{ QFileDialog::getOpenFileName(this, QStringLiteral("選擇影像") , QStringLiteral("../Image/") , QStringLiteral("Image (*.pcx *.png *.jepg *.jpg *.bmp);")) };
	
		if (!file_name.isEmpty())
		{
			std::string fpath{ file_name.toStdString() };
			std::string fname{ fpath.substr(fpath.find_last_of('/') + 1) };

			pages->addTab(new Page{ fpath,fname,this }, fname.c_str());


			pages->show();

			// set to show the new page
			pages->setCurrentIndex(pages->count() - 1);
		}
	}

	// connect to file menu save
	void store_to_old_slot(void)
	{
		now_page->store_img(1);
	}

	// connect to file menu save
	void store_to_new_slot(void)
	{
		QString fpath = QFileDialog::getSaveFileName(this, QStringLiteral("儲存到新影像"), "../Image/", "Images ( *.pcx )");

		std::string file{ fpath.toStdString() };

		std::string type{ file.substr(file.find_last_of('.') + 1) };

		// default to store to pcx
		if (type.size() == 0)
			now_page->store_img(2, fpath + ".pcx");
		else if(type == "pcx")
			now_page->store_img(2, fpath);
	}

	// connect to pages close request
	void removePage_slot(int idx)
	{
		disconnectActionFromPrePage_idx(idx);

		// store the pointer to deleted page
		auto p = static_cast<Page*>(pages->widget(idx));

		// remove from tab
		pages->removeTab(idx);

		// release deleted page memory
		delete p;

		// clear status bar
		coordinate_status_bar_element->setText(QString{ "(X,Y) = ( --- , --- )" });
		color_block_status_bar_element->setStyleSheet("background-color:rgb(240,240,240)");
		rgb_status_bar_element->setText(QString{ "RGB = ( ---,---,--- )" });
		zoom_size_status_bar_element->setText(QStringLiteral("縮放 : ") + QString{ "---%" });
		rotate_degree_status_bar_element->setText(QStringLiteral("角度 : ") + QString{ "---" } +QStringLiteral(" 度"));
	}

	// connect to changing page
	void connectActionsToCurrentPage_slot(int idx)
	{
		// if no element, break
		if (idx < 0)
			return;

		// if is not the first element, disconnect from previous page
		if (pages->count() != 1)
			disconnectActionFromPrePage(now_page);


		// record now page
		now_page = static_cast<Page*>(pages->widget(idx));


		// set event filter to catch mouse event
		now_page->installEventFilter(this);


		// update something in status  bar
		updateZoomNumber_slot();
		updateAngleNumber_slot();

		// connect header window to side bar
		sidebar->addUpperBlock(now_page->getHeaderWindow(), now_page->getName().c_str());

		// connect palette window to side bar
		if (now_page->hasPaletteWindow())
			sidebar->addLowerBlock(now_page->getPaletteWindow(), (now_page->getName() + " palette").c_str());

		// connect histogram window to side bar
		sidebar->addLowerBlock(now_page->getHistogramWindow(), "histogram");


		// set Approximation flags
		switch (now_page->getApproximation())
		{
		case img::Approximation::BilinearInterpolation:
			BilinearInterpolation_setting_action->setChecked(true);
			break;

		case img::Approximation::NearestNeighbor:
			NearestNeighbor_setting_action->setChecked(true);
			break;
		}

		// set rotate flags
		switch (now_page->getRotateType())
		{
		case img::RotateType::INVERSE:
			inverseRotate_setting_action->setChecked(true);
			break;

		case img::RotateType::DIRECT:
			directRotate_setting_action->setChecked(true);
			break;
		}


		// color operation
		QObject::connect(complement_action, SIGNAL(triggered()), now_page, SLOT(setComplement_change_slot()));
		QObject::connect(gray_level_action, SIGNAL(triggered()), now_page, SLOT(setGrayLevel_change_slot()));
		QObject::connect(bit_planes_action, SIGNAL(triggered()), now_page, SLOT(getBitPlanes_slot()));
		QObject::connect(gray_code_bit_planes_action, SIGNAL(triggered()), now_page, SLOT(getGrayCodeBitPlanes_slot()));
		QObject::connect(color_planes_action, SIGNAL(triggered()), now_page, SLOT(getColorPlanes_slot()));
		QObject::connect(histogram_equalization_action, SIGNAL(triggered()), now_page, SLOT(histogramEqualization_change_slot()));
		QObject::connect(set_threshold_action, SIGNAL(triggered()), this, SLOT(pressSetThresholdsButtom_slot()));
		QObject::connect(otsu_threshold_action, SIGNAL(triggered()), now_page, SLOT(otsu_change_slot()));
		QObject::connect(add_watermark_action, SIGNAL(triggered()), now_page, SLOT(addWatermark_change_slot()));
		QObject::connect(image_overlay_action, SIGNAL(triggered()), now_page, SLOT(showImageOverlayWindow_slot()));
	

		
		QObject::connect(complement_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::connect(gray_level_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::connect(histogram_equalization_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::connect(add_watermark_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::connect(otsu_threshold_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));


		// zoom operation
		QObject::connect(large_action, SIGNAL(triggered()), now_page, SLOT(large_slot()));
		QObject::connect(small_action, SIGNAL(triggered()), now_page, SLOT(small_slot()));
		QObject::connect(set_org_size_action, SIGNAL(triggered()), now_page, SLOT(org_size_slot()));

		QObject::connect(large_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));
		QObject::connect(small_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));
		QObject::connect(set_org_size_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));


		// rotation operation
		QObject::connect(r_rotate_action, SIGNAL(triggered()), now_page, SLOT(rotate_right_90_slot()));
		QObject::connect(l_rotate_action, SIGNAL(triggered()), now_page, SLOT(rotate_left_90_slot()));
		QObject::connect(set_org_angle_action, SIGNAL(triggered()), now_page, SLOT(org_angle_slot()));

		QObject::connect(r_rotate_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));
		QObject::connect(l_rotate_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));
		QObject::connect(set_org_angle_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));


		// flip operaton
		QObject::connect(horizontal_flip_action, SIGNAL(triggered()), now_page, SLOT(horizontalFlip_change_slot()));
		QObject::connect(vertical_flip_action, SIGNAL(triggered()), now_page, SLOT(verticalFlip_change_slot()));


		// rotate menu
		QObject::connect(BilinearInterpolation_setting_action, SIGNAL(triggered()), now_page, SLOT(setApproximationBilinear_slot()));
		QObject::connect(NearestNeighbor_setting_action, SIGNAL(triggered()), now_page, SLOT(setApproximationNearest_slot()));
		QObject::connect(inverseRotate_setting_action, SIGNAL(triggered()), now_page, SLOT(setRotateTypeInverse_slot()));
		QObject::connect(directRotate_setting_action, SIGNAL(triggered()), now_page, SLOT(setRotateTypeDirect_slot()));


		// file menu
		QObject::connect(store_to_new_action, SIGNAL(triggered()), this, SLOT(store_to_new_slot()));
		QObject::connect(store_to_old_action, SIGNAL(triggered()), this, SLOT(store_to_old_slot()));
	}

	// connect to zooming operation and new,delete page
	void updateZoomNumber_slot(void)
	{
		zoom_size_status_bar_element->setText(QStringLiteral("縮放 : ") + std::to_string(now_page->getMultiple()).c_str() + "%");

		this->update();
	}

	// connect to rotation operation and new,delete page
	void updateAngleNumber_slot(void)
	{
		rotate_degree_status_bar_element->setText(QStringLiteral("角度 : ") + std::to_string(now_page->getAngleDegree()).c_str() +QStringLiteral(" 度"));

		this->update();
	}

	// connect to update histogram at side bar
	void updateHistogramWindow_slot(void)
	{
		sidebar->removeFromLowerBlock(now_page->getHistogramWindow());

		now_page->updateHistogramWindow();

		sidebar->addLowerBlock(now_page->getHistogramWindow(),"Histogram");
	}

	// connect to open or hide side bar button at status bar
	void changeSideBarShowOrHide_slot(bool flag)
	{
		if (flag)
			sidebar->setToShow();
		else
			sidebar->setToHide();
	}



	// connect to set_threshold_action
	void pressSetThresholdsButtom_slot(void)
	{
		sidebar->addUpperBlock(threshold_window, QStringLiteral("設定 thresholds"));
	}

	// connect to threshold window , will call its function to get data and sent to the page
	void recieveThresholdWindowSignal_slot(void)
	{
		now_page->setThresholds(threshold_window->getThresholds());

		updateHistogramWindow_slot();
	}

	// connect to threshold window
	void storeThresholdDataToOrgImg_slot(void)
	{
		now_page->storeThresholdDataToOrgData(threshold_window->getThresholds());
	}

	// connect to threshold window
	void undoTresholdData_slot(void)
	{
		now_page->undoTresholdData();

		updateHistogramWindow_slot();
	}


	// connect to fractal open action at fractal menu
	void openFractalPannel_slot(void)
	{
		FractalWindow *w = new FractalWindow(this);

		w->show();
	}

private: /*-------- inner data --------*/

	Ui::ImgViewClass ui;

	SideBar *sidebar;

	QTabWidget *pages;
	Page *now_page{ nullptr };		// save the now page, if add a new page ,this will record pre page

	ThresholdWindow *threshold_window;

	// layout data
	int menu_and_tool_bar_height;	// menu_bar_height + tool_bar_height
	int pages_height;				// the pages widget height
	int pages_width;
	int now_sideBar_width;
	const int max_sideBar_width{ 300 };

	// menu
	QMenu *file_menu;
	QMenu *rotate_setting_menu;
	QMenu *approximation_setting_menu;
	QMenu *fractal_option_menu;

	// tool bar
	QToolBar *rotate_tool_bar;
	QToolBar *zoom_tool_bar;
	QToolBar *color_tool_bar;
	QToolBar *filter_tool_bar;

	// status bar element
	QLabel *coordinate_status_bar_element;
	QLabel *rgb_status_bar_element;
	QLabel *color_block_status_bar_element;
	QLabel *zoom_size_status_bar_element;
	QLabel *rotate_degree_status_bar_element;
	QPushButton *sideBar_control_status_bar_element;

	// file menu actions
	QAction *open_file_action;
	QAction *store_to_old_action;
	QAction *store_to_new_action;
	QAction *exit_action;

	// img operation actions
	QAction *complement_action;
	QAction *gray_level_action;
	QAction *bit_planes_action;
	QAction *gray_code_bit_planes_action;
	QAction *color_planes_action;
	QAction *histogram_equalization_action;
	QAction *set_threshold_action;
	QAction *otsu_threshold_action;
	QAction *add_watermark_action;
	QAction *image_overlay_action;

	QAction *large_action;
	QAction *small_action;
	QAction *set_org_size_action;
	QAction *r_rotate_action;
	QAction *l_rotate_action;
	QAction *set_org_angle_action;
	QAction *horizontal_flip_action;
	QAction *vertical_flip_action;

	//QAction *box_filter_action;


	// img setting action groups
	QActionGroup *approximation_action_group;
	QActionGroup *rotate_setting_action_group;

	// img setting actions
	QAction *BilinearInterpolation_setting_action;
	QAction *NearestNeighbor_setting_action;
	QAction *directRotate_setting_action;
	QAction *inverseRotate_setting_action;

	// other
	QAction *fractal_pannel_open_action;


private: /*-------- inner data initialization --------*/

	void _fileMenuActionCreation(void)
	{
		// open an image
		open_file_action = new QAction{ QIcon{"../Resources/Icon/open_img.png"}, QStringLiteral("開啟影像") ,this };

		open_file_action->setShortcut(QStringLiteral("Ctrl+O"));


		// store to old image
		store_to_old_action = new QAction{ QIcon{"../Resources/Icon/save.png"}, QStringLiteral("儲存") ,this };

		store_to_old_action->setShortcut(QStringLiteral("Ctrl+S"));

		// store to new image
		store_to_new_action = new QAction{ QIcon{"../Resources/Icon/save_new.png"}, QStringLiteral("另存新檔") ,this };

		store_to_new_action->setShortcut(QStringLiteral("Ctrl+Shift+S"));

		// exit process
		exit_action = new QAction{ QIcon{"../Resources/Icon/exit.png"}, QStringLiteral("結束") ,this };

		store_to_new_action->setShortcut(QStringLiteral("Ctrl+Q"));
	}

	void _fractalMenuActionCreation(void)
	{
		fractal_pannel_open_action = new QAction{ QIcon{"../Resources/Icon/fractal.png"}, QStringLiteral("開啟碎形面板") ,this };
	}

	void _menuImageSettingActionCreation(void)
	{
		// setting
		BilinearInterpolation_setting_action = new QAction{ QStringLiteral("雙線性內插法"),this };
		BilinearInterpolation_setting_action->setCheckable(true);
		NearestNeighbor_setting_action = new QAction{ QStringLiteral("鄰近鄰居法"),this };
		NearestNeighbor_setting_action->setCheckable(true);
		directRotate_setting_action = new QAction{ QStringLiteral("正向旋轉法"),this };
		directRotate_setting_action->setCheckable(true);
		inverseRotate_setting_action = new QAction{ QStringLiteral("逆向旋轉法"),this };
		inverseRotate_setting_action->setCheckable(true);

		// approxmation setting group
		approximation_action_group = new QActionGroup{ this };
		approximation_action_group->addAction(BilinearInterpolation_setting_action);
		approximation_action_group->addAction(NearestNeighbor_setting_action);
		approximation_action_group->setExclusive(true);

		// rotate setting group
		rotate_setting_action_group = new QActionGroup{ this };
		rotate_setting_action_group->addAction(inverseRotate_setting_action);
		rotate_setting_action_group->addAction(directRotate_setting_action);
		rotate_setting_action_group->setExclusive(true);
	}

	void _toolBarActionsCreation(void)
	{
		// zoom image
		large_action = new QAction{ QIcon{"../Resources/Icon/large.png"}, QStringLiteral("放大"),this };
		small_action = new QAction{ QIcon{"../Resources/Icon/small.png"}, QStringLiteral("縮小"),this };
		set_org_size_action = new QAction{ QIcon{"../Resources/Icon/original_size.png"}, QStringLiteral("恢復原始大小"),this };

		// rotate
		r_rotate_action = new QAction{ QIcon{"../Resources/Icon/rotate_right.png"}, QStringLiteral("向右旋轉"),this };
		l_rotate_action = new QAction{ QIcon{"../Resources/Icon/rotate_left.png"}, QStringLiteral("向左旋轉"),this };
		set_org_angle_action = new QAction{ QIcon{"../Resources/Icon/original_angle.png"}, QStringLiteral("恢復原始角度"),this };

		// flip
		horizontal_flip_action = new QAction{ QIcon{"../Resources/Icon/hor_flip.png"}, QStringLiteral("水平翻轉"),this };
		vertical_flip_action = new QAction{ QIcon{"../Resources/Icon/ver_flip.png"}, QStringLiteral("垂直翻轉"),this };

		// filter
		//box_filter_action = new QAction{ QIcon{"../Resources/Icon/L.png"}, QStringLiteral("低通濾波"),this };

		// other operation
		complement_action = new QAction{ QIcon{"../Resources/Icon/complement.png"}, QStringLiteral("負片"),this };
		gray_level_action = new QAction{ QIcon{"../Resources/Icon/gray.png"}, QStringLiteral("轉成灰階"),this };
		bit_planes_action = new QAction{ QIcon{"../Resources/Icon/bit_planes.png"},QStringLiteral("獲取 bit planes"),this };
		gray_code_bit_planes_action = new QAction{ QIcon{"../Resources/Icon/graycode.png"},QStringLiteral("gray code 的 bit planes"),this };
		color_planes_action = new QAction{ QIcon{"../Resources/Icon/rgb.png"},QStringLiteral("獲取 RGB 分量圖"),this };
		histogram_equalization_action = new QAction{ QIcon{"../Resources/Icon/histogram_eq.png"},QStringLiteral("Histogram Equalization"),this };
		set_threshold_action = new QAction{ QIcon{"../Resources/Icon/threshold.png"},QStringLiteral("設定 Thresholds"),this };
		otsu_threshold_action = new QAction{ QIcon{"../Resources/Icon/otsu.png"},QStringLiteral("Otsu method"),this };
		add_watermark_action = new QAction{ QIcon{"../Resources/Icon/seal.png"},QStringLiteral("加入浮水印"),this };
		image_overlay_action = new QAction{ QIcon{"../Resources/Icon/overlay.png"},QStringLiteral("疊加2張影像"),this };
	}

	void _statusBarElementsCreation(void)
	{
		coordinate_status_bar_element = new QLabel{ this };
		coordinate_status_bar_element->setText(QString{ "(X,Y) = ( --- , --- )" });
		coordinate_status_bar_element->setFixedWidth(140);

		color_block_status_bar_element = new QLabel{ this };
		color_block_status_bar_element->setStyleSheet("background-color:rgb(240,240,240)");
		color_block_status_bar_element->setFixedWidth(25);

		rgb_status_bar_element = new QLabel{ this };
		rgb_status_bar_element->setText(QString{ "RGB = ( ---,---,--- )" });
		rgb_status_bar_element->setFixedWidth(180);

		zoom_size_status_bar_element = new QLabel{ this };
		zoom_size_status_bar_element->setText(QStringLiteral("縮放 : ") + QString{ "---%" });
		zoom_size_status_bar_element->setFixedWidth(120);

		rotate_degree_status_bar_element = new QLabel{ this };
		rotate_degree_status_bar_element->setText(QStringLiteral("角度 : ") + QString{ "---" } +QStringLiteral(" 度"));
		rotate_degree_status_bar_element->setFixedWidth(120);

		sideBar_control_status_bar_element = new QPushButton{ this };
		sideBar_control_status_bar_element->setText(QStringLiteral("資訊面板"));
		sideBar_control_status_bar_element->setFixedWidth(80);
		sideBar_control_status_bar_element->setCheckable(true);
		sideBar_control_status_bar_element->setChecked(false);
		QObject::connect(sideBar_control_status_bar_element, SIGNAL(toggled(bool)), this, SLOT(changeSideBarShowOrHide_slot(bool)));
	}


private: /*-------- update status function --------*/

	void updataStastusBar(QPoint point)
	{
		if ((point.x() == -100 && point.y() == -100) || now_page->getAngleDegree() % 360 != 0)
		{
			coordinate_status_bar_element->setText(QString{ "(X,Y) = ( --- , --- )" });
			color_block_status_bar_element->setStyleSheet("background-color:rgb(240,240,240)");
			rgb_status_bar_element->setText(QString{ "RGB = ( ---,---,--- )" });
		}
		else
		{
			coordinate_status_bar_element->setText(QString{ "(X,Y) = (" } +std::to_string(point.x()).c_str() + " , " + std::to_string(point.y()).c_str() + " )");

			QColor clr = now_page->getColor(point);

			QString color_str{ QString{"( " } +
				std::to_string(clr.red()).c_str() + " , " +
				std::to_string(clr.green()).c_str() + " , " +
				std::to_string(clr.blue()).c_str() + " )" };

			color_block_status_bar_element->setStyleSheet(QString{ "background-color:rgb" } +color_str);

			rgb_status_bar_element->setText(QString{ "RGB = " } +color_str);
		}
	}


	void _setLayoutData(void)
	{
		menu_and_tool_bar_height = menuBar()->height() + rotate_tool_bar->height();
		pages_height = height() - menu_and_tool_bar_height - statusBar()->height();

		// check if side bar should be set to 0
		now_sideBar_width = sideBar_control_status_bar_element->isChecked() ? max_sideBar_width : 0;

		pages_width = width() - now_sideBar_width;
	}


	void checkMenuAndToolIsValid(void)
	{
		if (pages->count())
		{
			_setMenuAndToolStatus(true);
		}
		else
		{
			_setMenuAndToolStatus(false);
		}
	}

	void _setMenuAndToolStatus(bool flag)
	{
		// menu
		rotate_setting_menu->setEnabled(flag);
		approximation_setting_menu->setEnabled(flag);

		// tool bar
		rotate_tool_bar->setEnabled(flag);
		zoom_tool_bar->setEnabled(flag);
		color_tool_bar->setEnabled(flag);
		filter_tool_bar->setEnabled(flag);

		// other independant action
		store_to_new_action->setEnabled(flag);
		store_to_old_action->setEnabled(flag);

		// set side bar
		if (!flag && sideBar_control_status_bar_element->isChecked())
			sideBar_control_status_bar_element->setChecked(false);
	}


	void disconnectActionFromPrePage(Page *page)
	{
		// if it is now_page, remove from side bar
		if (page == now_page)
		{
			// remove header window from side bar
			sidebar->removeFromUpperBlock(page->getHeaderWindow());

			// remove palette window from side bar
			if (page->hasPaletteWindow())
				sidebar->removeFromLowerBlock(page->getPaletteWindow());

			// remove histogram window from side bar
			sidebar->removeFromLowerBlock(page->getHistogramWindow());
		}

		// remove enent mouse filter
		QObject::removeEventFilter(page);

		// color operation
		QObject::disconnect(complement_action, SIGNAL(triggered()), page, SLOT(setComplement_change_slot()));
		QObject::disconnect(gray_level_action, SIGNAL(triggered()), now_page, SLOT(setGrayLevel_change_slot()));
		QObject::disconnect(bit_planes_action, SIGNAL(triggered()), now_page, SLOT(getBitPlanes_slot()));
		QObject::disconnect(gray_code_bit_planes_action, SIGNAL(triggered()), now_page, SLOT(getGrayCodeBitPlanes_slot()));
		QObject::disconnect(color_planes_action, SIGNAL(triggered()), now_page, SLOT(getColorPlanes_slot()));
		QObject::disconnect(histogram_equalization_action, SIGNAL(triggered()), now_page, SLOT(histogramEqualization_change_slot()));
		QObject::disconnect(set_threshold_action, SIGNAL(triggered()), this, SLOT(pressSetThresholdsButtom_slot()));
		QObject::disconnect(otsu_threshold_action, SIGNAL(triggered()), now_page, SLOT(otsu_change_slot()));
		QObject::disconnect(add_watermark_action, SIGNAL(triggered()), now_page, SLOT(addWatermark_change_slot()));
		QObject::disconnect(image_overlay_action, SIGNAL(triggered()), now_page, SLOT(showImageOverlayWindow_slot()));

		QObject::disconnect(complement_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::disconnect(gray_level_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::disconnect(histogram_equalization_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::disconnect(add_watermark_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));
		QObject::disconnect(otsu_threshold_action, SIGNAL(triggered()), this, SLOT(updateHistogramWindow_slot()));

		// zoom operation
		QObject::disconnect(large_action, SIGNAL(triggered()), page, SLOT(large_slot()));
		QObject::disconnect(small_action, SIGNAL(triggered()), page, SLOT(small_slot()));
		QObject::disconnect(set_org_size_action, SIGNAL(triggered()), page, SLOT(org_size_slot()));

		QObject::disconnect(large_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));
		QObject::disconnect(small_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));
		QObject::disconnect(set_org_size_action, SIGNAL(triggered()), this, SLOT(updateZoomNumber_slot()));

		// rotation operation
		QObject::disconnect(r_rotate_action, SIGNAL(triggered()), page, SLOT(rotate_right_90_slot()));
		QObject::disconnect(l_rotate_action, SIGNAL(triggered()), page, SLOT(rotate_left_90_slot()));
		QObject::disconnect(set_org_angle_action, SIGNAL(triggered()), page, SLOT(org_angle_slot()));

		QObject::disconnect(r_rotate_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));
		QObject::disconnect(l_rotate_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));
		QObject::disconnect(set_org_angle_action, SIGNAL(triggered()), this, SLOT(updateAngleNumber_slot()));

		// flip operaton
		QObject::disconnect(horizontal_flip_action, SIGNAL(triggered()), page, SLOT(horizontalFlip_change_slot()));
		QObject::disconnect(vertical_flip_action, SIGNAL(triggered()), page, SLOT(verticalFlip_change_slot()));

		// rotate menu
		QObject::disconnect(BilinearInterpolation_setting_action, SIGNAL(triggered()), page, SLOT(setApproximationBilinear_slot()));
		QObject::disconnect(NearestNeighbor_setting_action, SIGNAL(triggered()), page, SLOT(setApproximationNearest_slot()));
		QObject::disconnect(inverseRotate_setting_action, SIGNAL(triggered()), page, SLOT(setRotateTypeInverse_slot()));
		QObject::disconnect(directRotate_setting_action, SIGNAL(triggered()), page, SLOT(setRotateTypeDirect_slot()));

		// file menu
		QObject::disconnect(store_to_new_action, SIGNAL(triggered()), this, SLOT(store_to_new_slot()));
		QObject::disconnect(store_to_old_action, SIGNAL(triggered()), this, SLOT(store_to_old_slot()));
	}

	void disconnectActionFromPrePage_idx(int idx)
	{
		disconnectActionFromPrePage(static_cast<Page*>(pages->widget(idx)));
	}


private: /*-------- helper function --------*/

	void hideSideBar(void)
	{
		sideBar_control_status_bar_element->setChecked(false);
		sidebar->setToHide();
	}

	void showSideBar(void)
	{
		sideBar_control_status_bar_element->setChecked(true);
		sidebar->setToShow();
	}
};

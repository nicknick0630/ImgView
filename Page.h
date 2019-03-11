#ifndef _Page_H
#define _Page_H

#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QMessageBox>
#include <QDialog>
#include <QMouseEvent>
#include <PCX.h>

#include <thread>

#include "ui_Page.h"
#include "HeaderFrame_PCX.h"
#include "PaletteWindow.h"
#include "BitPlanesWindow.h"
#include "HistogramWindow.h"
#include "ColorPlanesWindow.h"
#include "OverLayWindow.h"

class Page : public QLabel
{

	Q_OBJECT

public:
	Page(const std::string &fpath, const std::string &name, QWidget *parent = Q_NULLPTR) : QLabel{ parent }, path{ fpath }, fname{ name }
	{
		ui.setupUi(this);

		this->setMouseTracking(true);

		// get format
		img_format = path.substr(path.find_last_of('.') + 1);

		// open image
		openImageGetHeaderAndPixelsAndPalette();

		// backup original data
		auto td = backupData();

		// transfer image data to QImage
		transferDataToQImage();

		// calculate coordinate
		calculateQImageCoordinate();

		// generate oringinal histogram
		generateHistogramWindow(operation_data);

		if (td.joinable())
			td.join();
	}


	virtual ~Page()
	{
		/*if (is_changed)
		{
			QMessageBox::warning(this, QStringLiteral("消息框"), QStringLiteral("這是警告消息框"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
		}*/
	}

public:

	void paintEvent(QPaintEvent *ev)
	{
		//updata coordinate
		calculateQImageCoordinate();

		QPainter p{ this };

		if (!qimg.isNull())
			p.drawImage(img_start.x(), img_start.y(), qimg);
	}

	QPoint transformPageXYToImgXY(QPoint coor) const
	{
		int x = round(1.0 * (coor.x() - center.x()) / (multiple / 100.0)) + org_data.getCol() / 2;
		int y = round(1.0 * (coor.y() - center.y()) / (multiple / 100.0)) + org_data.getRow() / 2;

		if (x < 0 || y < 0 || x >= org_data.getCol() || y >= org_data.getRow())
			return QPoint{ -100,-100 };
		else
			return QPoint{ x,y };
	}

public: /*-------- get inner data functions --------*/

	img::Approximation getApproximation(void) const
	{
		return global_approx;
	}

	img::RotateType getRotateType(void) const
	{
		return global_rotate_type;
	}

	int getMultiple(void) const
	{
		return multiple;
	}

	int getAngleDegree(void) const
	{
		return now_angle_dgree;
	}

	QColor getColor(QPoint point) const
	{
		if (org_data.getColor() == 3)
			return QColor{ org_data[point.y()][point.x()][0] ,org_data[point.y()][point.x()][1] ,org_data[point.y()][point.x()][2] };
		else
			return QColor{ org_data[point.y()][point.x()][0] ,0,0 };
	}

	HeaderFrame_PCX* getHeaderWindow(void)
	{
		return header_window;
	}

	PaletteWindow* getPaletteWindow(void)
	{
		return palette_window;
	}

	const std::string& getName(void) const
	{
		return fname;
	}

	bool hasPaletteWindow(void) const
	{
		return has_palette_window;
	}

	HistogramWindow* getHistogramWindow(void)
	{
		return histogram_window;
	}

	QString getFilePath(void) const
	{
		return path.c_str();
	}

public: /*-------- set inner data functions --------*/

	void setApproximation(img::Approximation approx)
	{
		global_approx = approx;
	}

	void setRotateType(img::RotateType type)
	{
		global_rotate_type = type;
	}

	void updateHistogramWindow(void)
	{
		generateHistogramWindow(operation_data);
	}


public: /*-------- public method --------*/

	// will be called by imgview , it acts likes a slot function
	void setThresholds(const std::vector<int> &values)
	{
		operation_data = std::move(img::ImgFunction::setThreshold(org_data, values));

		transferDataToQImage();

		this->update();
	}

	void storeThresholdDataToOrgData(const std::vector<int> &values)
	{
		org_data = std::move(img::ImgFunction::setThreshold(org_data, values));
	}

	void undoTresholdData(void)
	{
		operation_data = org_data;

		transferDataToQImage();

		this->update();
	}

	void store_img(int store_type, const QString &npath = QString{}) const
	{
		img::PCX nimage{ operation_data };

		switch (store_type)
		{
		case 1:	// store to old
			nimage.store(path);
			break;

		case 2:	// store to new
			nimage.store(npath.toStdString());
			break;
		}

		nimage.close();
	}



public slots: /*-------- image operation slot functions --------*/

	/*	-------------------------------------------------------------------------
		if the function will change the org_data and set is_changed flag true,
		  then the function name will have a "change" flag .                      
		------------------------------------------------------------------------- */

	void setComplement_change_slot(void)
	{
		// complement both operation_data and org_data

		img::ImgFunction::setComplementaryColor(operation_data);

		std::thread td{ [&]()
		{
			img::ImgFunction::setComplementaryColor(org_data);
		} };

		td.detach();

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}

	void setGrayLevel_change_slot(void)
	{
		// transform gray both operation_data and org_data

		img::ImgFunction::transformToGrayLevel(operation_data);
		
		std::thread td{ [&]()
		{
			img::ImgFunction::transformToGrayLevel(org_data);
		} };

		td.detach();

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}

	void getBitPlanes_slot(void)
	{
		BitPlanesWindow bit_plane_window { operation_data , 1 };

		bit_plane_window.exec();
	}

	void getGrayCodeBitPlanes_slot(void)
	{
		BitPlanesWindow bit_plane_window{ operation_data , 2 };

		bit_plane_window.exec();
	}

	void getColorPlanes_slot(void)
	{
		ColorPlanesWindow color_plane_window{ operation_data };

		color_plane_window.exec();
	}

	void histogramEqualization_change_slot(void)
	{
		img::ImgFunction::histogramEqualization(operation_data);

		std::thread td{ [&]()
		{
			img::ImgFunction::histogramEqualization(org_data);
		} };

		td.detach();

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}

	void otsu_change_slot(void)
	{
		img::ImgFunction::otsuThresholdMethod(operation_data);

		std::thread td{ [&]()
		{
			img::ImgFunction::otsuThresholdMethod(org_data);
		} };

		td.detach();

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}


	void addWatermark_change_slot(void)
	{
		img::ImgFunction::addWatermark(operation_data);

		std::thread td{ [&]()
		{
			img::ImgFunction::histogramEqualization(org_data);
		} };

		td.detach();

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();

		// show the watermark bit plane
		QDialog *wd = new QDialog{ this };
		QLabel *label = new QLabel{ wd };

		auto data = img::ImgFunction::getBitPlane(operation_data, 1);

		int h = data.getRow();
		int w = data.getCol();
		auto format = (data.getColor() == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);

		QImage qimg{ w,h,format };

		uchar *ptr;

		for (unsigned r = 0; r < h; ++r)
		{
			ptr = qimg.scanLine(r);

			for (unsigned c = 0; c < w; ++c)
				for (unsigned color = 0; color < data.getColor(); ++color)
					*(ptr++) = data[r][c][color];
		}

		label->setPixmap(QPixmap::fromImage(qimg));

		wd->resize(w+40, h+40);
		label->setGeometry(20, 20, w, h);

		wd->exec();
	}

	void showImageOverlayWindow_slot(void)
	{
		OverLayWindow w{ operation_data };

		w.exec();
	}


	void large_slot(void)
	{
		resizeImg(Zoom::LARGE);
	}

	void small_slot(void)
	{
		resizeImg(Zoom::SMALL);
	}

	void org_size_slot(void)
	{
		resizeImg(Zoom::ORIGINAL);
	}



	void rotate_right_90_slot(void)
	{
		rotateImg(Rotate::RIGHT, -10 );
	}

	void rotate_left_90_slot(void)
	{
		rotateImg(Rotate::LEFT, 10 );
	}

	void org_angle_slot(void)
	{
		rotateImg(Rotate::ORIGINAL, 0);
	}


	void horizontalFlip_change_slot(void)
	{
		// flip both operation_data and org_data

		img::ImgFunction::horizontalFlip(operation_data);
		img::ImgFunction::horizontalFlip(org_data);

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}

	void verticalFlip_change_slot(void)
	{
		// flip both operation_data and org_data

		img::ImgFunction::verticalFlip(operation_data);
		img::ImgFunction::verticalFlip(org_data);

		// changed
		is_changed = true;

		transferDataToQImage();

		this->update();
	}


	void setApproximationBilinear_slot(void)
	{
		global_approx = img::Approximation::BilinearInterpolation;
	}

	void setApproximationNearest_slot(void)
	{
		global_approx = img::Approximation::NearestNeighbor;
	}

	void setRotateTypeInverse_slot(void)
	{
		global_rotate_type = img::RotateType::INVERSE;
	}
	
	void setRotateTypeDirect_slot(void)
	{
		global_rotate_type = img::RotateType::DIRECT;
	}

private: /*-------- inner data --------*/

	Ui::Page ui;

	// header imformation window
	HeaderFrame_PCX *header_window;

	// palette window
	PaletteWindow *palette_window{ nullptr };
	bool has_palette_window{ false };

	// histogram window
	HistogramWindow *histogram_window{ nullptr };

	// file information
	std::string path;
	std::string fname;
	std::string img_format;

	// matrix data
	img::Matrix org_data;
	img::Matrix operation_data;

	// output image
	QImage qimg;

	// coordinate data
	QPoint center;
	QPoint img_start;

	// calculation flags
	img::Approximation global_approx{ img::Approximation::BilinearInterpolation };
	img::RotateType global_rotate_type{ img::RotateType::INVERSE };

	// status data
	const std::vector<int> multiple_times{ 10,17,25,30,50,70,100,150,200,250,300,400 };
	int multiple{ 100 };
	int now_angle_dgree{ 0 };

	bool is_changed{ false };	// some operation will need to check if want to save

private: /*-------- inner enumeration and constant data type --------*/

	enum class Zoom { LARGE, SMALL, ORIGINAL };
	enum class Rotate { RIGHT, LEFT, ORIGINAL };
	double const PI = 3.14159265359L;


private: /*-------- every image types for open image and get inner data --------*/

	void openImageGetHeaderAndPixelsAndPalette(void)
	{
		if (img_format == "pcx" || img_format == "PCX")
		{
			img::PCX image{ path };

			_PCX_getHeaderImformation(image.getHeader());

			_PCX_openImgAndGetPixelData(image);

			_PCX_getPalette(image);
		}
		else
			throw "format not match";
	}

	void _PCX_openImgAndGetPixelData(img::PCX &image)
	{
		operation_data = std::move(image.getImgPixelData_r());
	}

	void _PCX_getHeaderImformation(const img::PCXHeader &header)
	{
		header_window = new HeaderFrame_PCX{ header,this };

		header_window->hide();
	}

	void _PCX_getPalette(img::PCX &image)
	{
		auto &p = image.getExtendPlette();
		switch (image.getPaletteType())
		{
		case img::PCX::PaletteType::EXTEND_PALETTE :
			palette_window = new PaletteWindow{ image.getExtendPlette(),this };
			palette_window->hide();
			has_palette_window = true;
			break;

		case img::PCX::PaletteType::ORG_PALETTE :
			palette_window = new PaletteWindow{ _getHeaderPalette(image.getHeader()),this };
			palette_window->hide();
			has_palette_window = true;
			break;

		default :
			palette_window = nullptr;
			has_palette_window = false;
			break;
		}
	}


	img::PCX::Palette _getHeaderPalette(const img::PCXHeader &header) const
	{
		img::PCX::Palette palette;

		for (unsigned i = 0; i < 16; ++i)
			palette.push_back(std::vector<img::BYTE>{header.Palette[i], header.Palette[i + 1], header.Palette[i + 2]});

		return palette;
	}

private: /*-------- help function --------*/

	void transferDataToQImage(void)
	{
		QImage::Format format{ ((operation_data.getColor() == 1) ? QImage::Format_Grayscale8 : QImage::Format_RGB888) };

		qimg = QImage(operation_data.getCol(), operation_data.getRow(), format);

		unsigned r, c, color;


		for (r = 0; r < operation_data.getRow(); ++r)
		{
			// use ScanLine , because QImage inner data will be set alignment by 4 and prone to error
			uchar *ptr = qimg.scanLine(r);

			for (c = 0; c < operation_data.getCol(); ++c)
				for (color = 0; color < operation_data.getColor(); ++color)
					*(ptr++) = operation_data[r][c][color];
		}
	}

	void calculateQImageCoordinate(void)
	{
		// 置中設定

		center.setX(this->width() / 2);
		center.setY(this->height() / 2);

		img_start.setX(center.x() - operation_data.getCol() / 2);
		img_start.setY(center.y() - operation_data.getRow() / 2);
	}

	std::thread backupData(void)
	{
		std::thread td{ [&]()
		{
			org_data = operation_data;
		} };

		return td;
	}

	double degreeToRad(int deg) const
	{
		return deg / 180.0 * PI;
	}

	void generateHistogramWindow(const img::Matrix &data)
	{
		if (histogram_window)
			delete histogram_window;

		histogram_window = new HistogramWindow{ img::ImgFunction::getHistogram(data),this };

		histogram_window->hide();
	}


private: /*-------- image operation functions --------*/

	void resizeImg(Page::Zoom type)
	{
		// first : resize from org_data, and store in to operation_data
		// second : rotate the operation_data if needed

		int mul_idx = std::find(multiple_times.cbegin(), multiple_times.cend(), multiple) - multiple_times.cbegin();

		if (mul_idx == multiple_times.size())
			throw "multiple times error";

		bool is_zooming = true;

		switch (type)
		{
		case Zoom::LARGE :
			if (mul_idx != multiple_times.size() - 1)
			{
				multiple = multiple_times[++mul_idx];

				if (multiple == 100)
					operation_data = org_data;
				else
					operation_data = img::ImgFunction::resize(org_data, multiple / 100.0, global_approx);
			}
			else
				is_zooming = false;
			break;

		case Zoom::SMALL :
			if (mul_idx != 0)
			{
				multiple = multiple_times[--mul_idx];

				if (multiple == 100)
					operation_data = org_data;
				else
					operation_data = img::ImgFunction::resize(org_data, multiple / 100.0, global_approx);
			}
			else
				is_zooming = false;
			break;

		case Zoom::ORIGINAL :
			if (multiple != 100)
			{
				multiple = 100;

				operation_data = org_data;
			}
			else
				is_zooming = false;
			break;
		}

		if(is_zooming)
			operation_data = img::ImgFunction::rotate(operation_data, degreeToRad(now_angle_dgree), global_rotate_type, global_approx);

		transferDataToQImage();

		this->update();
	}

	void rotateImg(Page::Rotate type, double angle)
	{
		// first : rotate the angle from org_data, and store in operation_data
		// second : resize the operation_data if needed

		switch (type)
		{
		case Page::Rotate::RIGHT : 
		case Page::Rotate::LEFT :
			now_angle_dgree += angle;
			operation_data = img::ImgFunction::rotate(org_data, degreeToRad(now_angle_dgree), global_rotate_type, global_approx);
			break;

		case Page::Rotate::ORIGINAL :
			operation_data = org_data;
			now_angle_dgree = 0;
			break;
		}

		if(multiple != 100)
			operation_data = img::ImgFunction::resize(operation_data, multiple / 100.0, global_approx);

		transferDataToQImage();

		this->update();
	}
};


#endif	// end _Page_H
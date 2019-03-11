#ifndef _ColorPlanesWindow_H
#define _ColorPlanesWindow_H

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include "ui_ColorPlanesWindow.h"

#include "PCX.h"

class ColorPlanesWindow : public QDialog
{

	Q_OBJECT

public:
	ColorPlanesWindow(const img::Matrix &pdata, QWidget *parent = Q_NULLPTR) : QDialog(parent), pictures(pdata.getColor())
	{
		ui.setupUi(this);

		QSize s{ 307,641 };

		img_h = s.height();
		img_w = s.width();

		switch (findDecisionSide(pdata.getRow(), pdata.getCol()))
		{
		case 0:	// height
			op_data = img::ImgFunction::resize(pdata, h_times);
			break;

		case 1:	// width
			op_data = img::ImgFunction::resize(pdata, w_times);
			break;
		}

		img_h = op_data.getRow();
		img_w = op_data.getCol();

		switch (pictures.size())
		{
		case 1:
			pictures[0] = QImage{ img_w,img_h,QImage::Format_Grayscale8 };

			transferPixelsToQimg(op_data, pictures[0], 0);

			ui.plane2->setPixmap(QPixmap::fromImage(pictures[0]));
			break;

		case 3:
			pictures[0] = QImage{ img_w,img_h,QImage::Format_RGB888 };
			pictures[1] = QImage{ img_w,img_h,QImage::Format_RGB888 };
			pictures[2] = QImage{ img_w,img_h,QImage::Format_RGB888 };

			transferPixelsToQimg(op_data, pictures[0], 0);
			transferPixelsToQimg(op_data, pictures[1], 1);
			transferPixelsToQimg(op_data, pictures[2], 2);

			ui.plane1->setPixmap(QPixmap::fromImage(pictures[0]));
			ui.plane2->setPixmap(QPixmap::fromImage(pictures[1]));
			ui.plane3->setPixmap(QPixmap::fromImage(pictures[2]));
			break;
		}
	}

	~ColorPlanesWindow() {}

private:
	Ui::ColorPlanesWindow ui;

	int img_h;
	int img_w;

	double h_times;
	double w_times;

	img::Matrix op_data;

	std::vector<QImage> pictures;

private:

	int findDecisionSide(int h, int w)
	{
		h_times = 1.0 * img_h / h;
		w_times = 1.0 * img_w / w;

		if (h_times < w_times)
			return 0;
		else
			return 1;
	}

	void transferPixelsToQimg(const img::ImgFunction::PixelsMap &pdata, QImage &qimg, int color)
	{
		for (unsigned r = 0; r < pdata.getRow(); ++r)
		{
			uchar *ptr = qimg.scanLine(r);

			for (unsigned c = 0; c < pdata.getCol(); ++c)
				for (unsigned k = 0; k < pdata.getColor(); ++k)
				{
					if (k == color)
						*(ptr++) = pdata[r][c][color];
					else
						*(ptr++) = 0;
				}
		}
	}
};


#endif	// end _ColorPlanesWindow_H
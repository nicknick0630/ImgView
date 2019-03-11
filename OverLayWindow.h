#ifndef _OverLayWindow_H
#define _OverLayWindow_H

#include <QDialog>
#include <QImage>
#include <QFileDialog>
#include <QPixMap>
#include "ui_OverLayWindow.h"

#include "PCX.h"

class OverLayWindow : public QDialog
{
	Q_OBJECT

public:
	OverLayWindow(const img::Matrix &op_data, QWidget *parent = Q_NULLPTR) : QDialog(parent), multiples(2, 0), img_size(2)
	{
		ui.setupUi(this);

		data.push_back(op_data);

		openAnotherFile();

		// set image frame size
		img_h = 464;
		img_w = 390;

		bool have_gray = data[0].getColor() == 1 || data[1].getColor() == 1;

		if (have_gray)
		{
			color_cnt = 1;

			if (data[0].getColor() != 1)
				img::ImgFunction::transformToGrayLevel(data[0]);

			if (data[1].getColor() != 1)
				img::ImgFunction::transformToGrayLevel(data[1]);
		}
		else
			color_cnt = 3;


		resizeImage();

		initialButtoms();

		initialLabels();

		initailSlider();

		showFrontAndBack();
	}

	~OverLayWindow() {}

public slots:

	void showValue(int value)
	{
		ui.slider_value->setText(QString{ std::to_string(value).c_str() });
	}

	void changeFrontAndBack(void)
	{
		front_idx = 1 - front_idx;
		back_idx = 1 - back_idx;

		ui.slider->setValue(255);

		showFrontAndBack();
		overlay(255);
	}

	void overlay(int value)
	{
		auto &f = data[front_idx];
		auto &b = data[back_idx];

		int h = f.getRow() > b.getRow() ? f.getRow() : b.getRow();
		int w = f.getCol() > b.getCol() ? f.getCol() : b.getCol();

		showImg = QImage{ w,h, color_cnt == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888 };

		double f_rate = 1.0 * value / 255;
		double b_rate = 1.0 - f_rate;

		int fxs = (w - f.getCol()) / 2;
		int fys = (h - f.getRow()) / 2;
		int bxs = (w - b.getCol()) / 2;
		int bys = (h - b.getRow()) / 2;

		QColor color;

		bool check_f, check_b;
		double tem;

		for (int r = 0; r < h; ++r)
		{
			uchar *ptr = showImg.scanLine(r);

			for (int c = 0; c < w; ++c)
			{
				check_f = checkFront(r, c, fxs, fys);
				check_b = checkBack(r, c, bxs, bys);

				if (check_f && check_b)
				{
					for (int k = 0; k < color_cnt; ++k)
					{
						tem = f[r - fys][c - fxs][k] * f_rate + b[r - bys][c - bxs][k] * b_rate;

						*(ptr++) = (int)(tem + 0.5);
					}
				}
				else if (check_f && !check_b)
				{
					for (int k = 0; k < color_cnt; ++k)
					{
						tem = f[r - fys][c - fxs][k] * f_rate + 255 * b_rate;

						*(ptr++) = (int)(tem + 0.5);
					}
				}
				else if (!check_f && check_b)
				{
					for (int k = 0; k < color_cnt; ++k)
					{
						tem = 255 * f_rate + b[r - bys][c - bxs][k] * b_rate;

						*(ptr++) = (int)(tem + 0.5);
					}
				}
				else
				{
					for (int k = 0; k < color_cnt; ++k)
					{
						*(ptr++) = 255;
					}
				}
			}
		}

		ui.overImg->setPixmap(QPixmap::fromImage(showImg));

		this->update();
	}

	void store(void)
	{
		auto d = overlayImgToMatrix();

		img::PCX image{ d };

		QString fpath = QFileDialog::getSaveFileName(this, QStringLiteral("儲存到新影像"), "../Image/", "Images ( *.pcx )");

		std::string file{ fpath.toStdString() };

		std::string type{ file.substr(file.find_last_of('.') + 1) };

		// default to store to pcx
		if (type.size() == 0)
			image.store(fpath.toStdString() + ".pcx");
		else if (type == "pcx")
			image.store(fpath.toStdString());

		image.close();
	}

private:
	Ui::OverLayWindow ui;

	int front_idx{ 0 };
	int back_idx{ 1 };

	int color_cnt;

	std::vector<img::Matrix> data;
	std::vector<double> multiples;

	int img_h;
	int img_w;
	std::vector<QSize> img_size;

	QImage front;
	QImage back;
	QImage showImg;

private:

	void initialButtoms(void)
	{
		ui.change_buttom->setText(QStringLiteral("交換前後景"));

		QObject::connect(ui.change_buttom, SIGNAL(clicked()), this, SLOT(changeFrontAndBack()));
		
		ui.store_buttom->setText(QStringLiteral("儲存"));

		QObject::connect(ui.store_buttom, SIGNAL(clicked()), this, SLOT(store()));

		ui.h_buttom->hide();
		ui.h_buttom->setVisible(false);
	}

	void initialLabels(void)
	{
		ui.front_label->setText(QStringLiteral("前景影像"));
		ui.back_label->setText(QStringLiteral("背景影像"));
		ui.slider_name->setText(QStringLiteral("前景透明度"));
	}

	void initailSlider(void)
	{
		ui.slider->setRange(0, 255);

		QObject::connect(ui.slider, SIGNAL(valueChanged(int)), this, SLOT(showValue(int)));
		QObject::connect(ui.slider, SIGNAL(valueChanged(int)), this, SLOT(overlay(int)));

		ui.slider->setValue(255);
	}

	void openAnotherFile(void)
	{
		QString file_name{ QFileDialog::getOpenFileName(this, QStringLiteral("選擇影像") , QStringLiteral("../Image/") , QStringLiteral("Image (*.pcx *.png *.jepg *.jpg *.bmp);")) };

		img::PCX image{ file_name.toStdString() };

		data.push_back(std::move(image.getImgPixelData_r()));

		image.close();
	}

	void showFrontAndBack(void)
	{
		front = QImage(data[front_idx].getCol(), data[front_idx].getRow(), color_cnt == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);
		back = QImage(data[back_idx].getCol(), data[back_idx].getRow(), color_cnt == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);

		transferPixelsToQimg(data[front_idx], front);
		transferPixelsToQimg(data[back_idx], back);

		ui.front->setPixmap(QPixmap::fromImage(front));
		ui.back->setPixmap(QPixmap::fromImage(back));

		this->update();
	}

private:

	void resizeImage(void)
	{
		double times;

		for (unsigned i = 0; i < data.size(); ++i)
		{
			times = findDecisionSide(data[i].getRow(), data[i].getCol());

			data[i] = img::ImgFunction::resize(data[i], times);

			img_size[i].setHeight(data[i].getRow());
			img_size[i].setWidth(data[i].getCol());
		}
	}

	double findDecisionSide(int h, int w)
	{
		double h_times = 1.0 * img_h / h;
		double w_times = 1.0 * img_w / w;

		if (h_times < w_times)
			return h_times;
		else
			return w_times;
	}

	void transferPixelsToQimg(const img::ImgFunction::PixelsMap &pdata, QImage &qimg)
	{
		for (unsigned r = 0; r < pdata.getRow(); ++r)
		{
			uchar *ptr = qimg.scanLine(r);

			for (unsigned c = 0; c < pdata.getCol(); ++c)
				for (unsigned color = 0; color < color_cnt; ++color)
					*(ptr++) = pdata[r][c][color];
		}
	}

	bool checkFront(int r, int c, int sx, int sy)
	{
		if (r < sy || c < sx)
			return false;

		if (r - sy >= data[front_idx].getRow())
			return false;

		if (c - sx >= data[front_idx].getCol())
			return false;

		return true;
	}

	bool checkBack(int r, int c, int sx, int sy)
	{
		if (r < sy || c < sx)
			return false;

		if (r - sy >= data[back_idx].getRow())
			return false;

		if (c - sx >= data[back_idx].getCol())
			return false;

		return true;
	}

	img::Matrix overlayImgToMatrix(void) const
	{
		img::Matrix d( showImg.height(),showImg.width(),color_cnt );

		for (unsigned r = 0; r < d.getRow(); ++r)
			for (unsigned c = 0; c < d.getCol(); ++c)
			{
				QColor k = showImg.pixelColor(c, r);

				if (color_cnt == 1)
					d[r][c][0] = k.red();
				else
				{
					d[r][c][0] = k.red();
					d[r][c][1] = k.green();
					d[r][c][2] = k.blue();
				}
			}

		return d;
	}
};


#endif	// end _OverLayWindow_H
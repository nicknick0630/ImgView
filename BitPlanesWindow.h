#ifndef _BitPlanesWindow_H
#define _BitPlanesWindow_H

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include "ui_BItPlanesWindow.h"
#include "PCX.h"

class BitPlanesWindow : public QDialog
{
	Q_OBJECT

public:

	BitPlanesWindow(const img::Matrix &pdata, int _type, QWidget *parent = Q_NULLPTR) : QDialog(parent), pictures(8), type{ _type }
	{
		ui.setupUi(this);

		QSize s{ 263,316 };

		img_h = s.height();
		img_w = s.width();

		times = findDecisionSide(pdata.getRow(), pdata.getCol());

		if (type == 2)
		{
			// use gray code
			auto data = pdata;

			transformToGrayCode(data);

			getBitPlane(data);
		}
		else
			getBitPlane(pdata);


		setBitPlanes();
	}

	~BitPlanesWindow()
	{
		for (auto &p : pictures)
			p.~QImage();
	}

private:
	Ui::BitPlanesWindow ui;

	int img_h;
	int img_w;

	double times;

	std::vector<QImage> pictures;

	int type;

private:

	double findDecisionSide(int h, int w)
	{
		double h_times = 1.0 * img_h / h;
		double w_times = 1.0 * img_w / w;

		if (h_times < w_times)
			return h_times;
		else
			return w_times;
	}

	void getBitPlane(const img::Matrix &pdata)
	{
		auto fn = [&](int s, int e)
		{
			for (int i = s; i <= e; ++i)
			{
				auto op_data = img::ImgFunction::resize(img::ImgFunction::getBitPlane(pdata, i), times);

				int h = op_data.getRow();
				int w = op_data.getCol();

				auto &qimg = pictures[i - 1];

				qimg = QImage(w, h, op_data.getColor() == 1 ? QImage::Format_Grayscale8 : QImage::Format_RGB888);

				transferPixelsToQimg(img::ImgFunction::getBitPlane(op_data, i), qimg);
			}
		};

		std::thread td1{ fn,1,4 };
		std::thread td2{ fn,5,8 };

		td1.join();
		td2.join();
	}

	void transferPixelsToQimg(const img::ImgFunction::PixelsMap &pdata, QImage &qimg)
	{
		for (unsigned r = 0; r < pdata.getRow(); ++r)
		{
			uchar *ptr = qimg.scanLine(r);

			for (unsigned c = 0; c < pdata.getCol(); ++c)
				for (unsigned color = 0; color < pdata.getColor(); ++color)
					*(ptr++) = pdata[r][c][color];
		}
	}

	void setBitPlanes(void)
	{
		ui.plane1->setPixmap(QPixmap::fromImage(pictures[0]));
		ui.plane2->setPixmap(QPixmap::fromImage(pictures[1]));
		ui.plane3->setPixmap(QPixmap::fromImage(pictures[2]));
		ui.plane4->setPixmap(QPixmap::fromImage(pictures[3]));
		ui.plane5->setPixmap(QPixmap::fromImage(pictures[4]));
		ui.plane6->setPixmap(QPixmap::fromImage(pictures[5]));
		ui.plane7->setPixmap(QPixmap::fromImage(pictures[6]));
		ui.plane8->setPixmap(QPixmap::fromImage(pictures[7]));
	}

	void transformToGrayCode(img::ImgFunction::PixelsMap &data)
	{
		auto table = getGrayCodeTable();

		for (unsigned r = 0; r < data.getRow(); ++r)
			for (unsigned c = 0; c < data.getCol(); ++c)
				for (unsigned color = 0; color < data.getColor(); ++color)
					data[r][c][color] = table[data[r][c][color]];
	}

	std::vector<int> getGrayCodeTable(void)
	{
		std::vector<int> table;

		for (unsigned int i = 0; i < (1 << 8); i++)
			table.push_back(i ^ (i >> 1));

		return table;
	}
};


#endif	// end _BitPlanesWindow_H
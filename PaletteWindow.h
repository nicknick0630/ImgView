#ifndef _PaletteWindow_H
#define _PaletteWindow_H

#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include "ui_PaletteWindow.h"
#include "PCX.h"

class PaletteWindow : public QLabel
{
	Q_OBJECT

public:
	PaletteWindow(const img::PCX::Palette &p, QWidget *parent = Q_NULLPTR) : palette{ p }, QLabel(parent)
	{
		ui.setupUi(this);
		this->setWindowFlag(Qt::FramelessWindowHint);
		this->setMouseTracking(true);

		length = sqrt(palette.size());

		data_box = new QLabel{ this };

		data_box->setFixedHeight(20);
	}

	~PaletteWindow() {}

	void paintEvent(QPaintEvent *ev)
	{
		QPainter p{ this };

		if (!qimg.isNull())
			p.drawImage(h_padding, v_padding, qimg);
	}

	void resizeEvent(QResizeEvent *ev)
	{
		calculateBlockSize();

		calculateQimgSize();

		updateQImage();


		data_box->setGeometry(h_padding * 2, height() - data_box->height(), width(), data_box->height());
	}

	void mouseMoveEvent(QMouseEvent *ev)
	{
		int x = ev->x() - h_padding;
		int y = ev->y() - v_padding;
		
		if (x < 0 || x >= qimg.width() || y < 0 || y >= qimg.height())
		{
			data_box->setText("RGB : ( --- , --- , --- )");
		}
		else
		{
			int c = x / block_w;
			int r = y / block_h;

			data_box->setText(QString{ "RGB : ( " } +
				std::to_string(palette[r*length + c][0]).c_str() + " , " +
				std::to_string(palette[r*length + c][1]).c_str() + " , " +
				std::to_string(palette[r*length + c][2]).c_str() + " )");
		}
	}

private:
	Ui::PaletteWindow ui;

	QLabel *data_box;

	const img::PCX::Palette palette;

	QImage qimg;

	int length;

	int block_w;
	int block_h;

	int qwidth;
	int qheight;

	int h_padding{ 20 };
	int v_padding{ 20 };

private:

	void calculateBlockSize(void)
	{
		block_w = (width() - 2 * h_padding) / length;
		block_h = (height() - v_padding - data_box->height()) / length;
	}

	void calculateQimgSize(void)
	{
		qwidth = block_w * length;
		qheight = block_h * length;
	}

	void updateQImage(void)
	{
		qimg = QImage{ block_w*length,block_h*length,QImage::Format_RGB888 };

		if (qimg.height() != 0 && qimg.width() != 0)
		{
			for (unsigned r = 0; r < length; ++r)
			{
				for (unsigned qr = 0; qr < block_h; ++qr)
				{
					uchar *ptr = qimg.scanLine(r*block_h + qr);

					for (unsigned c = 0; c < length; ++c)
					{
						for (unsigned qc = 0; qc < block_w; ++qc)
						{
							*(ptr++) = palette[r*length + c][0];
							*(ptr++) = palette[r*length + c][1];
							*(ptr++) = palette[r*length + c][2];
						}
					}
				}
			}
		}
	}
};


#endif	// end _PaletteWindow_H
#ifndef _HeaderFrame_PCX_H
#define _HeaderFrame_PCX_H

#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include "ui_HeaderFrame_PCX.h"
#include "PCX.h"

class HeaderFrame_PCX : public QLabel
{
	Q_OBJECT

public:
	HeaderFrame_PCX(const img::PCXHeader &h, QWidget *parent = Q_NULLPTR) : header{ h }, QLabel(parent)
	{
		ui.setupUi(this);

		this->setWindowFlag(Qt::FramelessWindowHint);

		Itemsinitialization();

		Tableinitialization();

		for (int r = 0; r < 16; ++r)
		{
			for (int c = 0; c < 2; ++c)
			{
				auto item = table->item(r, c);

				item->setFlags(item->flags() & (~Qt::ItemIsEditable));
				item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
			}

			table->setRowHeight(r, 25);
		}

		table->setColumnWidth(0, 170);
		table->setColumnWidth(1, 100);

		table->horizontalHeader()->hide();
		table->verticalHeader()->hide();
		table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	~HeaderFrame_PCX() {}

	void resizeEvent(QResizeEvent *ev)
	{
		calculateCoor();

		table->setGeometry(h_padding, v_padding, wid, heig);
	}

private:
	Ui::HeaderFrame_PCX ui;

	const img::PCXHeader header;

	QTableWidget *table;

	int h_padding;
	int v_padding;

	int wid;
	int heig;

private: /*-------- value --------*/

	QTableWidgetItem *Identifier;		// PCX Id Number (manufactruer), constant to 10 (0x0A) = Zsoft.pcx
	QTableWidgetItem *Version;			// Version number = 0, 2, 3, 4, 5
	QTableWidgetItem *Encoding;			// .pcx run length encoding = 1
	QTableWidgetItem *BitsPerPixel;		// bits per pixel per color plane = 1, 2, 4, 8
	QTableWidgetItem *Xmin;				// left of image
	QTableWidgetItem *Ymin;				// top of image
	QTableWidgetItem *Xmax;				// right of image
	QTableWidgetItem *Ymax;				// bottom of image
	QTableWidgetItem *Hdpi;				// horizontal resolution of image in dpi
	QTableWidgetItem *Vdpi;				// vertical resolution of image in dpi

		//	Palette						// 16-color EGA palette

	QTableWidgetItem *Reserved;			// reserved = 0
	QTableWidgetItem *NPlanes;			// number of color planes
	QTableWidgetItem *BytesPerLine;		// Bytes per scan-line. must be EVEN and do not calculate from Xmax - Xmin
	QTableWidgetItem *PaletteType;		// Color.BW = 1,  Grayscale = 2
	QTableWidgetItem *HscreenSize;		// horizontal screen size
	QTableWidgetItem *VscreenSize;		// vertical screen size

		//	Reserved2[54]				// blank to fill out 128 byte header. = 0

private: /*-------- title --------*/

	QTableWidgetItem *Identifier_name;			// PCX Id Number (manufactruer), constant to 10 (0x0A) = Zsoft.pcx
	QTableWidgetItem *Version_name;				// Version number = 0, 2, 3, 4, 5
	QTableWidgetItem *Encoding_name;			// .pcx run length encoding = 1
	QTableWidgetItem *BitsPerPixel_name;		// bits per pixel per color plane = 1, 2, 4, 8
	QTableWidgetItem *Xmin_name;				// left of image
	QTableWidgetItem *Ymin_name;				// top of image
	QTableWidgetItem *Xmax_name;				// right of image
	QTableWidgetItem *Ymax_name;				// bottom of image
	QTableWidgetItem *Hdpi_name;				// horizontal resolution of image in dpi
	QTableWidgetItem *Vdpi_name;				// vertical resolution of image in dpi

		// Palette_name							// 16-color EGA palette

	QTableWidgetItem *Reserved_name;			// reserved = 0
	QTableWidgetItem *NPlanes_name;				// number of color planes
	QTableWidgetItem *BytesPerLine_name;		// Bytes per scan-line. must be EVEN and do not calculate from Xmax - Xmin
	QTableWidgetItem *PaletteType_name;			// Color.BW = 1,  Grayscale = 2
	QTableWidgetItem *HscreenSize_name;			// horizontal screen size
	QTableWidgetItem *VscreenSize_name;			// vertical screen size

		//Reserved2_name						// blank to fill out 128 byte header. = 0

private:

	void Itemsinitialization(void)
	{
		Identifier_name = new QTableWidgetItem{ " Identifier" };
		Identifier = new QTableWidgetItem{ QString{std::to_string(header.Identifier).c_str()} };

		Version_name = new QTableWidgetItem{ " Version" };
		Version = new QTableWidgetItem{ QString{std::to_string(header.Version).c_str()} };

		Encoding_name = new QTableWidgetItem{ " Encoding" };
		Encoding = new QTableWidgetItem{ QString{std::to_string(header.Encoding).c_str()} };

		BitsPerPixel_name = new QTableWidgetItem{ " BitsPerPixel" };
		BitsPerPixel = new QTableWidgetItem{ QString{std::to_string(header.BitsPerPixel).c_str()} };

		Xmin_name = new QTableWidgetItem{ " Xmin" };
		Xmin = new QTableWidgetItem{ QString{std::to_string(header.Xmin).c_str()} };

		Ymin_name = new QTableWidgetItem{ " Ymin" };
		Ymin = new QTableWidgetItem{ QString{std::to_string(header.Ymin).c_str()} };

		Xmax_name = new QTableWidgetItem{ " Xmax" };
		Xmax = new QTableWidgetItem{ QString{std::to_string(header.Xmax).c_str()} };

		Ymax_name = new QTableWidgetItem{ " Ymax" };
		Ymax = new QTableWidgetItem{ QString{std::to_string(header.Ymax).c_str()} };

		Hdpi_name = new QTableWidgetItem{ " Hdpi" };
		Hdpi = new QTableWidgetItem{ QString{std::to_string(header.Hdpi).c_str()} };

		Vdpi_name = new QTableWidgetItem{ " Vdpi" };
		Vdpi = new QTableWidgetItem{ QString{std::to_string(header.Vdpi).c_str()} };

		Reserved_name = new QTableWidgetItem{ " Reserved" };
		Reserved = new QTableWidgetItem{ QString{std::to_string(header.Reserved).c_str()} };

		NPlanes_name = new QTableWidgetItem{ " NPlanes" };
		NPlanes = new QTableWidgetItem{ QString{std::to_string(header.NPlanes).c_str()} };

		BytesPerLine_name = new QTableWidgetItem{ " BytesPerLine" };
		BytesPerLine = new QTableWidgetItem{ QString{std::to_string(header.BytesPerLine).c_str()} };

		PaletteType_name = new QTableWidgetItem{ " PaletteType" };
		PaletteType = new QTableWidgetItem{ QString{std::to_string(header.PaletteType).c_str()} };

		HscreenSize_name = new QTableWidgetItem{ " HscreenSize" };
		HscreenSize = new QTableWidgetItem{ QString{std::to_string(header.HscreenSize).c_str()} };

		VscreenSize_name = new QTableWidgetItem{ " VscreenSize" };
		VscreenSize = new QTableWidgetItem{ QString{std::to_string(header.VscreenSize).c_str()} };
	}

	void Tableinitialization(void)
	{
		table = new QTableWidget{ 16,2,this };

		table->setItem(0, 0, Identifier_name);
		table->setItem(0, 1, Identifier);

		table->setItem(1, 0, Version_name);
		table->setItem(1, 1, Version);

		table->setItem(2, 0, Encoding_name);
		table->setItem(2, 1, Encoding);

		table->setItem(3, 0, BitsPerPixel_name);
		table->setItem(3, 1, BitsPerPixel);

		table->setItem(4, 0, Xmin_name);
		table->setItem(4, 1, Xmin);

		table->setItem(5, 0, Ymin_name);
		table->setItem(5, 1, Ymin);

		table->setItem(6, 0, Xmax_name);
		table->setItem(6, 1, Xmax);

		table->setItem(7, 0, Ymax_name);
		table->setItem(7, 1, Ymax);

		table->setItem(8, 0, Hdpi_name);
		table->setItem(8, 1, Hdpi);

		table->setItem(9, 0, Vdpi_name);
		table->setItem(9, 1, Vdpi);

		table->setItem(10, 0, Reserved_name);
		table->setItem(10, 1, Reserved);

		table->setItem(11, 0, NPlanes_name);
		table->setItem(11, 1, NPlanes);

		table->setItem(12, 0, BytesPerLine_name);
		table->setItem(12, 1, BytesPerLine);

		table->setItem(13, 0, PaletteType_name);
		table->setItem(13, 1, PaletteType);

		table->setItem(14, 0, HscreenSize_name);
		table->setItem(14, 1, HscreenSize);

		table->setItem(15, 0, VscreenSize_name);
		table->setItem(15, 1, VscreenSize);
	}

	void calculateCoor(void)
	{
		heig = table->rowCount() * table->rowHeight(0);
		wid = table->columnWidth(0) + table->columnWidth(1);

		v_padding = height() - heig;
		h_padding = width() - wid;

		v_padding /= 2;
		h_padding /= 2;
	}
};


#endif	// end _HeaderFrame_PCX_H
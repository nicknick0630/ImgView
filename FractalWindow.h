#ifndef _FRACTAL_WINDOW_H
#define _FRACTAL_WINDOW_H

#include <QDialog>
#include <QImage>
#include <QTableWidget>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include "ui_FractalWindow.h"

#include "PCX.h"
#include "Fractal.h"

#include <thread>

class FractalWindow : public QDialog
{
	Q_OBJECT

public:
	FractalWindow(QWidget *parent = Q_NULLPTR) : QDialog(parent)
	{
		ui.setupUi(this);

		this->setWindowFlag(Qt::WindowMinMaxButtonsHint);

		initialTableWidgetItems();

		initialTable();

		QObject::connect(this, SIGNAL(showCompressOK_signal()), this, SLOT(showCompressOK_slot()));
		QObject::connect(this, SIGNAL(showCompressOK_signal()), this, SLOT(hideCompressingStatus_slot()));

		hideCompressingStatus_slot();

		ui.show_compressing->setStyleSheet("color:red; font-size:20px;");
	}

	~FractalWindow() {}

public:
	void paintEvent(QPaintEvent *ev)
	{
		QPainter p{ this };

		if (!org_qimg.isNull())
			p.drawImage(50, 100, org_qimg);

		if (!cpr_qimg.isNull())
			p.drawImage(400, 100, cpr_qimg);
	}

public slots:

	void openImage_slot(void)
	{
		QString file_name{ QFileDialog::getOpenFileName(this, QStringLiteral("選擇影像") , QStringLiteral("../Image/Fractal") , QStringLiteral("Image (*.pcx);")) };

		if (!file_name.isNull())
		{
			std::string fpath{ file_name.toStdString() };

			org_image = img::PCX{ fpath }.getImgPixelData_r();

			transferDataToQImage(org_image, org_qimg);

			setHeightAndWidth();

			update();
		}
	}

	void openFractalData_slot(void)
	{
		QString file_name{ QFileDialog::getOpenFileName(this, QStringLiteral("選擇碎形檔案") , QStringLiteral("../Image/Fractal/Demo_Result") , QStringLiteral("Image (*.frc);")) };

		QString file_name2{ QFileDialog::getOpenFileName(this, QStringLiteral("選擇碎形檔案") , QStringLiteral("../Image/Fractal") , QStringLiteral("Image (*.pcx);")) };

		if (!file_name2.isNull())
		{
			std::string fpath{ file_name2.toStdString() };

			start_image = img::PCX{ fpath }.getImgPixelData_r();
		}

		if (!file_name.isNull())
		{
			std::string fpath{ file_name.toStdString() };

			fractal.decode(0, fpath);
			cpr_image = start_image;

			transferDataToQImage(cpr_image, cpr_qimg);

			setPSNR();
			setRange();

			resetIterationCount();

			update();
		}

		
	}

	void iteration_slot(void)
	{
		if (fractal.getRangeSize() == 0 || cpr_image.getRow() == 0 || org_image.getRow() == 0)
		{
			return;
		}

		fractal.decode(cpr_image);

		transferDataToQImage(cpr_image, cpr_qimg);

		setPSNR();
		addIterationCount();

		update();
	}

	void compress_slot(void)
	{
		auto num_str = ui.block_size_input->text().toStdString();

		if (num_str.size() == 0)
		{
			QMessageBox message(QMessageBox::Warning, "Information", "Invalid block size", QMessageBox::Ok, NULL);

			message.exec();

			return;
		}
		
		auto block_size = std::stoi(num_str);

		if (block_size != 2 && block_size != 4 && block_size != 8 && block_size != 16)
		{
			QMessageBox message(QMessageBox::Warning, "Information", "Invalid block size", QMessageBox::Ok, NULL);

			message.exec();

			return;
		}


		QString fpath = QFileDialog::getSaveFileName(this, QStringLiteral("儲存為碎形格式"), "../Image/Fractal/Result", "Images ( *.frc )");

		if (!fpath.isNull())
		{
			ui.show_compressing->show();

			std::string file{ fpath.toStdString() };

			std::thread td{ [&]() {

				std::string f{file};

				fractal.compress(org_image, block_size);
				fractal.output(f);

				showCompressOK_signal();
			} };

			td.detach();
		}
	}

	void showCompressOK_slot(void)
	{
		QMessageBox message(QMessageBox::Information, nullptr, QStringLiteral("壓縮完成"), QMessageBox::Ok, nullptr);

		message.exec();
	}

	void hideCompressingStatus_slot(void)
	{
		ui.show_compressing->hide();
	}

signals:
	void showCompressOK_signal(void);

private:
	Ui::FractalWindow ui;

	img::Matrix org_image;
	img::Matrix cpr_image;

	img::Matrix start_image;

	QImage org_qimg;
	QImage cpr_qimg;

	img::Fractal fractal;

private:
	QTableWidgetItem *Height;
	QTableWidgetItem *Height_v;
	QTableWidgetItem *Width;
	QTableWidgetItem *Width_v;
	QTableWidgetItem *PSNR;
	QTableWidgetItem *PSNR_v;
	QTableWidgetItem *Ragne;
	QTableWidgetItem *Ragne_v;
	QTableWidgetItem *Iteration_count;
	QTableWidgetItem *Iteration_count_v;

private:
	void initialTableWidgetItems(void)
	{
		Height = new QTableWidgetItem{ "Height" };
		Height_v = new QTableWidgetItem{ "---" };

		Width = new QTableWidgetItem{ "Width" };
		Width_v = new QTableWidgetItem{ "---" };

		PSNR = new QTableWidgetItem{ "PSNR" };
		PSNR_v = new QTableWidgetItem{ "---" };

		Ragne = new QTableWidgetItem{ "Ragne Bk Size" };
		Ragne_v = new QTableWidgetItem{ "---" };

		Iteration_count = new QTableWidgetItem{ QStringLiteral("遞迴次數") };
		Iteration_count_v = new QTableWidgetItem{ "0" };
	}

	void initialTable(void)
	{
		ui.table->setRowCount(5);
		ui.table->setColumnCount(2);

		ui.table->setItem(0, 0, Height);
		ui.table->setItem(0, 1, Height_v);

		ui.table->setItem(1, 0, Width);
		ui.table->setItem(1, 1, Width_v);

		ui.table->setItem(2, 0, PSNR);
		ui.table->setItem(2, 1, PSNR_v);

		ui.table->setItem(3, 0, Iteration_count);
		ui.table->setItem(3, 1, Iteration_count_v);

		ui.table->setItem(4, 0, Ragne);
		ui.table->setItem(4, 1, Ragne_v);

		for (int r = 0; r < 5; ++r)
		{
			for (int c = 0; c < 2; ++c)
			{
				auto item = ui.table->item(r, c);

				item->setFlags(item->flags() & (~Qt::ItemIsEditable));
				item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
			}
		}

		ui.table->horizontalHeader()->hide();
		ui.table->verticalHeader()->hide();
		ui.table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		ui.table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

private:
	void transferDataToQImage(const img::Matrix &data, QImage &qimg)
	{
		QImage::Format format{ ((data.getColor() == 1) ? QImage::Format_Grayscale8 : QImage::Format_RGB888) };

		qimg = QImage(data.getCol(), data.getRow(), format);

		unsigned r, c, color;


		for (r = 0; r < data.getRow(); ++r)
		{
			// use ScanLine , because QImage inner data will be set alignment by 4 and prone to error
			uchar *ptr = qimg.scanLine(r);

			for (c = 0; c < data.getCol(); ++c)
				for (color = 0; color < data.getColor(); ++color)
					*(ptr++) = data[r][c][color];
		}
	}

	void setHeightAndWidth(void)
	{
		Height_v->setText(std::to_string(org_image.getRow()).c_str());
		Width_v->setText(std::to_string(org_image.getCol()).c_str());
	}

	void setRange(void)
	{
		Ragne_v->setText(std::to_string(fractal.getRangeSize()).c_str());
	}

	void setPSNR(void)
	{
		auto psnr = img::ImgFunction::getPSNR(org_image, cpr_image);

		PSNR_v->setText(std::to_string(psnr).c_str());
	}

	void resetIterationCount(void)
	{
		Iteration_count_v->setText("0");
	}

	void addIterationCount(void)
	{
		auto str = Iteration_count_v->text().toStdString();

		auto num = std::stoi(str);

		auto n = std::to_string(++num);

		Iteration_count_v->setText(n.c_str());
	}
};

#endif

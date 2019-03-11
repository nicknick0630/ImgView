#ifndef _pWindow_H
#define _pWindow_h

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QMovie>
#include <thread>

class pWindow : public QWidget
{

	Q_OBJECT

public :
	pWindow(const QString &image_path, QWidget *parent = nullptr) : QWidget{ parent }, width{ 700 }, height{ 400 }
	{
		// set position
		setToCenter();

		// set ui
		this->setWindowFlag(Qt::FramelessWindowHint);

		this->setStyleSheet(QString{ "background-color:rgb(255,255,255);" });

		// set connection
		QObject::connect(this, SIGNAL(_close()), this, SLOT(close()));


		// set label
		QLabel *label1 = new QLabel{ this };
		QLabel *label2 = new QLabel{ this };

		QMovie *movie = new QMovie{ image_path };
		
		label1->setGeometry(100, 0, 250, 400);
		label1->setMovie(movie);
		
		label2->setGeometry(width - 250 - 40, 0, 250, 400);
		label2->setMovie(movie);

		label1->show();
		label2->show();

		movie->start();
	}
	
	~pWindow() {}

	void connect(QWidget *w)
	{
		QObject::connect(this, SIGNAL(_callMainWindow()), w, SLOT(show()));
	}

	void _show(long millisec)
	{
		this->show();

		std::thread td{ [&]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds{millisec});

			this->_close();

			this->_callMainWindow();
		} };

		td.detach();
	}

signals :
	void _close();

	void _callMainWindow();

private :
	int width;
	int height;

	void setToCenter(void)
	{
		int x = 1920 / 2;
		int y = 1080 / 2;

		this->setGeometry(x - width / 2, y - height / 2, width, height);
	}
};

#endif	// end _pWindow_H
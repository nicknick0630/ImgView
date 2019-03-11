#include "ImgView.h"
#include "pWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	ImgView w;

	pWindow pw{ "../Resources/PreImg/manwalk.gif" };

	pw.connect(&w);

	pw._show(2000);

	/*img::PCX image{ "../Image/Lena_256.pcx" };

	ImageOverlayWindow w{image.getImgPixelData()};

	w.show();*/

	//w.resize(300, 400);

	return a.exec();
}

#include <QApplication>
#include <QtWidgets>

#include "controller/MainWindow.hpp"

using namespace AdExpSim;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	app.setApplicationName("AdExpSimGui");

	MainWindow window;
	window.show();

	return app.exec();
}


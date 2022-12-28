#include <QApplication>
#include "Eval.h"

using namespace Ui;

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	Eval E;
	E.show();

	return app.exec();
}
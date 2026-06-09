#include "peakspeakspeaks.h"
#include <QApplication>
#include <iostream>

/** @brief Application entry point; runs the Qt event loop. */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PeaksPeaksPeaks w;
    w.show();
    return app.exec();
}

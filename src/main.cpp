// PeaksPeaksPeaks — Copyright (c) 2026 Zofia Tryznowska
#include "peakspeakspeaks.h"

#include <QApplication>
#include <QIcon>

#include <iostream>

/** @brief Application entry point; runs the Qt event loop. */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon.png"));
    PeaksPeaksPeaks w;
    w.show();
    return app.exec();
}

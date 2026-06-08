#include "peakspeakspeaks.h"
#include <QApplication>
#include <iostream>

/**
 * @brief Entry point for the PeaksPeaksPeaks application.
 * * Initializes the Qt framework infrastructure via QApplication, instantiates the 
 * main user interface window, makes it visible to the user, and kicks off the 
 * continuous application event execution loop.
 * * @param argc Number of command-line arguments passed to the program.
 * @param argv Array of strings containing the command-line arguments.
 * @return int Standard exit status code returned by the Qt event loop loop (0 if successful).
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PeaksPeaksPeaks w;
    w.show();
    return app.exec();
}
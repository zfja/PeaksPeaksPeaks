#include "peakspeakspeaks.h"
#include "./ui_peakspeakspeaks.h"

PeaksPeaksPeaks::PeaksPeaksPeaks(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PeaksPeaksPeaks)
{
    ui->setupUi(this);
}

PeaksPeaksPeaks::~PeaksPeaksPeaks()
{
    delete ui;
}

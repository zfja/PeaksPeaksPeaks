#ifndef PEAKSPEAKSPEAKS_H
#define PEAKSPEAKSPEAKS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class PeaksPeaksPeaks;
}
QT_END_NAMESPACE

class PeaksPeaksPeaks : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeaksPeaksPeaks(QWidget *parent = nullptr);
    ~PeaksPeaksPeaks() override;

private:
    Ui::PeaksPeaksPeaks *ui;
};
#endif // PEAKSPEAKSPEAKS_H

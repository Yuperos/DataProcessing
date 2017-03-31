#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <Qlist>
#include <QPair>
#include "parsercsv.h"
#include "processingunit.h"

template<typename... Args> struct SELECT {
    template<typename C, typename R>
    static constexpr auto OVERLOAD_OF( R (C::*pmf)(Args...) ) -> decltype(pmf) {
        return pmf;
    }
};

namespace Ui {
   class MainWindow;
   }

class MainWindow : public QMainWindow
   {
   Q_OBJECT

public:
   bool autosave;
   QTimer* timer;
   QTime time;
   uint64_t elapsedMillis;
   uint64_t counter;
   int chanel;
   QString currPath;
   MeasurementParameters params;
   QList<QPair<double,double>> list;
   QList<double> convoluted;

   ProcessingUnit procUnit;

   void updateChTrigText(int chNumber);

   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();

private:
   Ui::MainWindow *ui;

public slots:
   void openRawFile();
   void openFile();
   void saveFile();
   void saveCsvFile();
   void saveProcessed();
   void calcRemainingTime();
   void toggledAutosave(bool checked);
   void processing();

   void ch1up(double val);
   void ch1down(double val);
   void ch2up();
   void ch2down();

signals:
   void progress1(int);
   void progress2(int);
   };

#endif // MAINWINDOW_H

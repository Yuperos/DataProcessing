#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QTime>
#include <math.h>



MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow)
   {
   ui->setupUi(this);

   timer = new QTimer(this);
   time.start();
   autosave = ui->cbAutoSave->isChecked();
   procUnit.init();

   ui->dsbCh1Up->setValue(procUnit.offsetCh1[0]);
   ui->dsbCh1Down->setValue(procUnit.offsetCh1[1]);
   ui->dsbCh2Up->setValue(1.21);
   ui->sbExpCh2Up->setValue(-6);

   updateChTrigText(0);
   updateChTrigText(1);

   connect(timer, &QTimer::timeout, this, &MainWindow::calcRemainingTime);
   connect(ui->a_OpenFileSrc,&QAction::triggered,this,&MainWindow::openRawFile);
   connect(ui->a_OpenFile,&QAction::triggered,this,&MainWindow::openFile);
   connect(ui->a_SaveFileAs,&QAction::triggered,this,&MainWindow::saveFile);
   connect(ui->a_SaveCSV,&QAction::triggered,this,&MainWindow::saveCsvFile);
   connect(ui->cbAutoSave,&QCheckBox::toggled, this, &MainWindow::toggledAutosave);
   connect(this,&MainWindow::progress1,ui->pbChanel1,&QProgressBar::setValue);
   connect(this,&MainWindow::progress2,ui->pbChanel2,&QProgressBar::setValue);
   connect(ui->pbProcess, &QPushButton::clicked, this, &MainWindow::processing);
   connect(ui->action, &QAction::triggered, this, &MainWindow::saveProcessed);


   connect(ui->dsbCh1Up,SELECT<double>::OVERLOAD_OF(&QDoubleSpinBox::valueChanged), this, &MainWindow::ch1up);
   connect(ui->dsbCh1Down,SELECT<double>::OVERLOAD_OF(&QDoubleSpinBox::valueChanged), this, &MainWindow::ch1down);
   connect(ui->dsbCh2Up,SELECT<double>::OVERLOAD_OF(&QDoubleSpinBox::valueChanged), this, &MainWindow::ch2up);
   connect(ui->sbExpCh2Up,SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),this,&MainWindow::ch2up);
   connect(ui->dsbCh2Down,SELECT<double>::OVERLOAD_OF(&QDoubleSpinBox::valueChanged), this, &MainWindow::ch2down);
   connect(ui->sbExpCh2Down,SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),this,&MainWindow::ch2down);

   currPath.clear();
   //   currPath.append("d:\\Work\\Programms\\DataProcessing\\Data\\");
   currPath.append("d:\\Work\\Data\\new\\");
   }

MainWindow::~MainWindow()
   {
   delete ui;
   }

void MainWindow::openRawFile()
   {
   list.clear();
   chanel = 0;

   QString path = QFileDialog::getOpenFileName(this, tr("Open File"),
                                               currPath,
                                               tr("Data Table (*.csv)"));
   if(path.isEmpty()) return;
   params = ParserCSV::getMeasureParameters(path);
   setWindowTitle(params.fileName);
   QString tempPath = path;
   tempPath.chop(params.fileName.length());
   currPath.clear();
   currPath.append(tempPath);

   timer->start(2000);
   time.restart();

   while (chanel < 2){
      switch(chanel){
         case 0: {
            path.replace(QString("Ch2"), QString("Ch1"));
            ui->lCurrentProcess->setText(QString("Opening Raw Ch1"));
            } break;
         case 1: {
            path.replace(QString("Ch1"), QString("Ch2"));
            ui->lCurrentProcess->setText(QString("Opening Raw Ch2"));
            } break;
         default : break;
         }

      QFile file(path);
      if(!file.open(QIODevice::ReadOnly)) {
         QMessageBox::information(0, "Open file error", file.errorString());
         }

      QTextStream in(&file);

      counter = 0;
      double singleData = 0.0;

      while(!in.atEnd()){
         elapsedMillis = time.elapsed();
         singleData = ParserCSV::getData(in.readLine());
         if (chanel == 0 && ParserCSV::len != 0 &&  counter < 10)
            {
            ui->pbChanel1->setRange(0,(ParserCSV::len-1)*2/1000);
            emit progress1(0);
            }

         switch(chanel){
            case 0:{
               list.append(QPair<double,double>(singleData,0.0));
               } break;
            case 1:{
               list[counter].second = singleData;
               } break;
            default: break;
            }
         emit progress1(((counter+1)+(chanel)*ParserCSV::len)/1000);

         qApp->processEvents();
         counter++;
         }

      file.close();
      chanel++;
      }
   timer->stop();
   ui->lCurrentProcess->setText(QString("Opening Raw Files Finished"));
   ui->lReminingTime->setText(QString(""));
   if (autosave)
      saveFile();
   }

void MainWindow::openFile()
   {
   QString path = QFileDialog::getOpenFileName(this, tr("Open File"),
                                               currPath,
                                               tr("Data Table (*.odf)"));
   if(path.isEmpty()) return;

   QString fileName = path.split('/' ,QString::SkipEmptyParts).last();
   fileName.truncate(fileName.indexOf('_'));
   setWindowTitle(params.fileName);

   ui->lCurrentProcess->setText(QString("Opening Bin File"));
   QFile file(path);
   if(file.open(QIODevice::ReadOnly))
      {
      QDataStream stream(&file);
      stream.setVersion(QDataStream::Qt_5_6);

      list.clear();
      quint32 c;
      stream >> c;
      list.reserve(c);
      ui->pbChanel1->setMaximum(c/1000*((double)19/100));
      emit progress1(0);
      for(quint32 i = 0; i < c; ++i)
         {
         QPair<double,double> t;
         stream >> t;
         list.append(t);
         if (stream.atEnd())
            break;
         emit progress1(i/1000+1);
         }


      if (stream.status() != QDataStream::Ok)
         {
         qDebug() << "Ошибка чтения";
         }
      }
   file.close();
   ui->lCurrentProcess->setText(QString("Opening Bin File Finished"));
   if (ui->cbAutoStart->isChecked())
      processing();
   }

void MainWindow::saveFile()
   {
   ui->lCurrentProcess->setText(QString("Saving Bin File"));
   QString path;
   if (autosave){
      QString fileName = params.fileName;
      fileName.truncate(fileName.indexOf("_"));
      fileName.append(".odf");
      path = QString("%1%2").arg(currPath).arg(fileName);
      }
   else
      path = QFileDialog::getSaveFileName(this, tr("Save File"), currPath,
                                          tr("OutputDataFile (*.odf)"));
   if(path.isEmpty()) return;

   QFile file(path);
   if(file.open(QIODevice::WriteOnly))
      {
      QDataStream stream(&file);
      stream.setVersion(QDataStream::Qt_5_6);

      stream << quint32(list.size());

      ui->pbChanel1->setMaximum(list.size()/100);
      emit progress1(0);

      for(int i = 0; i < list.size(); ++i){
         list[i].second = procUnit.summizer.averaging(list.at(i).second);
         if (procUnit.trigCh1.checkValAndCount(list.at(i).first))
            stream << list.at(i);

         emit progress1(i/100+1);
         qApp->processEvents();
         }

      if (stream.status() != QDataStream::Ok)
         {
         qDebug() << "Ошибка записи";
         }
      else
         ui->lCurrentProcess->setText(QString("Saving Bin File Finished"));
      }
   file.close();


   }

void MainWindow::saveCsvFile()
   {
   QString path = QFileDialog::getSaveFileName(this, tr("Save Data File"), currPath,
                                               tr("OutputDataFile (*.csv)"));
   ui->lCurrentProcess->setText(QString("Saving Preprocessed CSV File"));
   if(path.isEmpty()) return;
   QFile file(path);
   if(file.open(QIODevice::WriteOnly))
      {
      QTextStream stream(&file);
      ui->pbChanel1->setMaximum(list.length()/100);
      emit progress1(0);
      int i=0;
      for(auto a: list){
         ++i;
         QString str = QString::number(a.first,'E',10);
         str.append('\t');
         str.append(QString::number(a.second,'E',10));
         str.append('\n');
         str.replace('.',',');
         stream << str;
         emit progress1(i/100);
         qApp->processEvents();
         }
      if (stream.status() != QTextStream::Ok)
         {
         qDebug() << "Ошибка записи";
         }
      }
   file.close();
   ui->lCurrentProcess->setText(QString("Saving Preprocessed CSV File Finished"));
   }

void MainWindow::saveProcessed()
   {
   QString path = QFileDialog::getSaveFileName(this, tr("Save Convoluted File"), currPath,
                                               tr("OutputDataFile (*.csv)"));
   ui->lCurrentProcess->setText(QString("Saving Convoluted File"));
   if(path.isEmpty()) return;
   QFile file(path);
   if(file.open(QIODevice::WriteOnly))
      {
      QTextStream stream(&file);
      ui->pbChanel1->setMaximum(convoluted.length()/100);
      emit progress1(0);
      int i=0;
      for(auto a: convoluted){
         ++i;
         QString str = QString::number(a,'E',10);
         if (i==1) {
            str.prepend(
                     QString("Peaks Detected %1/%2\n")
                     .arg(procUnit.trigCh2.counter)
                     .arg(procUnit.trigCh1.counter));
            }
         str.append('\n');

         str.replace('.',',');
         stream << str;
         emit progress1(i/100);
         qApp->processEvents();
         }
      if (stream.status() != QTextStream::Ok)
         {
         qDebug() << "Ошибка записи";
         }
      }
   file.close();
   ui->lCurrentProcess->setText(QString("Saving Convoluted File Finished"));
   }

void MainWindow::calcRemainingTime()
   {
   uint64_t remainingMillis = (double)elapsedMillis / ((counter+1)+(chanel)*ParserCSV::len-1)  * ((uint64_t)ParserCSV::len*2) - elapsedMillis;
   ui->lReminingTime->setText(QTime::fromMSecsSinceStartOfDay(remainingMillis).toString());
   }

void MainWindow::toggledAutosave(bool checked)
   {
   autosave = checked;
   }

void MainWindow::processing()
   {
   ui->lCurrentProcess->setText(QString("Processing Data"));
   convoluted.clear();
   procUnit.setLists(&list,&convoluted);   
   procUnit.summizer.reset();
   procUnit.initTrigger();
   ui->pbChanel2->setMaximum(list.length()/1000);
   emit progress2(0);
   while(procUnit.counter < list.length()){
      procUnit.process();
      ui->lCh1Peaks->setText(QString::number(procUnit.trigCh1.counter));
      ui->lCh2Peaks->setText(QString::number(procUnit.trigCh2.counter));
      emit progress2((procUnit.counter+1)/1000);
      qApp->processEvents();
      }
   ui->lCurrentProcess->setText(QString("Processing Data Finished"));
   if (ui->cbAutoSave->isChecked())
      saveProcessed();
   }

void MainWindow::updateChTrigText(int chNumber)
   {
   char param = 'e';
   int decNumbers = 1;
   switch(chNumber){
      case 0:{
         QString str = QString("Ch1 %1 .. %2 :")
                       .arg(QString::number(procUnit.trigCh1.getUp(),param,decNumbers))
                       .arg(QString::number(procUnit.trigCh1.getDown(),param,decNumbers));
         ui->lTriggerCh1->setText(str);
         } break;
      case 1: {
         QString str = QString("Ch2 %1 .. %2 :")
                       .arg(QString::number(procUnit.trigCh2.getUp(),param,decNumbers))
                       .arg(QString::number(procUnit.trigCh2.getDown(),param,decNumbers));
         ui->lTriggerCh2->setText(str);
         }break;
      }
   }

void MainWindow::ch1up(double val)
   {
   procUnit.trigCh1.setRange(val,procUnit.trigCh1.getDown());
   updateChTrigText(0);
   }

void MainWindow::ch1down(double val)
   {
   procUnit.trigCh1.setRange(procUnit.trigCh1.getUp(), val);
   updateChTrigText(0);
   }

void MainWindow::ch2up()
   {
   double _val = ui->dsbCh2Up->value()*pow(10,ui->sbExpCh2Up->value());
   double temp = procUnit.trigCh2.getDown();
   procUnit.trigCh2.setRange(_val, temp);
   updateChTrigText(1);
   }

void MainWindow::ch2down()
   {
   double _val = ui->dsbCh2Down->value()*pow(10,ui->sbExpCh2Down->value());
   double temp  = procUnit.trigCh2.getUp();
   procUnit.trigCh2.setRange(temp, _val);
   updateChTrigText(1);
   }

#include "parsercsv.h"

int ParserCSV::infoLines = 6;

uint64_t ParserCSV::len = 0;
double ParserCSV::interval = 0;

ParserCSV::ParserCSV()
   {

   }

MeasurementParameters ParserCSV::getMeasureParameters(QString filePath)
   {
   QString fileName = filePath.split('/' ,QString::SkipEmptyParts).last();
   return parseFileName(fileName);
   }

MeasurementParameters ParserCSV::parseFileName(QString fileName)
   {
   MeasurementParameters rw;
   rw.fileName.clear();
   rw.fileName.append(fileName);
   fileName.truncate(fileName.indexOf('_'));
   QStringList parameters = fileName.split('-');
   parameters.removeFirst();
   for(int i =0; i<3; i++){
      int cutCount = parameters.at(i).lastIndexOf(QRegExp("[a-zA-Z]")) - 1;
      double value = parameters.at(i).left(cutCount).toDouble();
      rw.parameters[i].first = value;
      rw.parameters[i].second = parameters.at(i).mid(cutCount);
      }
   return rw;
   }

double ParserCSV::getData(QString line)
   {
   QStringList strList = line.split(',', QString::SkipEmptyParts);
   if (line.at(0) == '\"'){
      for (int i =0; i < strList.length(); i++){
         if (strList.at(i).compare("\"Record Length\"") == 0){
            len = strList.at(i+1).toLong();
            }

         if (strList.at(i).compare("\"Sample Interval\"") == 0){
            interval = strList.at(i+1).toDouble();
            }
         }
      }
   else
      {
      return strList.last().toDouble();
      }
   return 0.0;
   }

QDataStream &operator >>(QDataStream &stream, MeasurementParameters &data)
   {
   data.fileName.clear();
   stream >> data.fileName;
   for(auto a: data.parameters){
      stream >> a;
      }
   return stream;
   }

QDataStream &operator <<(QDataStream &stream, const MeasurementParameters &data)
   {
   stream << data.fileName;
   for(auto a: data.parameters){
      stream << a;
      }
   return stream;
   }

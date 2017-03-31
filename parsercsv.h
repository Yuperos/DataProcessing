#ifndef PARSERCSV_H
#define PARSERCSV_H

#include <QString>
#include <QStringList>
#include <Qpair>
#include <QDataStream>

enum ParametersName{
   PN_current =0,
   PN_frequency,
   PN_Power,
   };

struct MeasurementParameters {
   QString fileName;
   QPair<double, QString> parameters[3];

   MeasurementParameters& operator= (MeasurementParameters p1){
      fileName.clear();
      fileName.append(p1.fileName);
      for(int i=0; i<3; i++){
         parameters[i].first = p1.parameters[i].first;
         parameters[i].second.clear();
         parameters[i].second.append(p1.parameters[i].second);
         }
      return *this;
      }
   };

QDataStream &operator <<(QDataStream &stream,  const MeasurementParameters &data);
QDataStream &operator >>(QDataStream &stream, MeasurementParameters &data);

class ParserCSV
   {
public:
   static int infoLines;

   static uint64_t len;
   static double interval;

   ParserCSV();

   static MeasurementParameters getMeasureParameters(QString mFilePath);
   static MeasurementParameters parseFileName(QString fileName);
   static double getData(QString line);
   };

#endif // PARSERCSV_H

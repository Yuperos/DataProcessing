#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

#include <stdint.h>
#include <QList>
#include <QObject>

class SumRing
   {
public:
   uint16_t size = 0;
   double sum = 0;
   uint32_t wrIdx = 0;
   double* data = nullptr;

   SumRing(int len = 20);

   void setLen(uint32_t len);

   inline double getSum() { return sum; }
   inline double getAverage() { return getSum()/size; }
   double averaging(double value);

   inline void reset() { wrIdx = 0; sum = 0; }
   };


//===================================================================


class Trigger{
   enum idx{ top = 0, bot = 1 };
   idx i = top;
   double trig[2];
   uint64_t valCounter =0;
   uint64_t maxCount = 3000;

public:
   uint64_t counter;
   bool checkVal(double val){
      if((i == top && val > trig[i] && counter++) ||
         (i == bot && val < trig[i]))
         i = (idx)!i;
      return bool(i);
      }

   bool checkValAndCount(double val){
      if((i == top && val > trig[i] && counter++) ||
         (i == bot && (val < trig[i] || valCounter > maxCount))){
         i = (idx)!i;
         valCounter = 0;
         }
      ++valCounter;
      return bool(i);
      }

   operator bool() {return i;}

   double getUp() { return trig[top]; }
   double getDown() { return trig[bot]; }


   void setRange(double up, double down){
      if(up > down){
         trig[0] = up;
         trig[1] = down;
         }
      else
         {
         trig[0] = down;
         trig[1] = up;
         }
      }

   Trigger(){ init(); }

   Trigger(double up, double down){
      init();
      setRange(up, down);
      }

   void  init(){ counter =0; valCounter =0; }
   };


//===================================================================


class ConvolutionTool
   {
   const double timeConst  = 3.6621094E-01;
   const double amp = 1.18;
   const double offsetY = -0.1;

   uint32_t size = 1<<11;
   uint32_t sizeMask = size - 1;

   uint32_t beginIdx;
   uint32_t endIdx;

   double* waveletArray;
   double* convolutionArray;

   double wavelet1(double x);

   void init();
   void initWaveletArray();
   void initConvolArray();

public:
   ConvolutionTool();
   ConvolutionTool(const ConvolutionTool &other);
   ConvolutionTool& operator =(const ConvolutionTool & other);

   double convolute(double val);
   };


//===================================================================


class ProcessingUnit
   {
public:
   double offsetCh1[2] = {0.4,0.27};
   double offsetCh2[2] = {1.21E-6,0};

   SumRing summizer;
   Trigger trigCh1;
   Trigger trigCh2;
   ConvolutionTool tool;
   QList<double> *outList;
   QList<QPair<double,double>> * inList;
   uint64_t counter = 0;

   ProcessingUnit();
   ProcessingUnit(const ProcessingUnit &other);
   ~ProcessingUnit(){}

   void initTrigger();
   void init();

   void process();
   void setLists(QList<QPair<double,double>> *src, QList<double> *dst);
   };

#endif // PROCESSINGUNIT_H

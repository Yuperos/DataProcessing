#include "processingunit.h"
#include <math.h>

ProcessingUnit::ProcessingUnit()
   {
   init();
   }

ProcessingUnit::ProcessingUnit(const ProcessingUnit &other)
   {
   init();
   summizer  = other.summizer ;
   trigCh1      = other.trigCh1     ;
   tool      = other.tool     ;
   outList   = other.outList  ;
   inList    = other.inList   ;
   }

void ProcessingUnit::initTrigger()
   {
   trigCh1.init();
   trigCh1.setRange(offsetCh1[0],offsetCh1[1]);
   trigCh2.init();
   trigCh2.setRange(offsetCh2[0],offsetCh2[1]);
   }

void ProcessingUnit::init()
   {
   summizer.setLen(20);
   initTrigger();
   }

void ProcessingUnit::process()
   {
   int max = 10000;
   while(counter < inList->length() && ((counter++)%max) < (max-1) ){
      auto ch12 = inList->at(counter-1);
//      double tempVal = summizer.averaging(ch12.second);
      if (trigCh1.checkValAndCount(ch12.first)){
         double temp = tool.convolute(ch12.second); //tempval
         trigCh2.checkVal(temp);
         if (temp) outList->append(temp);
         }
      }
   }

void ProcessingUnit::setLists(QList<QPair<double, double> > *src, QList<double> *dst)
   {
   inList = src;
   outList = dst;
   }


SumRing::SumRing(int len) {
   setLen(len);
   }

void SumRing::setLen(uint32_t len)
   {
   if (data != nullptr)
      delete[] data;
   size = len;
   data = new double[len];
   }

double SumRing::averaging(double value)
   {
   sum += (value - data[wrIdx]);
   data[wrIdx] = value;
   wrIdx = (wrIdx+1)%size;
   return getAverage();
   }

double ConvolutionTool::wavelet1(double x)
   {
   return (x>0.083)? amp*exp(-x/timeConst) + offsetY : 0.01 ;
   }

ConvolutionTool::ConvolutionTool()
   {
   init();
   initConvolArray();
   initWaveletArray();
   }

ConvolutionTool::ConvolutionTool(const ConvolutionTool &other)
   {
   init();
   initConvolArray();
   initWaveletArray();
   beginIdx               =  other.beginIdx         ;
   endIdx                 =  other.endIdx           ;
   waveletArray           =  other.waveletArray     ;
   convolutionArray       =  other.convolutionArray ;
   }

ConvolutionTool &ConvolutionTool::operator =(const ConvolutionTool &other)
   {
   beginIdx               =  other.beginIdx         ;
   endIdx                 =  other.endIdx           ;
   waveletArray           =  other.waveletArray     ;
   convolutionArray       =  other.convolutionArray ;
   return *this;
   }

void ConvolutionTool::init()
   {
   beginIdx = 0;
   endIdx = 0;
   }

void ConvolutionTool::initWaveletArray()
   {
   double fullSum=0;
   if (waveletArray != nullptr)
      delete[] waveletArray;
   waveletArray = new double[size];

   for(uint32_t i = 0; i < size; i++){
      double tempY = wavelet1((double)i/size);
      fullSum += tempY;
      waveletArray[i] = tempY;
      }

   for(uint32_t i = 0; i < size; i++){
      waveletArray[i] /= fullSum;
      }
   }

void ConvolutionTool::initConvolArray()
   {
   if (convolutionArray != nullptr)
      delete[] convolutionArray;
   convolutionArray = new double[size]{0};
   }

double ConvolutionTool::convolute(double val)
   {
   convolutionArray[beginIdx] = 0;
   //N always [1..size]
   uint32_t n = ((size + endIdx - beginIdx) & sizeMask) + 1;
   endIdx = (endIdx+1) & sizeMask;
   for(uint32_t i = 0; i < n; ++i)
      convolutionArray[(i+beginIdx) & sizeMask] += val*waveletArray[i];
   //use conditions lasyness for increase beginIdx
   return (beginIdx == endIdx && (beginIdx = (beginIdx+1) & sizeMask))
          ? convolutionArray[endIdx]
          : 0;

   }


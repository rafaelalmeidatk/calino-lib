#ifndef Calino_h
#define Calino_h

#include "Arduino.h"
class Calino
{
  public:
    Calino();

    // send data
    void putData(int id, int value);
    void commit();

    // receive data
    bool gatherData();
    void processByte(char byteChar);
    int getValue(int id);
   
  private:
    int _retries;
    float _maxTimeout;
    float _timeoutMillis;

    bool _requestSent;
    int _sending;
    int _receivedAll;
    int** _data;
    int _dataSize;

    void requestData();
    void addDataEntry();
};

#endif
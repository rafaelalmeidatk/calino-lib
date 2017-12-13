#include "Arduino.h"
#include "Calino.h"

Calino::Calino()
{
  _data = (int**)malloc(sizeof(int*));
  _maxTimeout = 2000;
}

void Calino::putData(int id, int value)
{
  if (!_sending) {
    _sending = true;
    Serial.print("{{");
  }
  Serial.print("[");
  Serial.print(id);
  Serial.print(",");
  Serial.print(value);
  Serial.print("]");
}

void Calino::commit()
{
  _sending = false;
  Serial.print("}}");
  Serial.println();
  Serial.flush();
}

void Calino::requestData() {
  _requestSent = true;
  Serial.println("{data}");
  delay(500);

  _timeoutMillis = millis();
}

bool Calino::gatherData() {
  if (!_requestSent) {
    requestData();
  }

  static bool reading = false;
  char startMarker = '{';
  char endMarker = '}';
  char rc;

  if (millis() > _timeoutMillis + _maxTimeout) {
    _requestSent = false;
    reading = false;
    processByte('r');
    _retries = _retries >= 2 ? 2 : _retries + 1;
    _maxTimeout = 2000 * 1 << _retries;
    return false;
  }
  
  while (Serial.available() > 0 && !_receivedAll) {
    rc = Serial.read();
    if (reading) {
      if (rc != endMarker) {
        processByte(rc);
      } else {
        reading = false;
        _receivedAll = true;
      }
    } else if (rc == startMarker) {
      reading = true;
    }
  }

  return _receivedAll;
}

void Calino::processByte(char byteChar) {
  static int i = 0;
  static int j = 0;
  static int id = 0;
  static int val = 0;
  static int sign = 1;

  // r stands for reset
  if (byteChar == 'r') {
    i = 0;
    j = 0;
    id = 0;
    val = 0;
    sign = 1;
    return;
  }

  switch (byteChar) {
    case '-':
      sign = -1;
      break;

    case '[':
      val = 0;
      j = 0;
      id = 0;
      sign = 1;
      addDataEntry();
      break;

    case ',':
      _data[i][j++] = val;
      val = 0;
      break;

    case ']':
      _data[i++][j] = val * sign;
      break;

    case ' ':
      break;
    default:
      val = (byteChar - 48) + val * 10;
      break;
  }
}

void Calino::addDataEntry() {
  int newSize = _dataSize + 1;

  int** newArray = (int**)malloc(sizeof(int*) * newSize);
  for (int i = 0; i < _dataSize; i++) {
    newArray[i] = _data[i];
  }
  newArray[_dataSize++] = (int*)malloc(sizeof(int) * 2);

  delete[] _data;
  _data = newArray;
}

int Calino::getValue(int id) {
  for (int i = 0; i < _dataSize; i++) {
    if (_data[i][0] == id) {
      return _data[i][1];
    }
  }
  return -1;
}
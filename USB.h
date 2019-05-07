#pragma once

#define FTD2XX_STATIC

#include <iostream>
#include <string>
#include <cstdint>
#include "FTDI/ftd2xx.h"
#include "MessageFormats.h"

using namespace std;

typedef void* readPt;

class USB{
   public:
      USB(string & message);
      ~USB();

      void send(void* data, size_t size, string & message);

      void reset(string & message);

      bool connected();

   private:
      FT_STATUS ftStatus;
      FT_DEVICE_LIST_INFO_NODE *devInfo;
      FT_HANDLE handle;
      DWORD numDevs;

      bool boardFound = false;
      UCHAR RESET_PIN_MASK = 0b1000;

//      const string FT201XQ = "FT201X USB I2C";
      const string BOARD_NAME = "RichArduino";
      const DWORD txBufferSize = 512, rxBufferSize = 512;

      void toggleReset(string & message);
};

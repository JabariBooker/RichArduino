#pragma once

#define FTD2XX_STATIC

#include <iostream>
#include <string>
#include "FTDI/ftd2xx.h"
#include <cstdint>

using namespace std;

typedef void* readPt;

class USB{
   public:
      USB(string & message);
      ~USB();
      void send(void* data, size_t size, string & message);

      bool read(readPt data, int & size, string & message);

	  bool initialized() { return boardFound;  }

   private:
      FT_STATUS ftStatus;
      FT_DEVICE_LIST_INFO_NODE *devInfo;
      FT_HANDLE handle;
      DWORD numDevs;

      bool boardFound = false;

      const string FT201XQ = "FT201X USB I2C";
      const DWORD txBufferSize = 512, rxBufferSize = 512;
};

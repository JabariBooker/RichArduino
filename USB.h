#pragma once

#define FTD2XX_STATIC
//#define FTD2XX_EXPORTS


#include <iostream>
#include <string>
#include "FTDI/ftd2xx.h"
#include <cstdint>
#include <QTextStream>

using namespace std;

typedef void* readPt;

class USB{
   public:
      USB();
      ~USB();
      void send(void* data, size_t size);

	  void read(readPt data, int & size);

	  bool initialized() { return boardFound;  }

   private:
      FT_STATUS ftStatus;
      FT_DEVICE_LIST_INFO_NODE *devInfo;
      FT_HANDLE handle;
      DWORD numDevs;

      bool boardFound = false,
           devInfoAlloc = false;

      const string FT201XQ = "FT201X USB I2C";
      const DWORD txBufferSize = 512, rxBufferSize = 512;

	  QTextStream& qStdOut()
	  {
		  static QTextStream ts(stdout);
		  return ts;
	  }
};

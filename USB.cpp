#include "USB.h"

USB::USB(string & message){
   ftStatus = FT_CreateDeviceInfoList(&numDevs); 

   if(ftStatus != FT_OK){
      message = "Could not create info list for FTDI devices!";
      return;
   }

   if(numDevs != 0) {

      devInfo = new FT_DEVICE_LIST_INFO_NODE[numDevs];

      ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
      if(ftStatus != FT_OK){
         message = "Could not fetch info for FTDI devices!";
		 return;
      }

      unsigned int index;

      for(size_t i=0; i<numDevs; ++i){
         string des = devInfo[i].Description;
         if(des == FT201XQ){
            boardFound = true;
            index = i;
         }
      }

      if(!boardFound){
         message = "Could not find FT201XQ!";
         return;
      }

      ftStatus = FT_Open(index, &handle);
      if(ftStatus != FT_OK){
         message = "Could not gain access to FT201XQ!";
         return;
      }

      message = "Connected to RichArduino!";
   }
   else {
       message = "Could not find RichArduino!";
   }
}

USB::~USB(){
   FT_Close(handle);
   if(devInfo) delete[] devInfo;
}

void USB::send(void* data, size_t size, string & message){
   
    if (!boardFound) {
        message = "Cannot find RichArduino!";
		return;
	}

	uint8_t *curr = (uint8_t*)data,
			*end  = (uint8_t*)data + size;
   
	DWORD txBufferAmount,
			rxBufferAmount,
			eventStatus,
			bytesToWrite,
			bytesWritten;

	while(curr != end){
		ftStatus = FT_GetStatus(handle, &rxBufferAmount, &txBufferAmount, &eventStatus);
		if(ftStatus != FT_OK){
            message = "Unable to check status of FT201XQ!";
			return;
		}
      
		bytesToWrite = txBufferSize - txBufferAmount;

		if(bytesToWrite > size){
			bytesToWrite = size;
		}
		else if(bytesToWrite > end - curr){
			bytesToWrite = end - curr;
		}

        ftStatus = FT_Write(handle, curr, bytesToWrite, &bytesWritten);
		if(ftStatus != FT_OK){
            message = "Failed to write data!";
            return;
		}
		else if(bytesWritten != bytesToWrite){
            message = "Failed to write all data to USB!";
			return;
		}

		curr += bytesWritten;
	}

    message = "Wrote to USB";
}

bool USB::read(readPt data, int & size, string & message) {

    if(data != nullptr){
        delete[] data;
	}

	DWORD txBufferAmount,
		rxBufferAmount,
		eventStatus,
		bytesRead;

	ftStatus = FT_GetStatus(handle, &rxBufferAmount, &txBufferAmount, &eventStatus);
	if (ftStatus != FT_OK) {
        message = "Unable to check status of FT201XQ!";
        return false;
	}

	data = new uint8_t[rxBufferAmount];

	size = -1;

	ftStatus = FT_Read(handle, data, rxBufferAmount, &bytesRead);
	if (ftStatus != FT_OK) {
        message = "Unable to read from USB!";
        delete[] data;
        return false;
	}
	if (bytesRead != rxBufferAmount) {
        message = "Did not read all data from USB!";
        delete[] data;
        return false;
	}

    message = "Read from RichArduino";
	size = bytesRead;

    return true;
}

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
         message = mesAlert + "Could not fetch info for FTDI devices!" + mesEnd;
         return;
      }

      unsigned int index;

      for(size_t i=0; i<numDevs; ++i){
         string des = devInfo[i].Description;
         if(des == BOARD_NAME){
            boardFound = true;
            index = i;
         }
      }

      if(!boardFound){
         message = mesAlert + "Could not find FT201XQ!" + mesEnd;
         return;
      }

      ftStatus = FT_Open(index, &handle);
      if(ftStatus != FT_OK){
         message = mesAlert + "Could not create handle for FT201XQ!" + mesEnd;
         return;
      }

      message = mesSuccess + "Connected to RichArduino!" + mesEnd;
   }
   else {
      message = mesAlert + "Could not find RichArduino!" + mesEnd;
   }
}

USB::~USB(){
   FT_Close(handle);
   if(devInfo) delete[] devInfo;
}

void USB::reset(string & message){

//    message = "done";
//    UCHAR getMode;
//    ftStatus = FT_GetBitMode(handle, &getMode);
//    if(ftStatus == FT_OK){
//        std::cout << std::hex << (unsigned int)getMode << std::endl;
//    }

//    //writing reset pin high
//    UCHAR mask = 0x88;
//    ftStatus = FT_SetBitMode(handle, mask, 0x20);    //CBUS Bit Bang

//    if(ftStatus != FT_OK){
//        message = mesAlert + "(1)Unable to reset RichArduino!" + mesEnd;
//        return;
//    }

//    //waiting for reset time
//    Sleep(2000);
//    cout << "Done high" << endl;

//    //writing reset pin back low
//    mask = 0x80;
//    ftStatus = FT_SetBitMode(handle, mask, 0x20);    //CBUS Bit Bang

//    if(ftStatus != FT_OK){
//        message = mesAlert + "(2)Unable to reset RichArduino!" + mesEnd;
//        return;
//    }

//    Sleep(2000);
//    cout << "Done low" << endl;

//    ftStatus = FT_SetBitMode(handle, 00, 0x20);    //CBUS Bit Bang

//    message = mesSuccess + "RichArduino was reset!" + mesEnd;
}

void USB::send(void* data, size_t size, string & message){
   
    if (!boardFound) {
        message = mesAlert + "Cannot find RichArduino!" + mesEnd;
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
            message = mesAlert + "Unable to check status of FT201XQ!" + mesEnd;
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
            message = mesAlert + "Failed to write data!" + mesEnd;
            return;
		}
		else if(bytesWritten != bytesToWrite){
            message = mesAlert + "Failed to write all data to USB!" + mesEnd;
			return;
		}

		curr += bytesWritten;
	}

    message = mesSuccess + "Wrote to USB" + mesEnd;
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
        message = mesAlert + "Unable to check status of FT201XQ!" + mesEnd;
        return false;
	}

	data = new uint8_t[rxBufferAmount];

	size = -1;

	ftStatus = FT_Read(handle, data, rxBufferAmount, &bytesRead);
	if (ftStatus != FT_OK) {
        message = mesAlert + "Unable to read from USB!" + mesEnd;
        delete[] data;
        return false;
	}
	if (bytesRead != rxBufferAmount) {
        message = mesAlert + "Did not read all data from USB!" + mesEnd;
        delete[] data;
        return false;
	}

    message = mesSuccess + "Read from RichArduino" + mesEnd;
    size = bytesRead;

    return true;
}

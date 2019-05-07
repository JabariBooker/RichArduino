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
         message = mesAlert + "Could not find RichArduino!" + mesEnd;
         return;
      }

      ftStatus = FT_Open(index, &handle);
      if(ftStatus != FT_OK){
         message = mesAlert + "Could not create handle for RichArduino!" + mesEnd;
         return;
      }

      message = mesSuccess + "Connected to RichArduino!" + mesEnd;
   }
   else {
      message = mesAlert + "No FTDI devices found!" + mesEnd;
   }
}

bool USB::connected(){
    DWORD txBufferAmount, rxBufferAmount, eventStatus;

    ftStatus = FT_GetStatus(handle, &rxBufferAmount, &txBufferAmount, &eventStatus);
    return ftStatus == FT_OK;
}

USB::~USB(){
   FT_Close(handle);
   if(devInfo) delete[] devInfo;
}

void USB::reset(string & message){

    //writing reset pin high
    UCHAR mask = 0x88;
    ftStatus = FT_SetBitMode(handle, mask, 0x20);    //CBUS Bit Bang

    if(ftStatus != FT_OK){
        message = mesAlert + "Unable to reset RichArduino!" + mesEnd;
        return;
    }

    //waiting for reset time
    Sleep(2000);

    //writing reset pin back low
    mask = 0x80;
    ftStatus = FT_SetBitMode(handle, mask, 0x20);    //CBUS Bit Bang

    if(ftStatus != FT_OK){
        message = mesAlert + "Unable to reset RichArduino!" + mesEnd;
        return;
    }

    Sleep(2000);

    //going back to original bit mode
    ftStatus = FT_SetBitMode(handle, 00, 0x20);    //CBUS Bit Bang

    message = mesSuccess + "RichArduino was reset!" + mesEnd;
}

void USB::send(void* data, size_t size, string & message){
   
    if (!connected()) {
        message = mesAlert + "Not connected to RichArduino!" + mesEnd;
        return;
    }

	uint8_t *curr = (uint8_t*)data,
			*end  = (uint8_t*)data + size;
   
	DWORD txBufferAmount,
			rxBufferAmount,
			eventStatus,
			bytesToWrite,
			bytesWritten;

    size_t attempts = 0;

	while(curr != end){
        ++attempts;
		ftStatus = FT_GetStatus(handle, &rxBufferAmount, &txBufferAmount, &eventStatus);
		if(ftStatus != FT_OK){
            message = mesAlert + "Unable to check status of RichArduino!" + mesEnd;
			return;
        }

        if(txBufferAmount == txBufferSize){
            ++attempts;

            if(attempts >= 1000){
                message = mesAlert + "Send timed-out, failed to write all data!" + mesEnd;
                return;
            }
            continue;
        }
        else attempts = 0;


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

#include "USB.h"

USB::USB(){
   ftStatus = FT_CreateDeviceInfoList(&numDevs); 

   if(ftStatus != FT_OK){
	  qStdOut() << "Could not create info list for FTDI devices!" << endl;
      return;
   }
   
   printf("%lu FTDI devices found!\n", numDevs);

   devInfoAlloc = false;

   if(numDevs != 0){

      devInfo = new FT_DEVICE_LIST_INFO_NODE[numDevs];
      devInfoAlloc = true;

      ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
      if(ftStatus != FT_OK){
		 qStdOut() << "Could not fetch info for FTDI devices!\n" << endl;
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
		 qStdOut() << "Could not find FT201XQ!" << endl;
         return;
      }

      ftStatus = FT_Open(index, &handle);
      if(ftStatus != FT_OK){
		 qStdOut() << "Could not gain access to FT201XQ!" << endl;
         return;
      }
   }
}

USB::~USB(){
   FT_Close(handle);
   if(devInfoAlloc) delete[] devInfo;
}

void USB::send(void* data, size_t size){
   
	if (!boardFound) {
		qStdOut() << "Cannot find RichArduino!" << endl;
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
			qStdOut() << "Unable to check status of FT201XQ!" << endl;
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
			cout << "Failed to write data!" << endl;
			exit(-1);
		}
		else if(bytesWritten != bytesToWrite){
			qStdOut() << "Failed to write all data to USB!" << endl;
			return;
		}

		curr += bytesWritten;
	}

	qStdOut() << "Wrote to USB" << endl;
}

void USB::read(readPt data, int & size) {

	if(data != nullptr){
		delete[] data;
	}

	DWORD txBufferAmount,
		rxBufferAmount,
		eventStatus,
		bytesRead;

	ftStatus = FT_GetStatus(handle, &rxBufferAmount, &txBufferAmount, &eventStatus);
	if (ftStatus != FT_OK) {
		qStdOut() << "Unable to check status of FT201XQ!" << endl;
		return;
	}

	data = new uint8_t[rxBufferAmount];

	size = -1;

	ftStatus = FT_Read(handle, data, rxBufferAmount, &bytesRead);
	if (ftStatus != FT_OK) {
		qStdOut() << "Unable to read from USB!" << endl;
		return;
	}
	if (bytesRead != rxBufferAmount) {
		qStdOut() << "Did not read all data from USB!" << endl;
		return;
	}

	size = bytesRead;

}

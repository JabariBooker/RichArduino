#include "Assembler.h"

string Assembler::assemble(string & code, string & message){
   map<string, size_t> labels;
   vector<string> lines;

   istringstream codeISS(code);

   string line;
   while(std::getline(codeISS, line)){
     lines.push_back(line);
   }

   //getting start address of code
   size_t codeStart;
   string org;


   //looking for first actual line of code
   string firstCodeLine;
   for(string line : lines){
       size_t start = line.find_first_not_of(" \t");
       if(start == line.find("//") || start == string::npos) continue;
       else{
           firstCodeLine = line;
           break;
       }
   }

   if(firstCodeLine.find(".org") == string::npos){
      message = "code doesn't start with \".org\"!";
      return "";
   }

   istringstream ss(firstCodeLine);
   ss >> org >> codeStart;

   //first pass: looking for label positioning
   size_t instrCounter = 0,
          codeLine = 1;
   for(size_t i = 0; i < lines.size(); ++i){
      string line = lines[i];

      size_t start = line.find_first_not_of(" \t");
      if(line == "" || start == string::npos){
          ++codeLine;
          continue;
      }

	  //Comment detection
	  size_t commentStart = line.find("//");
	  if (commentStart != string::npos) {
		  line.erase(commentStart);
	  }

	  if (line.find(".org") != string::npos){
		  istringstream iss(line);
          iss >> org >> instrCounter;
          lines[i] = line.substr(start);
          ++codeLine;
		  continue;
	}

      size_t labelEnd = line.find(":");
      if(labelEnd != string::npos){
		
		string label = line.substr(start, labelEnd - start);

        //checking if label is valid
        if(!checkLabel(label)){
            message = "Invalid label at line " + to_string(codeLine) + "!";
            return "";
        }

		//support for .equ convention
		size_t equStart = line.find(".equ");
		if (equStart != string::npos) {
			istringstream iss(line.substr(equStart + 4));
			size_t value;
			iss >> value;
			labels[label] = value;
		}
        else{
            labels[label] = instrCounter;
        }
        start = labelEnd + 1;
      }

      //getting rid of leading spaces and labels to make second pass easier
      lines[i] = line.substr(start);

      instrCounter += 4;
      ++codeLine;
   }

   //second pass: assembling code
   string machineCode = "";
   uint32_t lineNum = codeStart;
   codeLine = 1;
      
   for(string line : lines){
      
      //if the line is empty ignore it
      if (line == "" || line.find_first_not_of(" \t") == string::npos){
          ++codeLine;
          continue;
      }

	  //Comment detection
	  size_t commentStart = line.find("//");
	  if (commentStart != string::npos) {
		  line.erase(commentStart);
          //checking again now if the line is empty
          if (line == "" || line.find_first_not_of(" \t") == string::npos){
              ++codeLine;
              continue;
          }
	  }

      //searching for labels in current line and replacing them with their value
	  string label = "";
	  size_t value, labelPos = string::npos, labelLen = 0;
      for(map<string, size_t>::iterator it = labels.begin(); it != labels.end(); ++it){

         string newLabel = it->first;
		 size_t newValue = it->second,
			 newLabelPos = line.find(newLabel),
             newLabelLen = newLabel.size();

         if(newLabelPos != string::npos){
			 if ((newLabelLen > labelLen) || (labelPos == string::npos)) {
				 label = newLabel;
				 labelPos = newLabelPos;
				 labelLen = newLabelLen;
				 value = newValue;
			 }
         }
      }
      if(label != ""){
          line.replace(labelPos, labelLen, to_string(value));
      }

      //getting rid of commas
	   replaceAll(line, ",", " ");

      //getting instruction/pseudo op
	   istringstream iss(line);
	   string instr;
      iss >> instr;


      /**Support for RSRC Assembly Psuedo Operations**/

      if (instr == ".org") {    //describes where the next instruction will be stored
          uint32_t address;
          iss >> address;

          if(address < lineNum || address % 4 != 0){
              message = ".org address is not valid. Either violates linear ordering or is not word-aligned.";
              return "";
          }

          if(address == lineNum){
              ++codeLine;
              continue;
          }

          while(lineNum < address){
              machineCode.insert(machineCode.size(), "00000000\n");
              lineNum += 4;
          }

          ++codeLine;
          continue;
	  }
      else if (instr == ".equ") {	//already handled, ignore
          ++codeLine;
		  continue;
	  }
      else if (instr == ".dc") {		//allocate and set 32-bit values
		  uint32_t value;
		  while (iss >> value) {
			  stringstream hexStream;
			  hexStream << setw(8) << setfill('0') << hex << value;
              machineCode.insert(machineCode.size(), hexStream.str() + "\n");
			  lineNum += 4;
		  }

          ++codeLine;
		  continue;
	  }
      else if (instr == ".dcb") {	//allocate and set 8 bit values
		  uint32_t word = 0;
          size_t value, count = 0;
		  while (iss >> value) {
              word <<= 8;
              word |= value & 0xff;
			  ++count;
			  if (count % 4 == 0) {
				  stringstream hexStream;
				  hexStream << setw(8) << setfill('0') << hex << word;
                  machineCode.insert(machineCode.size(), hexStream.str() + "\n");
				  lineNum += 4;
				  word = count = 0;
			  }
		  }

		  if (word != 0) {
			  stringstream hexStream;
			  hexStream << setw(8) << setfill('0') << hex << word;
			  machineCode.insert(machineCode.size(), hexStream.str());
			  lineNum += 4;
		  }

          ++codeLine;
		  continue;
	  }
      else if (instr == ".dch") {	//allocate and set 16 bit values
		  uint32_t word = 0;
          size_t value, count = 0;
		  while (iss >> value) {
			  word <<= 16;;
			  word |= value & 0xffff;
			  ++count;
			  if (count % 2 == 0) {
				  stringstream hexStream;
				  hexStream << setw(8) << setfill('0') << hex << word;
				  machineCode.insert(machineCode.size(), hexStream.str() + "\n");
				  lineNum += 4;
				  word = count = 0;
			  }
		  }

		  if (word != 0) {
			  stringstream hexStream;
			  hexStream << setw(8) << setfill('0') << hex << word;
			  machineCode.insert(machineCode.size(), hexStream.str() + "\n");
			  lineNum += 4;
		  }

          ++codeLine;
		  continue;
      }
      else if (instr == ".dw") {	//allocate 32-bit words
          size_t count;
          iss >> count;
          for(size_t i=0; i<count; ++i){
              machineCode.insert(machineCode.size(), "00000000\n");
              lineNum += 4;
          }

          ++codeLine;
          continue;
	  }


      /**instr doesn't match a psuedo op, meaning check for instructions now**/

      //getting instruction opcode and putting into line
      Instruction in;
      uint32_t binaryLine;
      string ra, rb, rc, c1, c2, c3;

      try{
          in = instructions.at(instr);
          binaryLine = in.opcode;
      }catch(exception e){
          message = "Invalid instruction or psuedo op at line " +
                  to_string(codeLine) + "!";
          return "";
      }


      /**determining instruction format**/


      //checking if instr is a shift instruction
      if(in.group != SHIFT){
         
         //can remove r's from register names to get register numbers
		 replaceAll(line, "r", "");

         //greating a new stringstring for newly modified line
         istringstream iss_m(line);
         string toss;
         iss_m >> toss;       //prevent duplicate fetch of instruction

         //ld, st, la, addi, andi, ori
         if(in.group == MEM_IMMED){
            iss_m >> ra;

            if(instr == "ld" || instr == "st" || instr == "la"){
               if (line.find('(') != string::npos) {
                  string next;
                  iss_m >> next;

                  size_t rbStart = next.find('('),
                        rbEnd = next.find(')');

                  rb = next.substr(rbStart + 1, rbEnd - rbStart - 1);

                  if (rbStart == 0) c2 = "0";
                  else c2 = next.substr(0, rbStart);
               }
               else {
                  rb = "0";
                  iss_m >> c2;
               }
            }
            else{
               iss_m >> rb;
               iss_m >> c2;
            }

            if(!checkStringsNumberic(ra, rb, c2, "0", 0b0010)){
                  message = "Non-numeric register/constant value at line " + to_string(codeLine) + "!";
                  return "";
            }
            
            uint32_t raVal = stoul(ra),
                     rbVal = stoul(rb),
                     c2Val = stoul(c2);

            if(!checkRegisterVal(raVal, rbVal)){
                  message = "Invalid register at line " + to_string(codeLine) + "!";
                  return "";
            }

            if(!checkConstant(c2Val, C2)){
                  message = "Invalid c2 constant at line " + to_string(codeLine) + "!";
                  return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rbVal & REG_MASK);

            binaryLine <<= 17;
            binaryLine |= (c2Val & C2_MASK);
         }

         //ldr, str, lar
         else if(in.group == MEM_REL){
            iss_m >> ra;
            iss_m >> c1;

            if(!checkStringsNumberic(ra, c1, "0", "0", 0b0100)){
                  message = "Non-numeric register/constant value at line " + to_string(codeLine) + "!";
                  return "";
            }
            
            uint32_t raVal = stoul(ra),
                     c1Val = stoul(c1);

            c1Val -= lineNum;

            if(!checkRegisterVal(raVal)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            if(!checkConstant(c1Val, C1)){
                message = "Invalid c1 constant at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<=22;
            binaryLine |= (c1Val & C1_MASK);
         }
         
         //neg, not
         else if(in.group == NEG){
            iss_m >> ra;
            iss_m >> rc;

            if(!checkStringsNumberic(ra, rc)){
                  message = "Non-numeric register/constant value at line " + to_string(codeLine) + "!";
                  return "";
            }
            
            uint32_t raVal = stoul(ra),
                     rcVal = stoul(rc);

            if(!checkRegisterVal(raVal, rcVal)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;   //rbField unused

            binaryLine <<= 5;
            binaryLine |= (rcVal & REG_MASK);

            binaryLine <<= 12;
         }
         
         //br, brl
         else if(in.sec_group == BR_ALL){
            uint32_t con = 0;
            c3 = "0";

            if(in.group == BRL) iss_m >> ra;
            else ra = "0";

            if(instr == "brnv" || instr == "brlnv"){
               rb = rc = "0";
            }
            else{
               
               iss_m >> rb;

               if(instr == "br" || instr == "brl"){
                  if(!(iss_m >> rc)){
                     rc = "0";
                     con = 1;
                  }
                  else{
                     iss_m >> c3;
                  }
               }
               else{
                  
                  iss_m >> rc;

                  if(instr == "brzr" || instr == "brlzr")      con = 2;
                  else if(instr == "brnz" || instr == "brlnz") con = 3;
                  else if(instr == "brpl" || instr == "brlpl") con = 4;
                  else con = 5;     //br(l)mi
               }
            }

            if(!checkStringsNumberic(ra, rb, rc, c3, 0b0001)){
                  message = "Non-numeric register/constant value at line " + to_string(codeLine) + "!";
                  return "";
            }
            
            uint32_t raVal = stoul(ra),
                     rbVal = stoul(rb),
                     rcVal = stoul(rc),
                     c3Val = stoul(c3);

            if(!checkRegisterVal(raVal, rbVal, rcVal)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            if(!checkConstant(c3Val, C3)){
                message = "Invalid c3 constant at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rbVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rcVal & REG_MASK);

            binaryLine <<= 12;

            if(con == 0) binaryLine |= (c3Val & C3_MASK);
            else binaryLine |= (con & 3);
         }

         //add, sub, and, or
         else if(in.group == AASO){
            iss_m >> ra >> rb >> rc;

            if(!checkStringsNumberic(ra, rb, rc)){
                  message = "Non-numeric register/constant value at line " + to_string(codeLine) + "!";
                  return "";
            }
            
            uint32_t raVal = stoul(ra),
                     rbVal = stoul(rb),
                     rcVal = stoul(rc);

            if(!checkRegisterVal(raVal, rbVal, rcVal)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rbVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rcVal & REG_MASK);

            binaryLine <<= 12;
         }
         
         //nop, stop
         else{
            binaryLine <<= 27;
         }
      }

      //shr, shra, shl, shc
      else {
         string regA, regB, lastArg;

         iss >> ra >> rb >> lastArg;
         ra.replace(0, 1, "");
         rb.replace(0, 1, "");

         if(lastArg.find("r") == string::npos){
            
            if(!checkStringsNumberic(ra, rb, lastArg)){
                  message = "Non-numeric register or shift count value at line " + to_string(codeLine) + "!";
                  return "";
            }

            uint32_t raVal = stoul(ra), 
                     rbVal = stoul(rb),
                     count = stoul(lastArg);

            if(!checkRegisterVal(raVal, rbVal, count)){
                message = "Invalid register or shift count at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rbVal & REG_MASK);

            binaryLine <<= 17;
            binaryLine |= (count & REG_MASK);
         }
         else{
            lastArg.replace(0, 1, "");
            
            if(!checkStringsNumberic(ra, rb, lastArg)){
                  message = "Non-numeric register value at line " + to_string(codeLine) + "!";
                  return "";
            }

            uint32_t raVal = stoul(ra),
                     rbVal = stoul(rb),
                     rcVal = stoul(lastArg);

            if(!checkRegisterVal(raVal, rbVal, rcVal)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (raVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rbVal & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rcVal & REG_MASK);

            binaryLine <<= 12;
         }
      }

      stringstream hexStream;
      hexStream << setw(8) << setfill('0') << hex << binaryLine;

      //adding to machineCode output
      machineCode.insert(machineCode.size(), hexStream.str() + "\n");
      lineNum += 4;
      ++codeLine;
   }
   return machineCode;
}

void Assembler::replaceAll(string &in, string tar, string rep) {
	size_t pos;
	while ((pos = in.find(tar)) != string::npos) {
		in.replace(pos, tar.length(), rep);
	}
}

bool Assembler::checkRegisterVal(uint32_t reg1, uint32_t reg2,  uint32_t reg3){
    return !(reg1 > 31 || reg2 > 31 || reg3 > 31);
}

bool Assembler::checkStringsNumberic(string str1, string str2, string str3, string str4, uint8_t constantFlags){
   bool areValid = true;
   
   size_t size1 = str1.size(), size2 = str2.size(), size3 = str3.size(), size4 = str4.size();
   
   bool isConst1 = constantFlags & 0b1000,
        isConst2 = constantFlags & 0b0100,
        isConst3 = constantFlags & 0b0010,
        isConst4 = constantFlags & 0b0001;

   size_t maxLength = (size1 > size2) ? size1 : size2;
   maxLength = (maxLength > size3) ?  maxLength : size3;
   maxLength = (maxLength > size4) ?  maxLength : size4;

   for(size_t i=0; i < maxLength; ++i){
      if(i < size1){
         if(isConst1 && str1.at(i) == '-') areValid &= true;
         else areValid &= isdigit(str1.at(i));
      }
      if(i < size2){
         if(isConst2 && str2.at(i) == '-') areValid &= true;
         else areValid &= isdigit(str2.at(i));
      }
      if(i < size3){
         if(isConst3 && str3.at(i) == '-') areValid &= true;
         else areValid &= isdigit(str3.at(i));
      }
      if(i < size4){
         if(isConst4 && str4.at(i) == '-') areValid &= true;
         else areValid &= isdigit(str4.at(i));
      }

      if(!areValid) break;
   }

   return areValid;
}

bool Assembler::checkConstant(size_t constVal, CONSTANT_TYPE type){
    switch(type){
        case C1:
            return constVal < (1 << 22);
        case C2:
            return constVal < (1 << 17);
        case C3:
            return constVal < (1 << 12);
    }
}

bool Assembler::checkLabel(string label){
    bool isValid = true;


    size_t instrSize = instrNames.size(), regSize = registers.size();
    size_t largerSize = (instrSize > regSize) ? instrSize : regSize;

    for(size_t i = 0; i < largerSize; ++i){

        if(i < instrSize) isValid &= label != instrNames[i];
        if(i < regSize) isValid &= label != registers[i];

        if(!isValid) break;
    }

    return isValid;
}

#include "Assembler.h"

Assembler::Assembler(){
   instructions["nop"]   = Instruction(0, NO_OP);
   instructions["ld"]    = Instruction(1, MEM_IMMED);
   instructions["ldr"]   = Instruction(2, MEM_REL);
   instructions["st"]    = Instruction(3, MEM_IMMED);
   instructions["str"]   = Instruction(4, MEM_REL);
   instructions["la"]    = Instruction(5, MEM_IMMED);
   instructions["lar"]   = Instruction(6, MEM_REL);
   instructions["br"]    = Instruction(8, BR, BR_ALL);
   instructions["brnv"]  = Instruction(8, BR, BR_ALL);
   instructions["brzr"]  = Instruction(8, BR, BR_ALL);
   instructions["brnz"]  = Instruction(8, BR, BR_ALL);
   instructions["brpl"]  = Instruction(8, BR, BR_ALL);
   instructions["brmi"]  = Instruction(8, BR, BR_ALL);
   instructions["brl"]   = Instruction(9, BRL, BR_ALL);
   instructions["brlzr"] = Instruction(9, BRL, BR_ALL);
   instructions["brlnv"] = Instruction(9, BRL, BR_ALL);
   instructions["brlnz"] = Instruction(9, BRL, BR_ALL);
   instructions["brlpl"] = Instruction(9, BRL, BR_ALL);
   instructions["brlmi"] = Instruction(9, BRL, BR_ALL);
   instructions["add"]   = Instruction(12, AASO);
   instructions["addi"]  = Instruction(13, MEM_IMMED);
   instructions["sub"]   = Instruction(14, AASO);
   instructions["neg"]   = Instruction(15, NEG);
   instructions["and"]   = Instruction(20, AASO);
   instructions["andi"]  = Instruction(21, MEM_IMMED);
   instructions["or"]    = Instruction(22, AASO);
   instructions["ori"]   = Instruction(23, MEM_IMMED);
   instructions["not"]   = Instruction(24, NEG);
   instructions["shr"]   = Instruction(26, SHIFT);
   instructions["shra"]  = Instruction(27, SHIFT);
   instructions["shl"]   = Instruction(28, SHIFT);
   instructions["shc"]   = Instruction(29, SHIFT);
   instructions["stop"]  = Instruction(31, NO_OP);
}

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
   size_t instrCounter = 0;
   for(size_t i = 0; i < lines.size(); ++i){
      string line = lines[i];

      size_t start = line.find_first_not_of(" \t");
      if(line == "" || start == string::npos) continue;

	  //Comment detection
	  size_t commentStart = line.find("//");
	  if (commentStart != string::npos) {
		  line.erase(commentStart);
	  }

	  if (line.find(".org") != string::npos){
		  istringstream iss(line);
          iss >> org >> instrCounter;
          lines[i] = line.substr(start);
		  continue;
	}

      size_t labelEnd = line.find(":");
      if(labelEnd != string::npos){
		
		string label = line.substr(start, labelEnd - start);

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
   }

   //second pass: assembling code
   string machineCode = "";
   uint32_t lineNum = codeStart,
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

	  istringstream iss(line);
	  string instr;
      uint32_t ra, rb, rc, c1, c2, c3;

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

                    rb = stoul(next.substr(rbStart + 1, rbEnd - rbStart + 1));

					if (rbStart == 0) c2 = 0;
                    else c2 = stoul(next.substr(0, rbStart));
				}
				else {
					rb = 0;
					iss_m >> c2;
				}
			}
			else{
				iss_m >> rb;
				iss_m >> c2;
			}

            if(!checkRegister(ra, rb)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            if(!checkConstant(c2, C2)){
                message = "Invalid c2 constant at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rb & REG_MASK);

            binaryLine <<= 17;
            binaryLine |= (c2 & C2_MASK);
         }

         //ldr, str, lar
         else if(in.group == MEM_REL){
            iss_m >> ra;
            iss_m >> c1;

            c1 = c1 - lineNum;

            if(!checkRegister(ra)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            if(!checkConstant(c1, C1)){
                message = "Invalid c1 constant at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<=22;
            binaryLine |= (c1 & C1_MASK);
         }
         
         //neg, not
         else if(in.group == NEG){
            iss_m >> ra;
            iss_m >> rc;

            if(!checkRegister(ra, rc)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;   //rbField unused

            binaryLine <<= 5;
            binaryLine |= (rc & REG_MASK);

            binaryLine <<= 12;
         }
         
         //br, brl
         else if(in.sec_group == BR_ALL){
            uint32_t con = 0;
            c3 = 0;

            if(in.group == BRL) iss_m >> ra;
            else ra = 0;

            if(instr == "brnv" || instr == "brlnv"){
               rb = rc = 0;
            }
            else{
               
               iss_m >> rb;

               if(instr == "br" || instr == "brl"){
                  if(!(iss_m >> rc)){
                     rc = 0;
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

            if(!checkRegister(ra, rb, rc)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            if(!checkConstant(c3, C3)){
                message = "Invalid c3 constant at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rb & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rc & REG_MASK);

            binaryLine <<= 12;

            if(con == 0) binaryLine |= (c3 & C3_MASK);
            else binaryLine |= (con & 3);
         }

         //add, sub, and, or
         else if(in.group == AASO){
            iss_m >> ra >> rb >> rc;

            if(!checkRegister(ra, rb, rc)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rb & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rc & REG_MASK);

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

         iss >> regA >> regB >> lastArg;
         regA.replace(0, 1, "");
         regB.replace(0, 1, "");

         if(lastArg.find("r") == string::npos){
            ra = stoul(regA);
            rb = stoul(regB);
            uint32_t count = stoul(lastArg);

            if(!checkRegister(ra, rb, count)){
                message = "Invalid register or shift count at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rb & REG_MASK);

            binaryLine <<= 17;
            binaryLine |= (count & REG_MASK);
         }
         else{
            lastArg.replace(0, 1, "");

            ra = stoul(regA);
            rb = stoul(regB);
            rc = stoul(lastArg);

            if(!checkRegister(ra, rb, rc)){
                message = "Invalid register at line " + to_string(codeLine) + "!";
                return "";
            }

            binaryLine <<= 5;
            binaryLine |= (ra & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rb & REG_MASK);

            binaryLine <<= 5;
            binaryLine |= (rc & REG_MASK);

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

bool Assembler::checkRegister(uint32_t reg1, uint32_t reg2,  uint32_t reg3){
    return !(reg1 > 31 || reg2 > 31 || reg3 > 31);
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

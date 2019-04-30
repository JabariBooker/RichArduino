#include "Assembler.h"

string Assembler::assemble(string & code, string & message){
   map<string, string> labels;
   vector<vector<string>> lines;

   istringstream codeISS(code);

   string currLine;
   while(getline(codeISS, currLine)){

       //Comment detection
       size_t commentStart = currLine.find("//");
       if (commentStart != string::npos) {
           currLine.erase(commentStart);
       }

       //getting rid of commas and spacing out starting parentheses and colons
       replaceAll(currLine, ",", " ");

       size_t charPos = currLine.find('(');
       if(charPos != string::npos) currLine.replace(charPos, 1, " (");

       charPos = currLine.find(':');
       if(charPos != string::npos) currLine.replace(charPos, 1, ": ");

       vector<string> vLine;
       stringSplit(currLine, vLine);
       lines.push_back(vLine);
   }

   size_t codeLine = 1;

   //looking for first actual line of code
   vector<string>* firstCodeLine = nullptr;
   for(size_t i = 0; i < lines.size(); ++i){
       if(lines[i].size() != 0) {
           firstCodeLine = &(lines[i]);
           break;
       }
       ++codeLine;
   }
   if(firstCodeLine == nullptr){
       message = mesAlert + "No code to assemble!" + mesEnd;
       return "failed";
   }

   //getting start address of code
   size_t codeStart;
   string org;

   if((*firstCodeLine)[0] != ".org"){
      message =  mesError + "code doesn't start with .org pseudo op!" + mesEnd;
      return "failed";
   }
   else if(firstCodeLine->size() != 2){
       message =  mesError + ".org had incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
       return "failed";
   }

   string strCodeStart = (*firstCodeLine)[1];
   if(!checkStringNumberic(strCodeStart, true)){
       message = mesError + ".org has invalid parameter on line " + to_string(codeLine) + mesEnd;
       return "failed";
   }

   codeStart = toVal(strCodeStart);

   //first pass: looking for label positioning
   size_t instrCounter = 0;
   codeLine = 1;
   for(size_t i = 0; i < lines.size(); ++i){
      vector<string> & line = lines[i];

      if(line.size() == 0 ) continue;

      //checking for .equ used without label
      if(line[0] == ".equ"){
          message =  mesError + "incorrect used of .equ on line " + to_string(codeLine) + mesEnd;
          return "failed";
      }

      //setting line number based on .org (coincidentally checks for errors in .org's)
      if (line[0] == ".org"){
          if(line.size() != 2){
              message =  mesError + ".org had incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          if(!checkStringNumberic(line[1], true)){
              message = mesError + ".org has invalid parameter on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          instrCounter = toVal(line[1]);
          ++codeLine;
          continue;
      }

      //looking for proper label
      string label = "";
      if(line[0].find(':') == line[0].size()-1) label = line[0].substr(0, line[0].size()-1);
      else
      { //if one not found, checking that there is no stray colons
          for(string seg : line){
              if(seg.find(':') != string::npos){
                  message = mesError + "Unexpected ':' on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }
          }
      }

      if(!checkLabel(label)){
          message = mesError + "Invalid label on line " + to_string(codeLine) + "!" + mesEnd;
          return "failed";
      }

      if(label != ""){
          if(line.size() == 1) labels[label] = to_string(instrCounter);
          //handles .equ and check for numeric value for it
          else if(line[1] == ".equ"){
              if(line.size() != 3){
                  message =  mesError + ".equ had incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }
              if(!checkStringNumberic(line[2])){
                  message = mesError + ".equ has invalid parameter on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }

              labels[label] = line[2];
          }
          else labels[label] = to_string(instrCounter);

          //getting rid of label from line
          line.erase(line.begin());
      }

      instrCounter += 4;
      ++codeLine;
   }

   //second pass: assembling code
   string machineCode = "";
   uint32_t lineNum = codeStart;
   codeLine = 1;

   for(vector<string> line : lines){

      //if the line is empty ignore it
      if (line.size() == 0){
          ++codeLine;
          continue;
      }

      //searching for labels in current line and replacing them with their value
      for(size_t i = 0; i < line.size(); ++ i){
          try{
              string value = labels.at(line[i]);    //throws out_of_range if string line[i] is not a label
              line[i] = value;
          }catch(exception OOR){}
      }

      string instr = line[0];

      /**Support for RSRC Assembly Psuedo Operations**/

      if (instr == ".org") {    //describes where the next instruction will be stored
          if(line.size() != 2){
              message =  mesError + ".org has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          if(!checkStringNumberic(line[1], true)){
              message = mesError + ".org has invalid parameter on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          size_t address = toVal(line[1]);

          if(address < lineNum || address % 4 != 0){
              message = mesError + ".org address on line " + to_string(codeLine) +
                      " is not valid. Either violates linear ordering or is not word-aligned." + mesEnd;
              return "failed";
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
          uint32_t word;

          for(size_t i = 1; i < line.size(); ++i){
              if(!checkStringNumberic(line[i])){
                  message = mesError + ".dc has a invalid parameter on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }

              word = toVal(line[i]);
              stringstream hexStream;
              hexStream << setw(8) << setfill('0') << hex << word;
              machineCode.insert(machineCode.size(), hexStream.str() + "\n");
              lineNum += 4;
          }

          ++codeLine;
          continue;
      }
      else if (instr == ".dcb") {	//allocate and set 8 bit values
          uint32_t word = 0;
          uint32_t value, count = 0;

          for(size_t i = 1; i < line.size(); ++i){
              if(!checkStringNumberic(line[i])){
                  message = mesError + ".dcb has a invalid parameter on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }

              value = toVal(line[i]);
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
          uint32_t value, count = 0;

          for(size_t i = 1; i < line.size(); ++i){
              if(!checkStringNumberic(line[i])){
                  message = mesError + ".dch has a invalid parameter on line " + to_string(codeLine) + mesEnd;
                  return "failed";
              }

              value = toVal(line[i]);
              word <<= 16;
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
              machineCode.insert(machineCode.size(), hexStream.str());
              lineNum += 4;
          }

          ++codeLine;
          continue;
      }
      else if (instr == ".dw") {	//allocate 32-bit words
          if(line.size() != 2){
              message =  mesError + ".dw has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          if(!checkStringNumberic(line[1], true)){
              message = mesError + ".dch has a invalid parameter on line " + to_string(codeLine) + mesEnd;
              return "failed";
          }

          size_t count = toVal(line[1]);
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
          message = mesError + "Invalid instruction '" + instr + "' on line " + to_string(codeLine) + "!"  + mesEnd;
          return "failed";
      }



      /**determining instruction format**/

     //addi, andi, ori, ld, la, st
     if(in.group == MEM_IMMED){
        ra = line[1];

        if(instr == "ld" || instr == "st" || instr == "la"){
            if(line.size() == 3){
                size_t rbStart = line[2].find('('),
                       rbEnd = line[2].find(')');

                if(rbStart == 0 && rbEnd == line[2].size() - 1){
                    rb = line[2].substr(1, line[2].size() - 2);
                    c2 = "0";
                }
                else if(rbStart == string::npos && rbEnd == string::npos){
                    c2 = line[2];
                    rb = "r0";
                }
                else{
                    message = mesError + "Fix parentheses on line " + to_string(codeLine) + "!"  + mesEnd;
                    return "failed";
                }
            }
            else if(line.size() == 4){
                c2 = line[2];

                size_t rbStart = line[3].find('('),
                       rbEnd = line[3].find(')');

                if(rbStart == 0 && rbEnd == line[3].size() - 1){
                    rb = line[3].substr(rbStart+1, line[3].size() - 2);
                }
                else{
                    message = mesError + "Fix parentheses on line " + to_string(codeLine) + "!"  + mesEnd;
                    return "failed";
                }
            }
            else{
                message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }
        }
        else{
            if(line.size() != 4){
                message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }
            rb = line[2];
            c2 = line[3];
        }

        uint32_t raVal, rbVal, c2Val;

        try{
            raVal = regMap.at(ra);
            rbVal = regMap.at(rb);
        }catch(exception OOF){
            message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
        }

        if(!checkStringNumberic(c2)){
              message = mesError + "Non-numeric constant value on line " + to_string(codeLine) + "!" + mesEnd;
              return "failed";
        }

        c2Val = toVal(c2);

        if(!checkConstant(c2Val, C2)){
              message = mesError + "Invalid c2 constant on line " + to_string(codeLine) + "!" + mesEnd;
              return "failed";
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
        if(line.size() != 3){
            message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
            return "failed";
        }

        ra = line[1];
        c1 = line[2];

        uint32_t raVal, c1Val;

        try{
            raVal = regMap.at(ra);
        }catch(exception OOF){
            message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
        }

        if(!checkStringNumberic(c1)){
              message = mesError + "Non-numeric constant value on line " + to_string(codeLine) + "!" + mesEnd;
              return "failed";
        }

        c1Val = toVal(c1) - lineNum;

        if(!checkConstant(c1Val, C1)){
            message = mesError + "Invalid c1 constant on line " + to_string(codeLine) + "!"
             + mesEnd;
            return "failed";
        }


        binaryLine <<= 5;
        binaryLine |= (raVal & REG_MASK);

        binaryLine <<=22;
        binaryLine |= (c1Val & C1_MASK);
     }

     //neg, not
     else if(in.group == NEG){
        if(line.size() != 3){
            message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
            return "failed";
        }

        ra = line[1];
        rc = line[2];

        uint32_t raVal, rcVal;

        try{
            raVal = regMap.at(ra);
            rcVal = regMap.at(rc);
        }catch(exception OOF){
            message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
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

        if(in.group == BRL) ra = line[1];
        else ra = "r0";

        if(instr == "brlnv" || instr == "brnv"){
            if(line.size() != 1 && instr == "brnv"){
                message =  mesError + "brnv has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }
            if(line.size() != 2 && instr == "brlnv"){
                message =  mesError + "brlnv has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }

            rc = rb = "r0";
        }
        else if(instr == "br"){
            if(line.size() == 4){
                rb = line[1];
                rc = line[2];
                c3 = line[3];
            }
            else if(line.size() == 2){
                rb = line[1];
                rc = "r0";
                con = 1;
            }
            else{
                message =  mesError + "br has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }
        }
        else if(instr == "brl"){
            if(line.size() == 5){
                rb = line[2];
                rc = line[3];
                c3 = line[4];
            }
            else if(line.size() == 3){
                rb = line[2];
                rc = "r0";
                con = 1;
            }
            else{
                message =  mesError + "brl has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                return "failed";
            }
        }
        else{
            if(in.group == BR){
                if(line.size() != 3){
                    message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                    return "failed";
                }

                rb = line[1];
                rc = line[2];
            }
            else{
                if(line.size() != 4){
                    message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
                    return "failed";
                }

                rb = line[2];
                rc = line[3];
            }

            if(instr == "brzr" || instr == "brlzr")      con = 2;
            else if(instr == "brnz" || instr == "brlnz") con = 3;
            else if(instr == "brpl" || instr == "brlpl") con = 4;
            else con = 5;     //br(l)mi
        }


        uint32_t raVal, rbVal, rcVal, c3Val;

        try{
            raVal = regMap.at(ra);
            rbVal = regMap.at(rb);
            rcVal = regMap.at(rc);
        }catch(exception OOF){
            message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
        }

        if(!checkStringNumberic(c3, true)){
              message = mesError + "Non-numeric constant value on line " + to_string(codeLine) + "!" + mesEnd;
              return "failed";
        }

        c3Val = toVal(c3);

        if(!checkConstant(c3Val, C3)){
            message = mesError + "Invalid c3 constant on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
        }

        binaryLine <<= 5;
        binaryLine |= (raVal & REG_MASK);

        binaryLine <<= 5;
        binaryLine |= (rbVal & REG_MASK);

        binaryLine <<= 5;
        binaryLine |= (rcVal & REG_MASK);

        binaryLine <<= 12;

        if(con == 0) binaryLine |= (c3Val & C3_MASK);
        else binaryLine |= (con & 7);
     }

     //add, sub, and, or
     else if(in.group == AASO){
        if(line.size() != 4){
            message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
            return "failed";
        }

        ra = line[1];
        rb = line[2];
        rc = line[3];

        uint32_t raVal, rbVal, rcVal;

        try{
            raVal = regMap.at(ra);
            rbVal = regMap.at(rb);
            rcVal = regMap.at(rc);
        }catch(exception OOF){
            message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
            return "failed";
        }

        binaryLine <<= 5;
        binaryLine |= (raVal & REG_MASK);

        binaryLine <<= 5;
        binaryLine |= (rbVal & REG_MASK);

        binaryLine <<= 5;
        binaryLine |= (rcVal & REG_MASK);

        binaryLine <<= 12;
     }

     //shr, shra, shl, shc
     else if(in.group == SHIFT){
        if(line.size() != 4){
            message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
            return "failed";
        }

        ra = line[1];
        rb = line[2];
        string lastArg = line[3];

        if(lastArg.find("r") == string::npos){

           if(!checkStringNumberic(lastArg, true)){
                 message = mesError + "Non-numeric shift count value on line " + to_string(codeLine) + "!" + mesEnd;
                 return "failed";
           }

           uint32_t raVal, rbVal, count = toVal(lastArg);

           if(count > 31){
                 message = mesError + "Shift count value too large on line " + to_string(codeLine) + "!" + mesEnd;
                 return "failed";
           }

           try{
               raVal = regMap.at(ra);
               rbVal = regMap.at(rb);
           }catch(exception OOF){
               message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
               return "failed";
           }

           binaryLine <<= 5;
           binaryLine |= (raVal & REG_MASK);

           binaryLine <<= 5;
           binaryLine |= (rbVal & REG_MASK);

           binaryLine <<= 17;
           binaryLine |= (count & REG_MASK);
        }
        else{
           uint32_t raVal, rbVal, rcVal;

           try{
               raVal = regMap.at(ra);
               rbVal = regMap.at(rb);
               rcVal = regMap.at(rc);
           }catch(exception OOF){
               message = mesError + "Invalid register on line " + to_string(codeLine) + "!" + mesEnd;
               return "failed";
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

     //nop, stop
     else{
        if(line.size() != 1){
            message =  mesError + instr + " has incorrect number of parameters on line " + to_string(codeLine) + mesEnd;
            return "failed";
        }
        binaryLine <<= 27;
     }


      stringstream hexStream;
      hexStream << setw(8) << setfill('0') << hex << binaryLine;

      //adding to machineCode output
      machineCode.insert(machineCode.size(), hexStream.str() + "\n");
      lineNum += 4;
      ++codeLine;
   }

   //getting rid of last \n at the end;
   machineCode = machineCode.substr(0, machineCode.size()-1);

   return machineCode;
}

void Assembler::replaceAll(string &in, string tar, string rep) {
    size_t pos;
    while ((pos = in.find(tar)) != string::npos) {
        in.replace(pos, tar.length(), rep);
    }
}

void Assembler::stringSplit(string & strLine, vector<string> & vLine){
    istringstream iss(strLine);
    string segment;
    while(iss >> segment) vLine.push_back(segment);
}

bool Assembler::checkStringNumberic(string str, bool isUnsigned){
   bool isValid = true;
   bool hitHex = false;

   for(size_t i=0; i < str.size(); ++i){
      if(str.at(i) == '-' && !isUnsigned) continue;

      bool hexFound = false;
      if(i > 0) hexFound = (str.at(i) == 'x') && (str.at(i-1) == '0');

      isValid &= isdigit(str.at(i)) || hexFound;

      if(hexFound && !hitHex) hitHex = true;
      else if(hexFound && hitHex) isValid = false;

      if(!isValid) break;
   }

   return isValid;
}

bool Assembler::checkConstant(size_t constVal, CONSTANT_TYPE type){
    bool isValid = false;
    switch(type){
        case C1:
            isValid = constVal < (1 << 22);
            break;
        case C2:
            isValid = constVal < (1 << 17);
            break;
        case C3:
            isValid = constVal < (1 << 12);
            break;
    }
    return isValid;
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

uint32_t Assembler::toVal(string str){
    return (str.find("0x") == string::npos) ? stoul(str) : stoul(str, nullptr, 16);
}

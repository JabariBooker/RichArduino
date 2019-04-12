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

string Assembler::assemble(string code){
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
   if(lines[0].find(".org") == string::npos){
	  qStdOut() << "code doesn't start with \".org\"!" << endl;
      return NULL;
   }

   istringstream ss(line[0]);
   ss >> org >> codeStart;

   //first pass: looking for label positioning
   size_t instrCounter = 0;
   for(int i = 0; i < lines.size(); ++i){
      string line = lines[i];

	  //Comment detection
	  size_t commentStart = line.find("//");
	  if (commentStart != string::npos) {
		  line.erase(commentStart);
	  }

      size_t start = line.find_first_not_of(" \t");
      if(line == "" || start == string::npos) continue;

	  if (line.find(".org") != string::npos){
		  istringstream iss(line);
		  iss >> org >> instrCounter;
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

         labels[label] = instrCounter;
         start = labelEnd + 1;
      }

      //getting rid of leading spaces and labels to make second pass easier
      lines[i] = line.substr(start);

      instrCounter += 4;
   }

   //second pass: assembling code
   string machineCode = "";
   size_t lineNum = codeStart;
      
   for(string line : lines){
      
      //if the line is empty ignore it
	  if (line == "" || line.find_first_not_of(" \t") == string::npos) continue;

	  //Comment detection
	  size_t commentStart = line.find("//");
	  if (commentStart != string::npos) {
		  line.erase(commentStart);
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
	  if(label != "") line.replace(labelPos, labelLen, to_string(value));

      //getting rid of commas
	  replaceAll(line, ",", " ");

	  istringstream iss(line);
	  string instr;
	  size_t ra, rb, rc, c1, c2, c3;

	  /**Support for RSRC Assembly Language Conventions**/
	  /*
	  if (line.find(".org") != string::npos) {	//(not supported)
			  continue;
	  }
	  else if (line.find(".equ") != string::npos) {	//already handled, ignore
		  continue;
	  }
	  else if (line.find(".dc") != string::npos) {		//allocate and set 32-bit values
		  uint32_t value;
		  while (iss >> value) {
			  stringstream hexStream;
			  hexStream << setw(8) << setfill('0') << hex << value;
			  machineCode.insert(machineCode.size(), hexStream.str());
			  lineNum += 4;
		  }
		  continue;
	  }
	  else if (line.find(".dcb") != string::npos) {	//allocate and set 8 bit values
		  uint32_t word = 0;
		  uint8_t value, count = 0;
		  while (iss >> value) {
			  word <<= 8;;
			  word |= value & 0xff;
			  ++count;
			  if (count % 4 == 0) {
				  stringstream hexStream;
				  hexStream << setw(8) << setfill('0') << hex << word;
				  machineCode.insert(machineCode.size(), hexStream.str());
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
		  continue;
	  }
	  else if (line.find(".dch") != string::npos) {	//allocate and set 16 bit values
		  uint32_t word = 0;
		  uint16_t value, count = 0;
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
		  continue;
	  }
	  else if (line.find(".db") != string::npos) {	//allocate bytes (not supported)
		  continue;
	  }
	  else if (line.find(".dh") != string::npos) {	//allocate 16-bit halfwords (not supported)
		  continue;
	  }
	  else if (line.find(".dw") != string::npos) {	//allocate 32-bit words

	  }
	  */

      //getting instruction opcode and putting into line
      iss >> instr;
      bitset<5> opcode(instructions[instr].opcode);
      string binaryLine = opcode.to_string();


      /**determining instruction format**/


      //checking if instr is a shift instruction
      if(instructions[instr].group != SHIFT){
         
         //can remove r's from register names to get register numbers
		 replaceAll(line, "r", "");

         //greating a new stringstring for newly modified line
         istringstream iss_m(line);
         string toss;
         iss_m >> toss;       //prevent duplicate fetch of instruction

         //ld, st, la, addi, andi, ori
         if(instructions[instr].group == MEM_IMMED){
            iss_m >> ra;

            if(instr == "ld" || instr == "st" || instr == "la"){
				if (line.find('(') != string::npos) {
					string next;
					iss_m >> next;

					int rbStart = next.find('(');
					int rbEnd = next.find(')');

					rb = stoi(next.substr(rbStart + 1, rbEnd - rbStart + 1));

					if (rbStart == 0) c2 = 0;
					else c2 = stoi(next.substr(0, rbStart));	
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

            bitset<5> raField(ra), rbField(rb);
            bitset<17> c2Field(c2);

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), rbField.to_string());
            binaryLine.insert(binaryLine.size(), c2Field.to_string());
         }

         //ldr, str, lar
         else if(instructions[instr].group == MEM_REL){
            iss_m >> ra;
            iss_m >> c1;

            c1 = c1 - lineNum;

            bitset<5> raField(ra);
            bitset<22> c1Field(c1);

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), c1Field.to_string());
         }
         
         //neg, not
         else if(instructions[instr].group == NEG){
            iss_m >> ra;
            iss_m >> rc;

            //rbField unused
            bitset<5> raField(ra), rbField(0), rcField(rc);

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), rbField.to_string());
            binaryLine.insert(binaryLine.size(), rcField.to_string());
            binaryLine.insert(binaryLine.size(), "000000000000");
         }
         
         //br, brl
         else if(instructions[instr].sec_group == BR_ALL){
            int con = -1;

            if(instructions[instr].group == BRL) iss_m >> ra;
            else ra = 0;

            if(instr == "brnv" || instr == "brlnv"){
               rb = rc = c3 = con = 0;
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
                  c3 = 0;

                  if(instr == "brzr" || instr == "brlzr")      con = 2;
                  else if(instr == "brnz" || instr == "brlnz") con = 3;
                  else if(instr == "brpl" || instr == "brlpl") con = 4;
                  else con = 5;     //br(l)mi
               }
            }

            bitset<5> raField(ra), rbField(rb), rcField(rc);
            bitset<12> c3Field(c3);
            bitset<3> conbits(con);

            if(con == -1){
               binaryLine.insert(binaryLine.size(), raField.to_string());
               binaryLine.insert(binaryLine.size(), rbField.to_string());
               binaryLine.insert(binaryLine.size(), rcField.to_string());
               binaryLine.insert(binaryLine.size(), c3Field.to_string());
            }
            else{
               binaryLine.insert(binaryLine.size(), raField.to_string());
               binaryLine.insert(binaryLine.size(), rbField.to_string());
               binaryLine.insert(binaryLine.size(), rcField.to_string());
               binaryLine.insert(binaryLine.size(), "000000000");
               binaryLine.insert(binaryLine.size(), conbits.to_string());
            }
         }

         //add, sub, and, or
         else if(instructions[instr].group == AASO){
            iss_m >> ra >> rb >> rc;

            bitset<5> raField(ra), rbField(rb), rcField(rc);

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), rbField.to_string());
            binaryLine.insert(binaryLine.size(), rcField.to_string());
            binaryLine.insert(binaryLine.size(), "000000000000");
         }
         
         //nop, stop
         else{
            binaryLine.insert(binaryLine.size(), "000000000000000000000000000");
         }
      }

      //shr, shra, shl, shc
      else {
         string regA, regB, lastArg;

         iss >> regA >> regB >> lastArg;
         regA.replace(0, 1, "");
         regB.replace(0, 1, "");

         if(lastArg.find("r") == string::npos){
            bitset<5> raField(stoi(regA)), rbField(stoi(regB)), count(stoi(lastArg));

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), rbField.to_string());
            binaryLine.insert(binaryLine.size(), "000000000000");
            binaryLine.insert(binaryLine.size(), count.to_string());
         }
         else{
            lastArg.replace(0, 1, "");

            bitset<5> raField(stoi(regA)), rbField(stoi(regB)), rcField(stoi(lastArg));

            binaryLine.insert(binaryLine.size(), raField.to_string());
            binaryLine.insert(binaryLine.size(), rbField.to_string());
            binaryLine.insert(binaryLine.size(), rcField.to_string());
            binaryLine.insert(binaryLine.size(), "000000000000");
         }
      }
      
      unsigned long lineVal = stoul(binaryLine, nullptr, 2);
      stringstream hexStream;
      hexStream << setw(8) << setfill('0') << hex << lineVal;

      //adding to machineCode output
      machineCode.insert(machineCode.size(), hexStream.str() + "\n");
      lineNum += 4;
   }

   return machineCode;
}

void Assembler::replaceAll(string &in, string tar, string rep) {
	size_t pos;
	while ((pos = in.find(tar)) != string::npos) {
		in.replace(pos, tar.length(), rep);
	}
}

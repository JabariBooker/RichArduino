#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <bitset>
#include <algorithm>
#include <cstdint>
#include <QTextStream>

using namespace std;

enum InstructionGroup{
   MEM_IMMED, MEM_REL, NEG, BR, BRL, BR_ALL, AASO, SHIFT, NO_OP, NONE
};

struct Instruction{
   unsigned char opcode;
   InstructionGroup group, sec_group;
   Instruction() {
	  opcode = 0;
      group = sec_group = NONE;
   }
   Instruction(unsigned char op, InstructionGroup grp, InstructionGroup sec_grp = NONE){
      opcode = op;
      group = grp;
      sec_group = sec_grp;
   }
};

class Assembler{
   public:
      Assembler();
      string assemble(string code);
   
   private:
      map<string, Instruction> instructions;
	  void replaceAll(string &in, string tar, string rep);

	  /*
	  void checkRegister(size_t regVal);
	  void checkInstruction(string instr);
	  void checkConstant(size_t constVal, CONSTANT_TYPE type);
	  */
	  enum CONTANT_TYPE {
		  C1, C2, C3
	  };


	  QTextStream& qStdOut()
	  {
		  static QTextStream ts(stdout);
		  return ts;
	  }

};

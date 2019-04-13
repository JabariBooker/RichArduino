#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <QTextStream>

using namespace std;

enum InstructionGroup{
   MEM_IMMED, MEM_REL, NEG, BR, BRL, BR_ALL, AASO, SHIFT, NO_OP, NONE
};

struct Instruction{
   uint8_t opcode;
   InstructionGroup group, sec_group;
   Instruction() {
	  opcode = 0;
      group = sec_group = NONE;
   }
   Instruction(uint8_t op, InstructionGroup grp, InstructionGroup sec_grp = NONE){
      opcode = op;
      group = grp;
      sec_group = sec_grp;
   }
};

class Assembler{
   public:
      Assembler();
      string assemble(string & code, string & message);
   
   private:
      const uint32_t REG_MASK = 0x1F,
                     C1_MASK = 0x3FFFFF,
                     C2_MASK = 0x1FFFF,
                     C3_MASK = 0xFFF;

      enum CONSTANT_TYPE {
          C1, C2, C3
      };

      map<string, Instruction> instructions;
	  void replaceAll(string &in, string tar, string rep);

      bool checkRegister(uint32_t reg1, uint32_t reg2 = 0,  uint32_t reg3 = 0);
      bool checkConstant(size_t constVal, CONSTANT_TYPE type);



};

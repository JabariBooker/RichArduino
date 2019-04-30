#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <cstdint>
#include "MessageFormats.h"
#include <iostream>

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
      string assemble(string & code, string & message);
   
   private:
      const uint32_t REG_MASK = 0x1F,
                     C1_MASK = 0x3FFFFF,
                     C2_MASK = 0x1FFFF,
                     C3_MASK = 0xFFF;

      enum CONSTANT_TYPE {
          C1, C2, C3
      };

      const map<string, Instruction> instructions = {
         {"nop"  , Instruction(0, NO_OP)       },
         {"ld"   , Instruction(1, MEM_IMMED)   },
         {"ldr"  , Instruction(2, MEM_REL)     },
         {"st"   , Instruction(3, MEM_IMMED)   },
         {"str"  , Instruction(4, MEM_REL)     },
         {"la"   , Instruction(5, MEM_IMMED)   },
         {"lar"  , Instruction(6, MEM_REL)     },
         {"br"   , Instruction(8, BR, BR_ALL)  },
         {"brnv" , Instruction(8, BR, BR_ALL)  },
         {"brzr" , Instruction(8, BR, BR_ALL)  },
         {"brnz" , Instruction(8, BR, BR_ALL)  },
         {"brpl" , Instruction(8, BR, BR_ALL)  },
         {"brmi" , Instruction(8, BR, BR_ALL)  },
         {"brl"  , Instruction(9, BRL, BR_ALL) },
         {"brlzr", Instruction(9, BRL, BR_ALL) },
         {"brlnv", Instruction(9, BRL, BR_ALL) },
         {"brlnz", Instruction(9, BRL, BR_ALL) },
         {"brlpl", Instruction(9, BRL, BR_ALL) },
         {"brlmi", Instruction(9, BRL, BR_ALL) },
         {"add"  , Instruction(12, AASO)       },
         {"addi" , Instruction(13, MEM_IMMED)  },
         {"sub"  , Instruction(14, AASO)       },
         {"neg"  , Instruction(15, NEG)        },
         {"and"  , Instruction(20, AASO)       },
         {"andi" , Instruction(21, MEM_IMMED)  },
         {"or"   , Instruction(22, AASO)       },
         {"ori"  , Instruction(23, MEM_IMMED)  },
         {"not"  , Instruction(24, NEG)        },
         {"shr"  , Instruction(26, SHIFT)      },
         {"shra" , Instruction(27, SHIFT)      },
         {"shl"  , Instruction(28, SHIFT)      },
         {"shc"  , Instruction(29, SHIFT)      },
         {"stop" , Instruction(31, NO_OP)      }         
      };

      const map<string, size_t> regMap = {
         {"r0" , 0 },
         {"r1" , 1 },
         {"r2" , 2 },
         {"r3" , 3 },
         {"r4" , 4 },
         {"r5" , 5 },
         {"r6" , 6 },
         {"r7" , 7 },
         {"r8" , 8 },
         {"r9" , 9 },
         {"r10", 10},
         {"r11", 11},
         {"r12", 12},
         {"r13", 13},
         {"r14", 14},
         {"r15", 15},
         {"r16", 16},
         {"r17", 17},
         {"r18", 18},
         {"r19", 19},
         {"r20", 20},
         {"r21", 21},
         {"r22", 22},
         {"r23", 23},
         {"r24", 24},
         {"r25", 25},
         {"r26", 26},
         {"r27", 27},
         {"r28", 28},
         {"r29", 29},
         {"r30", 30},
         {"r31",  31}
      };


      //vector of all assembly instructions and pseudo ops
      vector<string> instrNames = {
         "nop"  ,
         "ld"   ,
         "ldr"  ,
         "st"   ,
         "str"  ,
         "la"   ,
         "lar"  ,
         "br"   ,
         "brnv" ,
         "brzr" ,
         "brnz" ,
         "brpl" ,
         "brmi" ,
         "brl"  ,
         "brlzr",
         "brlnv",
         "brlnz",
         "brlpl",
         "brlmi",
         "add"  ,
         "addi" ,
         "sub"  ,
         "neg"  ,
         "and"  ,
         "andi" ,
         "or"   ,
         "ori"  ,
         "not"  ,
         "shr"  ,
         "shra" ,
         "shl"  ,
         "shc"  ,
         "stop" ,
         ".org" ,
         ".equ" ,
         ".dc"  ,
         ".dcb" ,
         ".dch" ,
         ".dw"
      };

      vector<string> registers = {
         "r0 ",
         "r1 ",
         "r2 ",
         "r3 ",
         "r4 ",
         "r5 ",
         "r6 ",
         "r7 ",
         "r8 ",
         "r9 ",
         "r10",
         "r11",
         "r12",
         "r13",
         "r14",
         "r15",
         "r16",
         "r17",
         "r18",
         "r19",
         "r20",
         "r21",
         "r22",
         "r23",
         "r24",
         "r25",
         "r26",
         "r27",
         "r28",
         "r29",
         "r30",
         "r31"
      };

      void replaceAll(string &in, string tar, string rep);
      void stringSplit(string & strLine, vector<string> & vLine);
      bool checkStringNumberic(string str, bool isUnsigned = false);
      bool checkConstant(size_t constVal, CONSTANT_TYPE type);
      bool checkLabel(string label);
      uint32_t toVal(string str);

};

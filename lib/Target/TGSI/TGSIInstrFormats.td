//===- TGSIInstrFormats.td - TGSI Instruction Formats ----*- tablegen -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//

class tgsi_class<bits<8> instr, dag outs, dag ins,
      string asmstr, list<dag> pattern> : Instruction {
  let Namespace = "TGSI";
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  bits<1> vector = 0;
  bits<1> negate_op0 = 0;
  bits<1> negate_op1 = 0;
  bits<1> dst_resource = 0;
  bits<1> src_resource = 0;
  bits<3> addr_space = 0;

  let TSFlags{7-0} = instr;
  let TSFlags{8} = negate_op0;
  let TSFlags{9} = negate_op1;
  let TSFlags{10} = dst_resource;
  let TSFlags{11} = src_resource;
  let TSFlags{14-12} = addr_space;
}

class tgsi_negate_op0_class {
  bits<1> negate_op0 = 1;
}

class tgsi_negate_op1_class {
  bits<1> negate_op1 = 1;
}

class tgsi_dst_resource_class {
  bits<1> dst_resource = 1;
}

class tgsi_src_resource_class {
  bits<1> src_resource = 1;
}

class tgsi_addr_space_private_class {
  bits<3> addr_space = 0;
}

class tgsi_addr_space_global_class {
  bits<3> addr_space = 1;
}

class tgsi_addr_space_local_class {
  bits<3> addr_space = 2;
}

class tgsi_addr_space_constant_class {
  bits<3> addr_space = 3;
}

class tgsi_addr_space_input_class {
  bits<3> addr_space = 4;
}

// $Id$ -*- C++ -*-
//***************************************************************************
// File:
//	InstrScheduling.h
// 
// Purpose:
//	
// History:
//	7/23/01	 -  Vikram Adve  -  Created
//***************************************************************************

#ifndef LLVM_CODEGEN_INSTR_SCHEDULING_H
#define LLVM_CODEGEN_INSTR_SCHEDULING_H

#include "llvm/Support/CommandLine.h"
#include "llvm/CodeGen/MachineInstr.h"

class Method;
class SchedulingManager;
class TargetMachine;
class MachineSchedInfo;

// Debug option levels for instruction scheduling
enum SchedDebugLevel_t {
  Sched_NoDebugInfo,
  Sched_PrintMachineCode, 
  Sched_PrintSchedTrace,
  Sched_PrintSchedGraphs,
};

extern cl::Enum<SchedDebugLevel_t> SchedDebugLevel;



//---------------------------------------------------------------------------
// Function: ScheduleInstructionsWithSSA
// 
// Purpose:
//   Entry point for instruction scheduling on SSA form.
//   Schedules the machine instructions generated by instruction selection.
//   Assumes that register allocation has not been done, i.e., operands
//   are still in SSA form.
//---------------------------------------------------------------------------

bool ScheduleInstructionsWithSSA(Method* method, const TargetMachine &Target);


//---------------------------------------------------------------------------
// Function: ScheduleInstructions
// 
// Purpose:
//   Entry point for instruction scheduling on machine code.
//   Schedules the machine instructions generated by instruction selection.
//   Assumes that register allocation has been done.
//---------------------------------------------------------------------------

// Not implemented yet.
bool		ScheduleInstructions		(Method* method,
						 const TargetMachine &Target);

//---------------------------------------------------------------------------
// Function: instrIsFeasible
// 
// Purpose:
//   Used by the priority analysis to filter out instructions
//   that are not feasible to issue in the current cycle.
//   Should only be used during schedule construction..
//---------------------------------------------------------------------------

bool		instrIsFeasible			(const SchedulingManager& S,
						 MachineOpCode opCode);
#endif

//===-- DWARFDebugLine.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_DWARFDEBUGLINE_H
#define LLVM_DEBUGINFO_DWARFDEBUGLINE_H

#include "llvm/Support/DataExtractor.h"
#include <map>
#include <string>
#include <vector>

namespace llvm {

class raw_ostream;

class DWARFDebugLine {
public:
  struct FileNameEntry {
    FileNameEntry() : DirIdx(0), ModTime(0), Length(0) {}

    std::string Name;
    uint64_t DirIdx;
    uint64_t ModTime;
    uint64_t Length;
  };

  struct Prologue {
    Prologue()
      : TotalLength(0), Version(0), PrologueLength(0), MinInstLength(0),
        DefaultIsStmt(0), LineBase(0), LineRange(0), OpcodeBase(0) {}

    // The size in bytes of the statement information for this compilation unit
    // (not including the total_length field itself).
    uint32_t TotalLength;
    // Version identifier for the statement information format.
    uint16_t Version;
    // The number of bytes following the prologue_length field to the beginning
    // of the first byte of the statement program itself.
    uint32_t PrologueLength;
    // The size in bytes of the smallest target machine instruction. Statement
    // program opcodes that alter the address register first multiply their
    // operands by this value.
    uint8_t MinInstLength;
    // The initial value of theis_stmtregister.
    uint8_t DefaultIsStmt;
    // This parameter affects the meaning of the special opcodes. See below.
    int8_t LineBase;
    // This parameter affects the meaning of the special opcodes. See below.
    uint8_t LineRange;
    // The number assigned to the first special opcode.
    uint8_t OpcodeBase;
    std::vector<uint8_t> StandardOpcodeLengths;
    std::vector<std::string> IncludeDirectories;
    std::vector<FileNameEntry> FileNames;

    // Length of the prologue in bytes.
    uint32_t getLength() const {
      return PrologueLength + sizeof(TotalLength) + sizeof(Version) +
             sizeof(PrologueLength);
    }
    // Length of the line table data in bytes (not including the prologue).
    uint32_t getStatementTableLength() const {
      return TotalLength + sizeof(TotalLength) - getLength();
    }
    int32_t getMaxLineIncrementForSpecialOpcode() const {
      return LineBase + (int8_t)LineRange - 1;
    }
    void dump(raw_ostream &OS) const;
    void clear() {
      TotalLength = Version = PrologueLength = 0;
      MinInstLength = LineBase = LineRange = OpcodeBase = 0;
      StandardOpcodeLengths.clear();
      IncludeDirectories.clear();
      FileNames.clear();
    }
  };

  // Standard .debug_line state machine structure.
  struct Row {
    Row(bool default_is_stmt = false) { reset(default_is_stmt); }
    /// Called after a row is appended to the matrix.
    void postAppend();
    void reset(bool default_is_stmt);
    void dump(raw_ostream &OS) const;

    // The program-counter value corresponding to a machine instruction
    // generated by the compiler.
    uint64_t Address;
    // An unsigned integer indicating a source line number. Lines are numbered
    // beginning at 1. The compiler may emit the value 0 in cases where an
    // instruction cannot be attributed to any source line.
    uint32_t Line;
    // An unsigned integer indicating a column number within a source line.
    // Columns are numbered beginning at 1. The value 0 is reserved to indicate
    // that a statement begins at the 'left edge' of the line.
    uint16_t Column;
    // An unsigned integer indicating the identity of the source file
    // corresponding to a machine instruction.
    uint16_t File;
    // An unsigned integer whose value encodes the applicable instruction set
    // architecture for the current instruction.
    uint8_t Isa;
    // A boolean indicating that the current instruction is the beginning of a
    // statement.
    uint8_t IsStmt:1,
            // A boolean indicating that the current instruction is the
            // beginning of a basic block.
            BasicBlock:1,
            // A boolean indicating that the current address is that of the
            // first byte after the end of a sequence of target machine
            // instructions.
            EndSequence:1,
            // A boolean indicating that the current address is one (of possibly
            // many) where execution should be suspended for an entry breakpoint
            // of a function.
            PrologueEnd:1,
            // A boolean indicating that the current address is one (of possibly
            // many) where execution should be suspended for an exit breakpoint
            // of a function.
            EpilogueBegin:1;
  };

  struct LineTable {
    void appendRow(const DWARFDebugLine::Row &state) { Rows.push_back(state); }
    void clear() {
      Prologue.clear();
      Rows.clear();
    }

    uint32_t lookupAddress(uint64_t address, uint64_t cu_high_pc) const;
    void dump(raw_ostream &OS) const;

    struct Prologue Prologue;
    std::vector<Row> Rows;
  };

  struct State : public Row, public LineTable {
    // Special row codes.
    enum {
      StartParsingLineTable = 0,
      DoneParsingLineTable = -1
    };

    State() : row(0) {}

    virtual void appendRowToMatrix(uint32_t offset);
    virtual void finalize(uint32_t offset) { row = DoneParsingLineTable; }
    virtual void reset() { Row::reset(Prologue.DefaultIsStmt); }

    // The row number that starts at zero for the prologue, and increases for
    // each row added to the matrix.
    unsigned row;
  };

  struct DumpingState : public State {
    DumpingState(raw_ostream &OS) : OS(OS) {}
    virtual void finalize(uint32_t offset);
  private:
    raw_ostream &OS;
  };

  static bool parsePrologue(DataExtractor debug_line_data, uint32_t *offset_ptr,
                            Prologue *prologue);
  /// Parse a single line table (prologue and all rows).
  static bool parseStatementTable(DataExtractor debug_line_data,
                                  uint32_t *offset_ptr, State &state);

  /// Parse all information in the debug_line_data into an internal
  /// representation.
  void parse(DataExtractor debug_line_data);
  void parseIfNeeded(DataExtractor debug_line_data) {
    if (LineTableMap.empty())
      parse(debug_line_data);
  }
  static void dump(DataExtractor debug_line_data, raw_ostream &OS);
  const LineTable *getLineTable(uint32_t offset) const;

private:
  typedef std::map<uint32_t, LineTable> LineTableMapTy;
  typedef LineTableMapTy::iterator LineTableIter;
  typedef LineTableMapTy::const_iterator LineTableConstIter;

  LineTableMapTy LineTableMap;
};

}

#endif

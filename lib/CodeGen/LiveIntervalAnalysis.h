//===-- llvm/CodeGen/LiveInterval.h - Live Interval Analysis ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LiveInterval analysis pass.  Given some
// numbering of each the machine instructions (in this implemention
// depth-first order) an interval [i, j] is said to be a live interval
// for register v if there is no instruction with number j' > j such
// that v is live at j' abd there is no instruction with number i' < i
// such that v is live at i'. In this implementation intervals can
// have holes, i.e. an interval might look like [1,20], [50,65],
// [1000,1001]
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_LIVEINTERVALS_H
#define LLVM_CODEGEN_LIVEINTERVALS_H

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include <iostream>
#include <map>

namespace llvm {

    class LiveVariables;
    class MRegisterInfo;

    class LiveIntervals : public MachineFunctionPass
    {
    public:
        struct Interval {
            typedef std::pair<unsigned, unsigned> Range;
            typedef std::vector<Range> Ranges;
            unsigned reg;   // the register of this interval
            unsigned weight; // weight of this interval (number of uses)
            Ranges ranges; // the ranges this register is valid

            Interval(unsigned r);

            unsigned start() const {
                assert(!ranges.empty() && "empty interval for register");
                return ranges.front().first;
            }

            unsigned end() const {
                assert(!ranges.empty() && "empty interval for register");
                return ranges.back().second;
            }

            bool expiredAt(unsigned index) const {
                return end() <= index;
            }

            bool liveAt(unsigned index) const;

            bool overlaps(const Interval& other) const;

            void addRange(unsigned start, unsigned end);

        private:
            void mergeRangesForward(Ranges::iterator it);

            void mergeRangesBackward(Ranges::iterator it);
        };

        struct StartPointComp {
            bool operator()(const Interval& lhs, const Interval& rhs) {
                return lhs.ranges.front().first < rhs.ranges.front().first;
            }
        };

        struct EndPointComp {
            bool operator()(const Interval& lhs, const Interval& rhs) {
                return lhs.ranges.back().second < rhs.ranges.back().second;
            }
        };

        typedef std::vector<Interval> Intervals;
        typedef std::vector<MachineBasicBlock*> MachineBasicBlockPtrs;

    private:
        MachineFunction* mf_;
        const TargetMachine* tm_;
        const MRegisterInfo* mri_;
        MachineBasicBlock* currentMbb_;
        MachineBasicBlock::iterator currentInstr_;
        LiveVariables* lv_;

        std::vector<bool> allocatableRegisters_;

        typedef std::map<unsigned, MachineBasicBlock*> MbbIndex2MbbMap;
        MbbIndex2MbbMap mbbi2mbbMap_;

        typedef std::map<MachineInstr*, unsigned> Mi2IndexMap;
        Mi2IndexMap mi2iMap_;

        typedef std::map<unsigned, unsigned> Reg2IntervalMap;
        Reg2IntervalMap r2iMap_;

        Intervals intervals_;

    public:
        virtual void getAnalysisUsage(AnalysisUsage &AU) const;
        Intervals& getIntervals() { return intervals_; }
        MachineBasicBlockPtrs getOrderedMachineBasicBlockPtrs() const {
            MachineBasicBlockPtrs result;
            for (MbbIndex2MbbMap::const_iterator
                     it = mbbi2mbbMap_.begin(), itEnd = mbbi2mbbMap_.end();
                 it != itEnd; ++it) {
                result.push_back(it->second);
            }
            return result;
        }

    private:
        /// runOnMachineFunction - pass entry point
        bool runOnMachineFunction(MachineFunction&);

        /// computeIntervals - compute live intervals
        void computeIntervals();


        /// handleRegisterDef - update intervals for a register def
        /// (calls handlePhysicalRegisterDef and
        /// handleVirtualRegisterDef)
        void handleRegisterDef(MachineBasicBlock* mbb,
                               MachineBasicBlock::iterator mi,
                               unsigned reg);

        /// handleVirtualRegisterDef - update intervals for a virtual
        /// register def
        void handleVirtualRegisterDef(MachineBasicBlock* mbb,
                                      MachineBasicBlock::iterator mi,
                                      unsigned reg);

        /// handlePhysicalRegisterDef - update intervals for a
        /// physical register def
        void handlePhysicalRegisterDef(MachineBasicBlock* mbb,
                                       MachineBasicBlock::iterator mi,
                                       unsigned reg);

        unsigned getInstructionIndex(MachineInstr* instr) const;

        void printRegName(unsigned reg) const;
    };

    inline bool operator==(const LiveIntervals::Interval& lhs,
                           const LiveIntervals::Interval& rhs) {
        return lhs.reg == rhs.reg;
    }

    std::ostream& operator<<(std::ostream& os,
                             const LiveIntervals::Interval& li);

} // End llvm namespace

#endif

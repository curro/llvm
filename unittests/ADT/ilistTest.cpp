//===- llvm/unittest/ADT/APInt.cpp - APInt unit tests ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ilist.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ilist_node.h"
#include "gtest/gtest.h"
#include <ostream>

using namespace llvm;

namespace {

struct Node : ilist_node<Node> {
  int Value;

  Node() {}
  Node(int _Value) : Value(_Value) {}
};

TEST(ilistTest, Basic) {
  ilist<Node> List;
  List.push_back(Node(1));
  EXPECT_EQ(1, List.back().Value);
  EXPECT_EQ(0, List.back().getPrevNode());
  EXPECT_EQ(0, List.back().getNextNode());

  List.push_back(Node(2));
  EXPECT_EQ(2, List.back().Value);
  EXPECT_EQ(2, List.front().getNextNode()->Value);
  EXPECT_EQ(1, List.back().getPrevNode()->Value);

  const ilist<Node> &ConstList = List;
  EXPECT_EQ(2, ConstList.back().Value);
  EXPECT_EQ(2, ConstList.front().getNextNode()->Value);
  EXPECT_EQ(1, ConstList.back().getPrevNode()->Value);
}

TEST(ilistTest, SpliceOne) {
  ilist<Node> List;
  List.push_back(1);

  // The single-element splice operation supports noops.
  List.splice(List.begin(), List, List.begin());
  EXPECT_EQ(1u, List.size());
  EXPECT_EQ(1, List.front().Value);
  EXPECT_TRUE(llvm::next(List.begin()) == List.end());

  // Altenative noop. Move the first element behind itself.
  List.push_back(2);
  List.push_back(3);
  List.splice(llvm::next(List.begin()), List, List.begin());
  EXPECT_EQ(3u, List.size());
  EXPECT_EQ(1, List.front().Value);
  EXPECT_EQ(2, llvm::next(List.begin())->Value);
  EXPECT_EQ(3, List.back().Value);
}

}

//===- TableGenBackend.cpp - Base class for TableGen Backends ---*- C++ -*-===//
//
// This file provides useful services for TableGen backends...
//
//===----------------------------------------------------------------------===//

#include "TableGenBackend.h"
#include "Record.h"
#include <iostream>

void TableGenBackend::EmitSourceFileHeader(const std::string &Desc,
                                           std::ostream &OS) const {
  OS << "//===- TableGen'erated file -------------------------------------*-"
       " C++ -*-===//\n//\n// " << Desc << "\n//\n// Automatically generate"
       "d file, do not edit!\n//\n//===------------------------------------"
       "----------------------------------===//\n\n";
}

/// getQualifiedName - Return the name of the specified record, with a
/// namespace qualifier if the record contains one.
///
std::string TableGenBackend::getQualifiedName(Record *R) const {
  std::string Namespace = R->getValueAsString("Namespace");
  if (Namespace.empty()) return R->getName();
  return Namespace + "::" + R->getName();
}


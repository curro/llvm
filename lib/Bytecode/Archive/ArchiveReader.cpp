//===-- ArchiveReader.cpp - Read LLVM archive files -------------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Reid Spencer and is distributed under the 
// University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// Builds up standard unix archive files (.a) containing LLVM bytecode.
//
//===----------------------------------------------------------------------===//

#include "ArchiveInternals.h"
#include "llvm/Bytecode/Reader.h"

using namespace llvm;

/// Read a variable-bit-rate encoded unsigned integer
inline unsigned readInteger(const char*&At, const char*End) {
  unsigned Shift = 0;
  unsigned Result = 0;
  
  do {
    if (At == End) 
      throw std::string("Ran out of data reading vbr_uint!");
    Result |= (unsigned)((*At++) & 0x7F) << Shift;
    Shift += 7;
  } while (At[-1] & 0x80);
  return Result;
}

// Completely parse the Archive's symbol table and populate symTab member var.
void
Archive::parseSymbolTable(const void* data, unsigned size) {
  const char* At = (const char*) data;
  const char* End = At + size;
  while (At < End) {
    unsigned offset = readInteger(At, End);
    unsigned length = readInteger(At, End);
    if (At + length > End)
      throw std::string("malformed symbol table");
    // we don't care if it can't be inserted (duplicate entry)
    symTab.insert(std::make_pair(std::string(At, length), offset));
    At += length;
  }
  symTabSize = size;
}

// This member parses an ArchiveMemberHeader that is presumed to be pointed to
// by At. The At pointer is updated to the byte just after the header, which
// can be variable in size. 
ArchiveMember*
Archive::parseMemberHeader(const char*& At, const char* End) {
  assert(At + sizeof(ArchiveMemberHeader) < End && "Not enough data");

  // Cast archive member header
  ArchiveMemberHeader* Hdr = (ArchiveMemberHeader*)At;
  At += sizeof(ArchiveMemberHeader);

  // Instantiate the ArchiveMember to be filled
  ArchiveMember* member = new ArchiveMember(this);

  // Extract the size and determine if the file is 
  // compressed or not (negative length).
  int flags = 0;
  int MemberSize = atoi(Hdr->size);
  if (MemberSize < 0) {
    flags |= ArchiveMember::CompressedFlag;
    MemberSize = -MemberSize;
  }

  // Check the size of the member for sanity
  if (At + MemberSize > End)
    throw std::string("invalid member length in archive file");

  // Check the member signature
  if (!Hdr->checkSignature())
    throw std::string("invalid file member signature");

  // Convert and check the member name
  // The empty name ( '/' and 15 blanks) is for a foreign (non-LLVM) symbol 
  // table. The special name "//" and 14 blanks is for a string table, used 
  // for long file names. This library doesn't generate either of those but
  // it will accept them. If the name starts with #1/ and the remainder is 
  // digits, then those digits specify the length of the name that is 
  // stored immediately following the header. The special name 
  // __LLVM_SYM_TAB__ identifies the symbol table for LLVM bytecode. 
  // Anything else is a regular, short filename that is terminated with 
  // a '/' and blanks.

  std::string pathname;
  unsigned index;
  switch (Hdr->name[0]) {
    case '#':
      if (Hdr->name[1] == '1' && Hdr->name[2] == '/') {
        if (isdigit(Hdr->name[3])) {
          unsigned len = atoi(&Hdr->name[3]);
          pathname.assign(At, len);
          At += len;
          MemberSize -= len;
          flags |= ArchiveMember::HasLongFilenameFlag;
        } else
          throw std::string("invalid long filename");
      } else if (Hdr->name[1] == '_' && 
                 (0 == memcmp(Hdr->name, ARFILE_LLVM_SYMTAB_NAME, 16))) {
        // The member is using a long file name (>15 chars) format.
        // This format is standard for 4.4BSD and Mac OSX operating
        // systems. LLVM uses it similarly. In this format, the
        // remainder of the name field (after #1/) specifies the
        // length of the file name which occupy the first bytes of
        // the member's data. The pathname already has the #1/ stripped.
        pathname.assign(ARFILE_LLVM_SYMTAB_NAME);
        flags |= ArchiveMember::LLVMSymbolTableFlag;
      }
      break;
    case '/':
      if (Hdr->name[1]== '/') {
        if (0 == memcmp(Hdr->name, ARFILE_STRTAB_NAME, 16)) {
          pathname.assign(ARFILE_STRTAB_NAME);
          flags |= ArchiveMember::StringTableFlag;
        } else {
          throw std::string("invalid string table name");
        }
      } else if (Hdr->name[1] == ' ') {
        if (0 == memcmp(Hdr->name, ARFILE_SVR4_SYMTAB_NAME, 16)) {
          pathname.assign(ARFILE_SVR4_SYMTAB_NAME);
          flags |= ArchiveMember::SVR4SymbolTableFlag;
        } else {
          throw std::string("invalid SVR4 symbol table name");
        }
      } else if (isdigit(Hdr->name[1])) {
        unsigned index = atoi(&Hdr->name[1]);
        if (index < strtab.length()) {
          const char* namep = strtab.c_str() + index;
          const char* endp = strtab.c_str() + strtab.length();
          const char* p = namep;
          const char* last_p = p;
          while (p < endp) {
            if (*p == '\n' && *last_p == '/') {
              pathname.assign(namep, last_p - namep);
              flags |= ArchiveMember::HasLongFilenameFlag;
              break;
            }
            last_p = p;
            p++;
          }
          if (p >= endp)
            throw std::string("missing name termiantor in string table");
        } else {
          throw std::string("name index beyond string table");
        }
      }
      break;
    case '_':
      if (Hdr->name[1] == '_' && 
          (0 == memcmp(Hdr->name, ARFILE_BSD4_SYMTAB_NAME, 16))) {
        pathname.assign(ARFILE_BSD4_SYMTAB_NAME);
        flags |= ArchiveMember::BSD4SymbolTableFlag;
        break;
      }
      /* FALL THROUGH */

    default:
      char* slash = (char*) memchr(Hdr->name, '/', 16);
      if (slash == 0)
        slash = Hdr->name + 16;
      pathname.assign(Hdr->name, slash - Hdr->name);
      break;
  }

  // Determine if this is a bytecode file
  switch (sys::IdentifyFileType(At, 4)) {
    case sys::BytecodeFileType:
      flags |= ArchiveMember::BytecodeFlag;
      break;
    case sys::CompressedBytecodeFileType:
      flags |= ArchiveMember::CompressedBytecodeFlag;
      flags &= ~ArchiveMember::CompressedFlag;
      break;
    default:
      flags &= ~(ArchiveMember::BytecodeFlag|
                 ArchiveMember::CompressedBytecodeFlag);
      break;
  }

  // Fill in fields of the ArchiveMember
  member->next = 0;
  member->prev = 0;
  member->parent = this;
  member->path.setFile(pathname);
  member->info.fileSize = MemberSize;
  member->info.modTime.fromEpochTime(atoi(Hdr->date));
  sscanf(Hdr->mode, "%o", &(member->info.mode));
  member->info.user = atoi(Hdr->uid);
  member->info.group = atoi(Hdr->gid);
  member->flags = flags;
  member->data = At;

  return member;
}

void
Archive::checkSignature() {
  // Check the magic string at file's header
  if (mapfile->size() < 8 || memcmp(base, ARFILE_MAGIC, 8))
    throw std::string("invalid signature for an archive file");
}

// This function loads the entire archive and fully populates its ilist with 
// the members of the archive file. This is typically used in preparation for
// editing the contents of the archive.
void
Archive::loadArchive() {

  // Set up parsing
  members.clear();
  symTab.clear();
  const char *At = base;
  const char *End = base + mapfile->size();

  checkSignature();
  At += 8;  // Skip the magic string.

  bool seenSymbolTable = false;
  bool foundFirstFile = false;
  while (At < End) {
    // parse the member header 
    const char* Save = At;
    ArchiveMember* mbr = parseMemberHeader(At, End);

    // check if this is the foreign symbol table
    if (mbr->isSVR4SymbolTable() || mbr->isBSD4SymbolTable()) {
      // We just save this but don't do anything special
      // with it. It doesn't count as the "first file".
      if (foreignST) {
        // What? Multiple foreign symbol tables? Just chuck it
        // and retain the last one found.
        delete foreignST;
      }
      foreignST = mbr;
      At += mbr->getSize();
      if ((intptr_t(At) & 1) == 1)
        At++;
    } else if (mbr->isStringTable()) {
      // Simply suck the entire string table into a string
      // variable. This will be used to get the names of the
      // members that use the "/ddd" format for their names
      // (SVR4 style long names).
      strtab.assign(At, mbr->getSize());
      At += mbr->getSize();
      if ((intptr_t(At) & 1) == 1)
        At++;
      delete mbr;
    } else if (mbr->isLLVMSymbolTable()) { 
      // This is the LLVM symbol table for the archive. If we've seen it
      // already, its an error. Otherwise, parse the symbol table and move on.
      if (seenSymbolTable)
        throw std::string("invalid archive: multiple symbol tables");
      parseSymbolTable(mbr->getData(), mbr->getSize());
      seenSymbolTable = true;
      At += mbr->getSize();
      if ((intptr_t(At) & 1) == 1)
        At++;
      delete mbr; // We don't need this member in the list of members.
    } else {
      // This is just a regular file. If its the first one, save its offset.
      // Otherwise just push it on the list and move on to the next file.
      if (!foundFirstFile) {
        firstFileOffset = Save - base;
        foundFirstFile = true;
      }
      members.push_back(mbr);
      At += mbr->getSize();
      if ((intptr_t(At) & 1) == 1)
        At++;
    }
  }
}

// Open and completely load the archive file.
Archive*
Archive::OpenAndLoad(const sys::Path& file) {

  Archive* result = new Archive(file, true);

  result->loadArchive();
  
  return result;
}

// Get all the bytecode modules from the archive
bool
Archive::getAllModules(std::vector<Module*>& Modules, std::string* ErrMessage) {

  for (iterator I=begin(), E=end(); I != E; ++I) {
    if (I->isBytecode() || I->isCompressedBytecode()) {
      std::string FullMemberName = archPath.get() + 
        "(" + I->getPath().get() + ")";
      Module* M = ParseBytecodeBuffer((const unsigned char*)I->getData(), 
          I->getSize(), FullMemberName, ErrMessage);
      if (!M)
        return true;

      Modules.push_back(M);
    }
  }
  return false;
}

// Load just the symbol table from the archive file
void
Archive::loadSymbolTable() {

  // Set up parsing
  members.clear();
  symTab.clear();
  const char *At = base;
  const char *End = base + mapfile->size();

  // Make sure we're dealing with an archive
  checkSignature();

  At += 8; // Skip signature

  // Parse the first file member header
  const char* FirstFile = At;
  ArchiveMember* mbr = parseMemberHeader(At, End);

  if (mbr->isSVR4SymbolTable() || mbr->isBSD4SymbolTable()) {
    // Skip the foreign symbol table, we don't do anything with it
    At += mbr->getSize();
    if ((intptr_t(At) & 1) == 1)
      At++;
    delete mbr;

    // Read the next one
    FirstFile = At;
    mbr = parseMemberHeader(At, End);
  }

  if (mbr->isStringTable()) {
    // Process the string table entry
    strtab.assign((const char*)mbr->getData(), mbr->getSize());
    At += mbr->getSize();
    if ((intptr_t(At) & 1) == 1)
      At++;
    delete mbr;
    // Get the next one
    FirstFile = At;
    mbr = parseMemberHeader(At, End);
  }

  // See if its the symbol table
  if (mbr->isLLVMSymbolTable()) {
    parseSymbolTable(mbr->getData(), mbr->getSize());
    At += mbr->getSize();
    if ((intptr_t(At) & 1) == 1)
      At++;
    FirstFile = At;
  } else {
    // There's no symbol table in the file. We have to rebuild it from scratch
    // because the intent of this method is to get the symbol table loaded so 
    // it can be searched efficiently. 
    // Add the member to the members list
    members.push_back(mbr);
  }

  firstFileOffset = FirstFile - base;
}

// Open the archive and load just the symbol tables
Archive*
Archive::OpenAndLoadSymbols(const sys::Path& file) {
  Archive* result = new Archive(file, true);

  result->loadSymbolTable();

  return result;
}

// Look up one symbol in the symbol table and return a ModuleProvider for the
// module that defines that symbol.
ModuleProvider* 
Archive::findModuleDefiningSymbol(const std::string& symbol) {
  SymTabType::iterator SI = symTab.find(symbol);
  if (SI == symTab.end())
    return 0;

  // The symbol table was previously constructed assuming that the members were 
  // written without the symbol table header. Because VBR encoding is used, the
  // values could not be adjusted to account for the offset of the symbol table
  // because that could affect the size of the symbol table due to VBR encoding.
  // We now have to account for this by adjusting the offset by the size of the 
  // symbol table and its header.
  unsigned fileOffset = 
    SI->second +                // offset in symbol-table-less file
    firstFileOffset;            // add offset to first "real" file in archive

  // See if the module is already loaded
  ModuleMap::iterator MI = modules.find(fileOffset);
  if (MI != modules.end())
    return MI->second.first;

  // Module hasn't been loaded yet, we need to load it
  const char* modptr = base + fileOffset;
  ArchiveMember* mbr = parseMemberHeader(modptr, base + mapfile->size());

  // Now, load the bytecode module to get the ModuleProvider
  std::string FullMemberName = archPath.get() + "(" + 
    mbr->getPath().get() + ")";
  ModuleProvider* mp = getBytecodeBufferModuleProvider(
      (const unsigned char*) mbr->getData(), mbr->getSize(), 
      FullMemberName, 0);

  modules.insert(std::make_pair(fileOffset, std::make_pair(mp, mbr)));

  return mp;
}

// Look up multiple symbols in the symbol table and return a set of 
// ModuleProviders that define those symbols.
void
Archive::findModulesDefiningSymbols(std::set<std::string>& symbols,
                                    std::set<ModuleProvider*>& result)
{
  assert(mapfile && base && "Can't findModulesDefiningSymbols on new archive");
  if (symTab.empty()) {
    // We don't have a symbol table, so we must build it now but lets also
    // make sure that we populate the modules table as we do this to ensure
    // that we don't load them twice when findModuleDefiningSymbol is called
    // below.

    // Get a pointer to the first file
    const char* At  = ((const char*)base) + firstFileOffset;
    const char* End = ((const char*)base) + mapfile->size();

    while ( At < End) {
      // Compute the offset to be put in the symbol table
      unsigned offset = At - base - firstFileOffset;

      // Parse the file's header
      ArchiveMember* mbr = parseMemberHeader(At, End);

      // If it contains symbols
      if (mbr->isBytecode() || mbr->isCompressedBytecode()) {
        // Get the symbols 
        std::vector<std::string> symbols;
        std::string FullMemberName = archPath.get() + "(" + 
          mbr->getPath().get() + ")";
        ModuleProvider* MP = GetBytecodeSymbols((const unsigned char*)At,
            mbr->getSize(), FullMemberName, symbols);

        if (MP) {
          // Insert the module's symbols into the symbol table
          for (std::vector<std::string>::iterator I = symbols.begin(), 
               E=symbols.end(); I != E; ++I ) {
            symTab.insert(std::make_pair(*I, offset));
          }
          // Insert the ModuleProvider and the ArchiveMember into the table of
          // modules.
          modules.insert(std::make_pair(offset, std::make_pair(MP, mbr)));
        } else {
          throw std::string("Can't parse bytecode member: ") +
            mbr->getPath().get();
        }
      }

      // Go to the next file location
      At += mbr->getSize();
      if ((intptr_t(At) & 1) == 1)
        At++;
    }
  }

  // At this point we have a valid symbol table (one way or another) so we 
  // just use it to quickly find the symbols requested.

  for (std::set<std::string>::iterator I=symbols.begin(), 
       E=symbols.end(); I != E;) {
    // See if this symbol exists
    ModuleProvider* mp = findModuleDefiningSymbol(*I);
    if (mp) {
      // The symbol exists, insert the ModuleProvider into our result,
      // duplicates wil be ignored
      result.insert(mp);

      // Remove the symbol now that its been resolved, being careful to 
      // post-increment the iterator.
      symbols.erase(I++);
    } else {
      ++I;
    }
  }
}

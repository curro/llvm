//===- llvm/System/Linux/Path.cpp - Linux Path Implementation ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Reid Spencer and is distributed under the
// University of Illinois Open Source License. See LICENSE.TXT for details.
//
// Modified by Henrik Bach to comply with at least MinGW.
// Ported to Win32 by Jeff Cohen.
//
//===----------------------------------------------------------------------===//
//
// This file provides the Win32 specific implementation of the Path class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic Win32 code that
//===          is guaranteed to work on *all* Win32 variants.
//===----------------------------------------------------------------------===//

#include "Win32.h"
#include <llvm/System/Path.h>
#include <fstream>
#include <malloc.h>

namespace llvm {
namespace sys {

bool
Path::is_valid() const {
  if (path.empty())
    return false;

  // On Unix, the realpath function is used, which only requires that the
  // directories leading up the to final file name are valid.  The file itself
  // need not exist.  To get this behavior on Windows, we must elide the file
  // name manually.
  Path dir(*this);
  dir.elide_file();
  if (dir.path.empty())
    return true;

  DWORD attr = GetFileAttributes(dir.path.c_str());
  return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static Path *TempDirectory = NULL;

Path
Path::GetTemporaryDirectory() {
  if (TempDirectory)
    return *TempDirectory;

  char pathname[MAX_PATH];
  if (!GetTempPath(MAX_PATH, pathname))
    ThrowError("Can't determine temporary directory");

  Path result;
  result.set_directory(pathname);

  // Append a subdirectory passed on our process id so multiple LLVMs don't
  // step on each other's toes.
  sprintf(pathname, "LLVM_%u", GetCurrentProcessId());
  result.append_directory(pathname);

  // If there's a directory left over from a previous LLVM execution that
  // happened to have the same process id, get rid of it.
  result.destroy_directory(true);

  // And finally (re-)create the empty directory.
  result.create_directory(false);
  TempDirectory = new Path(result);
  return *TempDirectory;
}

Path::Path(std::string unverified_path)
  : path(unverified_path)
{
  if (unverified_path.empty())
    return;
  if (this->is_valid())
    return;
  // oops, not valid.
  path.clear();
  ThrowError(unverified_path + ": path is not valid");
}

// FIXME: the following set of functions don't map to Windows very well.
Path
Path::GetRootDirectory() {
  Path result;
  result.set_directory("/");
  return result;
}

static inline bool IsLibrary(Path& path, const std::string& basename) {
  if (path.append_file(std::string("lib") + basename)) {
    if (path.append_suffix(Path::GetDLLSuffix()) && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("a") && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("o") && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("bc") && path.readable())
      return true;
  } else if (path.elide_file() && path.append_file(basename)) {
    if (path.append_suffix(Path::GetDLLSuffix()) && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("a") && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("o") && path.readable())
      return true;
    else if (path.elide_suffix() && path.append_suffix("bc") && path.readable())
      return true;
  }
  path.clear();
  return false;
}

Path 
Path::GetLibraryPath(const std::string& basename, 
                     const std::vector<std::string>& LibPaths) {
  Path result;

  // Try the paths provided
  for (std::vector<std::string>::const_iterator I = LibPaths.begin(),
       E = LibPaths.end(); I != E; ++I ) {
    if (result.set_directory(*I) && IsLibrary(result,basename))
      return result;
  }

  // Try the LLVM lib directory in the LLVM install area
  //if (result.set_directory(LLVM_LIBDIR) && IsLibrary(result,basename))
  //  return result;

  // Try /usr/lib
  if (result.set_directory("/usr/lib/") && IsLibrary(result,basename))
    return result;

  // Try /lib
  if (result.set_directory("/lib/") && IsLibrary(result,basename))
    return result;

  // Can't find it, give up and return invalid path.
  result.clear();
  return result;
}

Path
Path::GetSystemLibraryPath1() {
  return Path("/lib/");
}

Path
Path::GetSystemLibraryPath2() {
  return Path("/usr/lib/");
}

Path
Path::GetLLVMDefaultConfigDir() {
  return Path("/etc/llvm/");
}

Path
Path::GetLLVMConfigDir() {
  return GetLLVMDefaultConfigDir();
}

Path
Path::GetUserHomeDirectory() {
  const char* home = getenv("HOME");
  if (home) {
    Path result;
    if (result.set_directory(home))
      return result;
  }
  return GetRootDirectory();
}
// FIXME: the above set of functions don't map to Windows very well.

bool
Path::is_file() const {
  return (is_valid() && path[path.length()-1] != '/');
}

bool
Path::is_directory() const {
  return (is_valid() && path[path.length()-1] == '/');
}

std::string
Path::get_basename() const {
  // Find the last slash
  size_t slash = path.rfind('/');
  if (slash == std::string::npos)
    slash = 0;
  else
    slash++;

  return path.substr(slash, path.rfind('.'));
}

bool Path::has_magic_number(const std::string &Magic) const {
  size_t len = Magic.size();
  char *buf = reinterpret_cast<char *>(_alloca(len+1));
  std::ifstream f(path.c_str());
  f.read(buf, len);
  buf[len] = '\0';
  return Magic == buf;
}

bool 
Path::is_bytecode_file() const {
  if (readable()) {
    return has_magic_number("llvm");
  }
  return false;
}

bool
Path::is_archive() const {
  if (readable()) {
    return has_magic_number("!<arch>\012");
  }
  return false;
}

bool
Path::exists() const {
  DWORD attr = GetFileAttributes(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES;
}

bool
Path::readable() const {
  // FIXME: take security attributes into account.
  DWORD attr = GetFileAttributes(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES;
}

bool
Path::writable() const {
  // FIXME: take security attributes into account.
  DWORD attr = GetFileAttributes(path.c_str());
  return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_READONLY);
}

bool
Path::executable() const {
  // FIXME: take security attributes into account.
  DWORD attr = GetFileAttributes(path.c_str());
  return attr != INVALID_FILE_ATTRIBUTES;
}

std::string
Path::getLast() const {
  // Find the last slash
  size_t pos = path.rfind('/');

  // Handle the corner cases
  if (pos == std::string::npos)
    return path;

  // If the last character is a slash
  if (pos == path.length()-1) {
    // Find the second to last slash
    size_t pos2 = path.rfind('/', pos-1);
    if (pos2 == std::string::npos)
      return path.substr(0,pos);
    else
      return path.substr(pos2+1,pos-pos2-1);
  }
  // Return everything after the last slash
  return path.substr(pos+1);
}

bool
Path::set_directory(const std::string& a_path) {
  if (a_path.size() == 0)
    return false;
  Path save(*this);
  path = a_path;
  size_t last = a_path.size() -1;
  if (last != 0 && a_path[last] != '/')
    path += '/';
  if (!is_valid()) {
    path = save.path;
    return false;
  }
  return true;
}

bool
Path::set_file(const std::string& a_path) {
  if (a_path.size() == 0)
    return false;
  Path save(*this);
  path = a_path;
  size_t last = a_path.size() - 1;
  while (last > 0 && a_path[last] == '/')
    last--;
  path.erase(last+1);
  if (!is_valid()) {
    path = save.path;
    return false;
  }
  return true;
}

bool
Path::append_directory(const std::string& dir) {
  if (is_file())
    return false;
  Path save(*this);
  path += dir;
  path += "/";
  if (!is_valid()) {
    path = save.path;
    return false;
  }
  return true;
}

bool
Path::elide_directory() {
  if (is_file())
    return false;
  size_t slashpos = path.rfind('/',path.size());
  if (slashpos == 0 || slashpos == std::string::npos)
    return false;
  if (slashpos == path.size() - 1)
    slashpos = path.rfind('/',slashpos-1);
  if (slashpos == std::string::npos)
    return false;
  path.erase(slashpos);
  return true;
}

bool
Path::append_file(const std::string& file) {
  if (!is_directory())
    return false;
  Path save(*this);
  path += file;
  if (!is_valid()) {
    path = save.path;
    return false;
  }
  return true;
}

bool
Path::elide_file() {
  if (is_directory())
    return false;
  size_t slashpos = path.rfind('/',path.size());
  if (slashpos == std::string::npos)
    return false;
  path.erase(slashpos+1);
  return true;
}

bool
Path::append_suffix(const std::string& suffix) {
  if (is_directory())
    return false;
  Path save(*this);
  path.append(".");
  path.append(suffix);
  if (!is_valid()) {
    path = save.path;
    return false;
  }
  return true;
}

bool
Path::elide_suffix() {
  if (is_directory()) return false;
  size_t dotpos = path.rfind('.',path.size());
  size_t slashpos = path.rfind('/',path.size());
  if (slashpos != std::string::npos && dotpos != std::string::npos &&
      dotpos > slashpos) {
    path.erase(dotpos, path.size()-dotpos);
    return true;
  }
  return false;
}


bool
Path::create_directory( bool create_parents) {
  // Make sure we're dealing with a directory
  if (!is_directory()) return false;

  // Get a writeable copy of the path name
  char *pathname = reinterpret_cast<char *>(_alloca(path.length()+1));
  path.copy(pathname,path.length()+1);

  // Null-terminate the last component
  int lastchar = path.length() - 1 ;
  if (pathname[lastchar] == '/')
    pathname[lastchar] = 0;

  // If we're supposed to create intermediate directories
  if ( create_parents ) {
    // Find the end of the initial name component
    char * next = strchr(pathname,'/');
    if ( pathname[0] == '/')
      next = strchr(&pathname[1],'/');

    // Loop through the directory components until we're done
    while ( next != 0 ) {
      *next = 0;
      if (!CreateDirectory(pathname, NULL))
          ThrowError(std::string(pathname) + ": Can't create directory: ");
      char* save = next;
      next = strchr(pathname,'/');
      *save = '/';
    }
  } else if (!CreateDirectory(pathname, NULL)) {
    ThrowError(std::string(pathname) + ": Can't create directory: ");
  }
  return true;
}

bool
Path::create_file() {
  // Make sure we're dealing with a file
  if (!is_file()) return false;

  // Create the file
  HANDLE h = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE)
    ThrowError(std::string(path.c_str()) + ": Can't create file: ");

  CloseHandle(h);
  return true;
}

bool
Path::destroy_directory(bool remove_contents) {
  // Make sure we're dealing with a directory
  if (!is_directory()) return false;

  // If it doesn't exist, we're done.
  if (!exists()) return true;

  char *pathname = reinterpret_cast<char *>(_alloca(path.length()+1));
  path.copy(pathname,path.length()+1);
  int lastchar = path.length() - 1 ;
  if (pathname[lastchar] == '/')
    pathname[lastchar] = 0;

  if (remove_contents) {
    // Recursively descend the directory to remove its content
    // FIXME: The correct way of doing this on Windows isn't pretty...
    // but this may work if unix-like utils are present.
    std::string cmd("rm -rf ");
    cmd += path;
    system(cmd.c_str());
  } else {
    // Otherwise, try to just remove the one directory
    if (!RemoveDirectory(pathname))
      ThrowError(std::string(pathname) + ": Can't destroy directory: ");
  }
  return true;
}

bool
Path::destroy_file() {
  if (!is_file()) return false;

  DWORD attr = GetFileAttributes(path.c_str());

  // If it doesn't exist, we're done.
  if (attr == INVALID_FILE_ATTRIBUTES)
    return true;

  // Read-only files cannot be deleted on Windows.  Must remove the read-only
  // attribute first.
  if (attr & FILE_ATTRIBUTE_READONLY) {
    if (!SetFileAttributes(path.c_str(), attr & ~FILE_ATTRIBUTE_READONLY))
      ThrowError(std::string(path.c_str()) + ": Can't destroy file: ");
  }

  if (!DeleteFile(path.c_str()))
    ThrowError(std::string(path.c_str()) + ": Can't destroy file: ");
  return true;
}

}
}

// vim: sw=2 smartindent smarttab tw=80 autoindent expandtab


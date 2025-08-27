#pragma once
#include "System/ProgressTask.hpp"
#include "fslib.hpp"

#include <minizip/unzip.h>
#include <minizip/zip.h>

namespace FS
{
    // This recursively copies Source to Destination. Progress task is so the progress can be shown to the user. Commit is false
    // by default. This needs to be true for User and System saves.
    void CopyDirectoryToDirectory(System::ProgressTask *Task,
                                  const fslib::Path &Source,
                                  const fslib::Path &Destination,
                                  bool Commit);
    // This recursively copies source to the zipFile passed. This needs to be like this, because 3DS threads don't normally have
    // enough stack space for minizip to work.
    void CopyDirectoryToZip(System::ProgressTask *Task, const fslib::Path &Source, zipFile Destination);
    // This unzips the unzFile passed to Destination
    void CopyZipToDirectory(System::ProgressTask *Task, unzFile Source, const fslib::Path &Destination, bool Commit);
} // namespace FS

#ifndef MAPPED_CACHE_FILE_H
#define MAPPED_CACHE_FILE_H

// This implements the handling of a pair of files, typically a
// "source" text file containing, say, a dictionary, and a "cache"
// file containing the result of pre-processing that dictionary.  The
// intention is that a program normally wants to use the precomputed
// information in the cache file, so that it can run quickly in the
// normal case.  But, whenever the source file changes, it recomputes
// the cached information, stores it in the cache file for future
// runs, and proceeds.  This simplifies the management of such pairs
// of files, and obviates the user inconvenience of having two
// programs, one for preparing the precomputed file and another for
// using it.

// These procedures use a client-supplied version number, the source
// file length, the source file modification time, and a measurement
// of the system's endian-ness to determine whether the source file is
// stale.  Those items are stored in the first few words of the cache
// file, and are invisible to the client.  The version number is
// normally a constant in your program.  Whenever you change your
// program so that the format of the information in the cache file
// might be incompatible, increase the version number.  This will
// cause all extant cache files to be considered stale.  You may also,
// of course, delete cache files at any time.

// The constructor takes the number of source files (typically 1), the
// source file names, the extension for the cache file, and the
// version number.  A typical call might look like:
//
//     char *sourcenames[1] = {"dict87.txt"};
//     MAPPED_CACHE_FILE the_cache(1, sourcenames, "frequencycount", 13);
//
// This would use the single file "dict87.txt" as the source file, and
// use the cache file "dict87.frequencycount".
//
// To use multiple source files, do this:
//
//     char *sourcenames[3] = {"alpha.txt", "beta.stats", "gamma.data"};
//     MAPPED_CACHE_FILE the_cache(3, sourcenames, "xyz", 13);

// This would use the given source files, and use the cache file
// "alpha+beta+gamma.xyz".  In this example, the version number is 13.
// We will change that whenever we modify the program in a way that
// changes the format of the cache file.  Doing so will cause all
// extant cache files to become obsolete.

// The constructor can take an optional 5th argument, which is an
// array of booleans, the same number as the number of source files.
// Each boolean in the array tells whether the corresponding source
// file is to be opened in binary mode.  If false, the file is opened
// in text mode.  If the argument array is not given, all files are
// opened in text mode.

// The constructor takes no action other than to save this
// information.  It saves only the the pointer to the list of pointers
// to the names of the source files, and the pointer to the list of
// binary mode flags.  You must not alter any of those arrays, or the
// source file name strings, until after "open_and_map" is called,

// The method "open_and_map" opens the source files as a plain POSIX
// files, in read-only mode.  They are opened in binary mode if the
// final optional argument array says to do so; otherwise they are in
// text mode.  The "srcfiles" argument is an array of "FILE *" items,
// the same number of them as specified during the constructor.  We
// open the files, and fill this array.  When we return, the cache
// file may also be open and mapped, perhaps in a read-only memory
// segment.  The source files will not have had anything read from
// them, so they will still be at the beginning of the file.  Examine
// the returned address pointer first.  If it is nonzero, we found and
// opened the cache file and all the source files, and found that the
// cache file matched the version numbers and the size and creation
// times of the source files.  The contents of the cache file (other
// than our header) are mapped at the returned address.  It will be
// word-aligned.  The source files are also open.

// At this point you may elect to look at the mapped memory and do
// further validation of its contents, perhaps reading the source
// files.  Use the "srcfiles" array for this.  If you find things to
// your liking, you may use the data.  You should close the source
// files at some point.

// The object destructor will remove the mapping and release the
// cache file.

// If the address returned by "open_and_map" is zero, or if you don't
// like what you see in the mapped memory, the cache file must be
// rewritten.  First, check the "srcfiles" array.  If any item is
// zero, we couldn't even open that source file.  You should
// presumably give some error message to that effect.  The other
// source file descriptors, if any, might be nonzero.  You should
// close them.

// If all the source files were successfully opened, as indicated by
// the "srcfiles" array elements all being nonzero, use those files to
// compute the new cached data.  Figure out how big that data will be,
// in bytes, and pass it as the argument to "map_for_writing".  The
// returned address will be the area into which you should write the
// indicated number of bytes of data.  It will be word-aligned, and,
// of course, writeable.  The address that had been returned by
// "open_and_map" will no longer be valid at this point.  You must use
// the new address from "map_for_writing".  After you have finished
// with the source files, you should close them.  At this point you
// may use the mapped memory area into which you have written the
// cached data, exactly as you would have used the memory area
// returned by "open_and_map".  Don't modify it, as the cache file may
// not yet have been written.

// The object destructor will remove the mapping and write and close
// the cache file.  The created cache file will be the size indicated
// in the call to "map_for_writing", plus 20 bytes or so for our
// header.  The destructor does *not* close the source files.  You
// should have closed them when you were finished reading them, or
// when you realized that you wouldn't need them.

// The information that we use for validating the cache file is the
// version number (provided by you, the client) along with the
// "st_size" and "st_mtime" attributes of the source file.  Whether
// those attributes are portable from one operating system to another
// is questionable.  There may also be endian-ness or word size
// problems that we do not catch, although we try to check that.  We
// do not recommend moving cache files from one system to another.

// The Windows NT version of this uses the file mapping system
// operations, and should be very efficient.

#include <stdio.h>

struct MAPPED_CACHE_INNARDS;

class MAPPED_CACHE_FILE {
   struct MAPPED_CACHE_INNARDS *innards;
 public:
   MAPPED_CACHE_FILE(int numsourcefiles,
                     const char * const * srcnames,
                     const char *mapext,
                     int clientversion,
                     const bool *srcbinary = 0);
   ~MAPPED_CACHE_FILE();

   int *open_and_map(FILE * * srcfiles);
   int *map_for_writing(int clientmapfilesize);
};

#endif

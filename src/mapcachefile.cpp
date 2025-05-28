// This is mapcachefile.cpp.
//
//    Copyright (C) 2003  William B. Ackerman.

// If this is for Windows or Linux, use the mapped file mechanism.
// Otherwise, read and write files the old-fashioned way.

// NOTE: This should be a static library, not a DLL.  DLL's don't properly
// return opened POSIX file descriptors to the callee.  If this is statically
// part of a DLL, but is not exported from the DLL, that's OK.

#include "mapcachefile.h"
#include <string.h>
#include <sys/stat.h>

#if defined(WIN32)
// This shuts off a lot of obscure stuff and makes the compilation go faster.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

// Mapped file format:
// 5 words (or so) of header.
//   The first word is the length of the client part of the file
//   The second word is the file format version number.
//   The third word is our determination of the endianness and word size
//   The fourth word is the length of the corresponding source file.
//   The fifth word is the last modify time of the corresponding source file.
//   Those last 2 words are repeated if multiple source files are used.
// Followed by whatever the client wants.

struct MAPPED_CACHE_INNARDS {
#if defined(WIN32)
   HANDLE mapfilemaphandle;
   HANDLE mapfilehandle;
#elif defined(__linux__)
   int mapfd;
#else
   FILE *mapfiledesc;
#endif
   int *map_address;
   int numsourcefiles;
   const char * const *srcnames;
   const bool *srcbinary;
   char *mapfilename;
   struct stat *source_stats;
   int filewords;
   int sversion;
   int header_size_in_words;
   bool properly_opened;
};


MAPPED_CACHE_FILE::MAPPED_CACHE_FILE(int numsourcefiles,
                                     const char * const * srcnames,
                                     const char *mapext,
                                     int clientversion,
                                     const bool *srcbinary)
{
   innards = new struct MAPPED_CACHE_INNARDS;
   innards->numsourcefiles = numsourcefiles;
   innards->srcnames = srcnames;
   innards->srcbinary = srcbinary;
   innards->sversion = clientversion;
   innards->header_size_in_words = 3 + 2*numsourcefiles;
   innards->properly_opened = false;
   innards->source_stats = new struct stat [numsourcefiles];

   // Figure out the map file name.  Use a conservative estimate for the size.
   int mapfilenamesize = strlen(mapext) + numsourcefiles;
   int i, j;

   for (i=0 ; i<numsourcefiles ; i++)
      mapfilenamesize += strlen(srcnames[i]);

   innards->mapfilename = new char [mapfilenamesize];

   int filenamepos = 0;

   // Append the source file base names.
   for (i=0 ; i<numsourcefiles ; i++) {
      const char *this_src = srcnames[i];
      for (j=strlen(this_src)-1 ; ; j--) {
         if (j <= 0 || this_src[j] == '.') {
            if (j <= 0) j = strlen(this_src);
            ::memcpy(innards->mapfilename+filenamepos, this_src, j);
            filenamepos += j;
            if (i != numsourcefiles-1)
               innards->mapfilename[filenamepos++] = '+';
            break;
         }
      }
   }

   innards->mapfilename[filenamepos++] = '.';
   ::strcpy(innards->mapfilename+filenamepos, mapext);
}

int *MAPPED_CACHE_FILE::open_and_map(FILE * * srcfiles)
{
   // Open the source files.

   bool cantopen = false;

   int i;
   for (i=0 ; i<innards->numsourcefiles ; i++) {
      // If last argument of constructor isn't given, it defaults to zero,
      // and we will interpret that as making all files text files.
      srcfiles[i] = fopen(innards->srcnames[i],
                          (innards->srcbinary && innards->srcbinary[i]) ? "rb" : "r");
      if (!srcfiles[i]) {
         // We will leave this file descriptor zero, which the client
         // will find.  The client will conclude that we can't proceed
         // with the operation, since we can't open the source files.
         cantopen = true;
      }
      else if (fstat(fileno(srcfiles[i]), &innards->source_stats[i])) {
         // If we can open the source files but can't get their
         // modification times, it's still possible to proceed.  We
         // will leave the file descriptor nonzero.  But we will
         // report that the cache file is stale.  The client can still
         // read the source files and perform the computation of the
         // new data.  The client wants to think of this in terms of a
         // mapped file, so we allow the cache file to be opened and
         // written to.  However, when the cache file is written, we
         // will cause it to have bogus data for the stat information.
         // On a future run, if the source file's stat information has
         // been repaired, we will see a mismatch and recompute things
         // one more time, but after that it may be OK.
         cantopen = true;
         innards->source_stats[i].st_size = ~0;
         innards->source_stats[i].st_mtime = ~0;
      }
   }

   if (cantopen) return (int *) 0;

   innards->map_address = (int *) 0;
   innards->properly_opened = true;

#if defined(WIN32)
   innards->mapfilemaphandle = (HANDLE) 0;
   innards->mapfilehandle = CreateFile(innards->mapfilename, GENERIC_READ,
                          FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   if (!innards->mapfilehandle || (int) innards->mapfilehandle == ~0) return (int *) 0;

   innards->mapfilemaphandle = CreateFileMapping(
         innards->mapfilehandle, 0, PAGE_READONLY, 0, 0, 0);

   if (!innards->mapfilemaphandle) return (int *) 0;

   innards->map_address = (int *) MapViewOfFile(innards->mapfilemaphandle,
                                                FILE_MAP_READ, 0, 0, 0);
   if (!innards->map_address) return (int *) 0;
#elif defined(__linux__)
   innards->mapfd = open(innards->mapfilename, O_RDONLY);
   if (innards->mapfd < 0) return (int *) 0;
   if ((int) read(innards->mapfd, &innards->filewords, 4) != 4) return (int *) 0;
   innards->filewords >>= 2;

   innards->map_address = (int *) mmap(0, innards->filewords<<2, PROT_READ,
                                       MAP_SHARED, innards->mapfd, 0);

   if (((int) innards->map_address) < 0) innards->map_address = (int *) 0;
   if (!innards->map_address) return (int *) 0;
#else
   innards->mapfiledesc = fopen(innards->mapfilename, "rb");

   if (!innards->mapfiledesc) return (int *) 0;
   if ((int) fread(&innards->filewords, 4, 1, innards->mapfiledesc) != 1) return (int *) 0;
   innards->filewords >>= 2;

   innards->map_address = new int [innards->filewords];
   if (!innards->map_address) return (int *) 0;
   if ((int) fread(innards->map_address+1, 4, innards->filewords-1, innards->mapfiledesc) !=
       innards->filewords-1)
      return (int *) 0;
#endif

   // Check the particulars of the source file against what the map file claims.

   int endiantest = 0;
   ((char *) &endiantest)[1] = sizeof(int);  // Also tests int size.

   bool bad_match = innards->map_address[1] != innards->sversion ||
      innards->map_address[2] != endiantest;

   for (i=0 ; i<innards->numsourcefiles ; i++) {
      if (innards->map_address[3+2*i] != innards->source_stats[i].st_size ||
          innards->map_address[4+2*i] != innards->source_stats[i].st_mtime)
         bad_match = true;
   }

   if (bad_match) return (int *) 0;
   return innards->map_address + innards->header_size_in_words;
}


int *MAPPED_CACHE_FILE::map_for_writing(int clientmapfilesize)
{
   if (!innards->properly_opened) return (int *) 0;

   clientmapfilesize += innards->header_size_in_words * sizeof(int);

#if defined(WIN32)
   if (innards->map_address) {
      // We thought the cache was OK, but the client
      // has rejected it.
      FlushViewOfFile(innards->map_address, 0);
      UnmapViewOfFile(innards->map_address);
   }

   // Close the handles that we had, because they were read-only.

   if (innards->mapfilehandle && (int) innards->mapfilehandle != ~0)
      CloseHandle(innards->mapfilehandle);
   if (innards->mapfilemaphandle)
      CloseHandle(innards->mapfilemaphandle);

   // Open the map file again, this time for writing.

   innards->mapfilehandle = CreateFile(innards->mapfilename, GENERIC_READ|GENERIC_WRITE,
                          0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

   if (!innards->mapfilehandle) return (int *) 0;

   innards->mapfilemaphandle = CreateFileMapping(innards->mapfilehandle, 0,
                                                 PAGE_READWRITE, 0, clientmapfilesize, 0);

   if (!innards->mapfilemaphandle) return (int *) 0;

   innards->map_address = (int *) MapViewOfFile(innards->mapfilemaphandle,
                                                FILE_MAP_WRITE, 0, 0, 0);
#elif defined(__linux__)
   if (innards->map_address) munmap(innards->map_address, innards->filewords<<2);
   if (innards->mapfd > 0) close(innards->mapfd);
   innards->map_address = (int *) 0;
   innards->filewords = (clientmapfilesize+3) >> 2;
   innards->mapfd = open(innards->mapfilename, O_WRONLY|O_CREAT|O_TRUNC,
                         S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
   if (innards->mapfd < 0) return (int *) 0;

   // We need to write to the file -- we can't just map a size and have the
   // file pages come into existence.

   {
      char *buffer = new char [1024];
      int bytesleft = innards->filewords<<2;
      while (bytesleft > 0) {
         int size = 1024;
         if (size > bytesleft) size = bytesleft;
         write(innards->mapfd, buffer, size);
         bytesleft -= size;
      }
      delete [] buffer;
   }

   // Now we need to close it, and open it again.
   close(innards->mapfd);
   innards->mapfd = open(innards->mapfilename, O_RDWR);
   if (innards->mapfd < 0) return (int *) 0;
   innards->map_address = (int *) mmap(0, innards->filewords<<2, PROT_READ|PROT_WRITE,
                                       MAP_SHARED, innards->mapfd, 0);
   if (((int) innards->map_address) < 0) innards->map_address = (int *) 0;
#else
   if (innards->mapfiledesc) fclose(innards->mapfiledesc);
   if (innards->map_address) delete [] innards->map_address;
   innards->map_address = (int *) 0;
   innards->filewords = (clientmapfilesize+3) >> 2;
   innards->mapfiledesc = fopen(innards->mapfilename, "wb");
   if (!innards->mapfiledesc) return (int *) 0;
   innards->map_address = new int [innards->filewords];
#endif

   if (!innards->map_address) return (int *) 0;

   // Write the header.
   innards->map_address[0] = clientmapfilesize;
   innards->map_address[1] = innards->sversion;
   innards->map_address[2] = 0;
   ((char *) &innards->map_address[2])[1] = sizeof(int);

   int i;
   for (i=0 ; i<innards->numsourcefiles ; i++) {
      innards->map_address[3+2*i] = innards->source_stats[i].st_size;
      innards->map_address[4+2*i] = innards->source_stats[i].st_mtime;
   }

   return innards->map_address + innards->header_size_in_words;
}

MAPPED_CACHE_FILE::~MAPPED_CACHE_FILE()
{
   if (innards->properly_opened) {
#if defined(WIN32)
      if (FlushViewOfFile(innards->map_address, 0) &&
          UnmapViewOfFile(innards->map_address)) {
         CloseHandle(innards->mapfilehandle);
         CloseHandle(innards->mapfilemaphandle);
      }
#elif defined(__linux__)
      if (innards->map_address) munmap(innards->map_address,
                                       innards->filewords<<2);
      if (innards->mapfd > 0) close(innards->mapfd);
#else
      if (innards->map_address) {
         fwrite(innards->map_address, 4, innards->filewords, innards->mapfiledesc);
         delete [] innards->map_address;
      }

      if (innards->mapfiledesc) fclose(innards->mapfiledesc);
#endif
   }

   delete [] innards->source_stats;
   delete [] innards->mapfilename;
   delete innards;
}

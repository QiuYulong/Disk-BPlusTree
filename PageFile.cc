#define _USE_32BIT_TIME_T 
#define NOCACHE
#include "BPBase.h"
#include "PageFile.h"
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <stdio.h>
//#include <cstdio>
//#include <stdlib.h>
//#include <stdio.h>

using std::string;

int PageFile::readCount = 0;
int PageFile::writeCount = 0;
int PageFile::cacheClock = 1;
struct PageFile::cacheStruct PageFile::readCache[PageFile::CACHE_COUNT];

PageFile::PageFile() 
{ 
  fd = -1; 
  epid = 0; 
}

PageFile::PageFile(const string& filename, char mode)
{
  fd = -1;
  epid = 0;
  open(filename.c_str(), mode);
}

RC PageFile::open(const string& filename, char mode)
{
  RC   rc;
  int  oflag;
  struct _stat statbuf;

  if (fd > 0) return RC_FILE_OPEN_FAILED;

  // set the unix file flag depending on the file mode
  switch (mode) {
  case 'r':
  case 'R':
	  oflag = _O_RDONLY|_O_BINARY;
    break;
  case 'w':
  case 'W':
    oflag = (_O_RDWR|_O_CREAT|_O_BINARY);
    break;
  default:
    return RC_INVALID_FILE_MODE;
  }

  // open the file
  fd = _open(filename.c_str(), oflag, 0644);
  if (fd < 0) { fd = -1; return RC_FILE_OPEN_FAILED; }

  // get the size of the file to set the end pid
  rc = _fstat32(fd, &statbuf);
  if (rc < 0) { _close(fd); fd = -1; return RC_FILE_OPEN_FAILED; }
  epid = statbuf.st_size / PAGE_SIZE;

  return 0;
}

RC PageFile::close()
{
  if (fd <= 0) return RC_FILE_CLOSE_FAILED;

  // close the file
  if (_close(fd) < 0) return RC_FILE_CLOSE_FAILED;

#ifndef NOCACHE
  // evict all cached pages for this file
  for (int i = 0; i < CACHE_COUNT; i++) {
    if (readCache[i].fd == fd && readCache[i].lastAccessed != 0) {
       readCache[i].fd = 0;
       readCache[i].pid = 0;
       readCache[i].lastAccessed = 0;
    }
  }
#endif

  // set the fd and epid to the initial state
  fd = -1; 
  epid = 0;
  return 0;
}

PageId PageFile::endPid() const 
{
  return epid;
}

RC PageFile::seek(PageId pid) const
{
  return (_lseek(fd, pid * PAGE_SIZE, SEEK_SET) < 0) ? RC_FILE_SEEK_FAILED : 0;
}

RC PageFile::write(PageId pid, const void* buffer)
{
  RC rc;
  if (pid < 0) return RC_INVALID_PID; 

  // seek to the location of the page
  if ((rc = seek(pid) < 0)) return rc;

  // write the buffer to the disk page
  if (_write(fd, buffer, PAGE_SIZE) < 0) return RC_FILE_WRITE_FAILED;

#ifndef NOCACHE
  // if the page is in read cache, invalidate it
  for (int i = 0; i < CACHE_COUNT; i++) {
    if (readCache[i].fd == fd && readCache[i].pid == pid &&
       readCache[i].lastAccessed != 0) {
       readCache[i].fd = 0;
       readCache[i].pid = 0;
       readCache[i].lastAccessed = 0;
       break;
    }
  }
#endif

  // if the written pid >= end pid, update the end pid
  if (pid >= epid) epid = pid + 1;

  // increase page write count
  writeCount++;

  return 0;
}

RC PageFile::read(PageId pid, void* buffer) const
{
#ifdef NOCACHE
	RC rc;
	DEBUG('p',"Read file fd:%d pid:%d without cache\n",fd ,pid);
	if (pid < 0 || pid >= epid) return RC_INVALID_PID; 
	// seek to the page
	if ((rc = seek(pid) < 0)) return rc;
	if(_read(fd, buffer, PAGE_SIZE) < 0)
		return RC_FILE_READ_FAILED;
	return 0;
#else
	RC rc;
	DEBUG('p',"Read file fd:%d pid:%d with cache",fd ,pid);
	if (pid < 0 || pid >= epid) return RC_INVALID_PID; 

	//
	// if the page is in cache, read it from there
	//
	for (int i = 0; i < CACHE_COUNT; i++) {
	if (readCache[i].fd == fd && readCache[i].pid == pid && 
		readCache[i].lastAccessed != 0) {
		memcpy(buffer, readCache[i].buffer, PAGE_SIZE);
		readCache[i].lastAccessed = ++cacheClock;
		DEBUG('p',", cache hit\n");
		return 0;
	}
	}
	DEBUG('p',"\n");

	// seek to the page
	if ((rc = seek(pid) < 0)) return rc;
  
	// find the cache slot to evict
	int toEvict = 0; 
	for (int i = 0; i < CACHE_COUNT; i++) {
	if (readCache[i].lastAccessed == 0) {
		toEvict = i;
		break;
	}
	if (readCache[i].lastAccessed < readCache[toEvict].lastAccessed) {
		toEvict = i;
	}
	}
	readCache[toEvict].fd = fd;
	readCache[toEvict].pid = pid;
	readCache[toEvict].lastAccessed = ++cacheClock;
 
	// read the page to cache first and copy it to the buffer
	if (_read(fd, readCache[toEvict].buffer, PAGE_SIZE) < 0) {
	return RC_FILE_READ_FAILED;
	}
	memcpy(buffer, readCache[toEvict].buffer, PAGE_SIZE);

	// increase the page read count
	readCount++;
   
	return 0;
#endif
}
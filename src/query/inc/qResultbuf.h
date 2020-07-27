/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TDENGINE_QRESULTBUF_H
#define TDENGINE_QRESULTBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tlist.h>
#include "hash.h"
#include "os.h"
#include "qExtbuffer.h"
#include "tlockfree.h"

typedef struct SArray* SIDList;

typedef struct SPageDiskInfo {
  int32_t offset;
  int32_t length;
} SPageDiskInfo;

typedef struct SPageInfo {
  SListNode*    pn;       // point to list node
  int32_t       pageId;
  SPageDiskInfo info;
  void*         pData;
  bool          used;     // set current page is in used
} SPageInfo;

typedef struct SFreeListItem {
  int32_t offset;
  int32_t len;
} SFreeListItem;

typedef struct SResultBufStatis {
  int32_t flushBytes;
  int32_t loadBytes;
  int32_t getPages;
  int32_t releasePages;
  int32_t flushPages;
  int32_t fileSize;
} SResultBufStatis;

typedef struct SDiskbasedResultBuf {
  int32_t   numOfRowsPerPage;
  int32_t   numOfPages;
  int64_t   totalBufSize;
  int64_t   diskFileSize;        // disk file size
  FILE*     file;
  int32_t   allocateId;          // allocated page id
  char*     path;                // file path
  int32_t   pageSize;            // current used page size
  int32_t   inMemPages;          // numOfPages that are allocated in memory
  SHashObj* groupSet;            // id hash table
  SHashObj* all;
  SList*    lruList;
  void*     handle;              // for debug purpose
  void*     emptyDummyIdList;    // dummy id list
  bool      comp;                // compressed before flushed to disk
  void*     assistBuf;           // assistant buffer for compress data
  SArray*   pFree;               // free area in file
  int32_t   nextPos;             // next page flush position

  SResultBufStatis statis;
} SDiskbasedResultBuf;

#define DEFAULT_INTERN_BUF_PAGE_SIZE (1024L)
#define DEFAULT_INMEM_BUF_PAGES       10
#define PAGE_INFO_INITIALIZER         (SPageDiskInfo){-1, -1}

/**
 * create disk-based result buffer
 * @param pResultBuf
 * @param size
 * @param rowSize
 * @return
 */
int32_t createDiskbasedResultBuffer(SDiskbasedResultBuf** pResultBuf, int32_t numOfPages, int32_t rowSize, int32_t pagesize,
                                    int32_t inMemPages, const void* handle);

/**
 *
 * @param pResultBuf
 * @param groupId
 * @param pageId
 * @return
 */
tFilePage* getNewDataBuf(SDiskbasedResultBuf* pResultBuf, int32_t groupId, int32_t* pageId);

/**
 *
 * @param pResultBuf
 * @return
 */
size_t getNumOfRowsPerPage(const SDiskbasedResultBuf* pResultBuf);

/**
 *
 * @param pResultBuf
 * @param groupId
 * @return
 */
SIDList getDataBufPagesIdList(SDiskbasedResultBuf* pResultBuf, int32_t groupId);

/**
 * get the specified buffer page by id
 * @param pResultBuf
 * @param id
 * @return
 */
tFilePage* getResBufPage(SDiskbasedResultBuf* pResultBuf, int32_t id);

/**
 * release the referenced buf pages
 * @param pResultBuf
 * @param page
 */
void releaseResBufPage(SDiskbasedResultBuf* pResultBuf, void* page);

void releaseResBufPageInfo(SDiskbasedResultBuf* pResultBuf, SPageInfo* pi);

/**
 *
 * @param pResultBuf
 * @param id
 * @return
 */
//tFilePage* getResBufPage(SDiskbasedResultBuf* pResultBuf, int32_t id);

/**
 * get the total buffer size in the format of disk file
 * @param pResultBuf
 * @return
 */
size_t getResBufSize(const SDiskbasedResultBuf* pResultBuf);

/**
 * get the number of groups in the result buffer
 * @param pResultBuf
 * @return
 */
size_t getNumOfResultBufGroupId(const SDiskbasedResultBuf* pResultBuf);

/**
 * destroy result buffer
 * @param pResultBuf
 */
void destroyResultBuf(SDiskbasedResultBuf* pResultBuf, void* handle);

/**
 *
 * @param pList
 * @return
 */
SPageInfo* getLastPageInfo(SIDList pList);

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_QRESULTBUF_H

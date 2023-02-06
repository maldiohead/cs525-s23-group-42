#include "storage_mgr.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

static FILE *fp;

void initStorageManager (void) {
    RC_message = "storage manager init success.";
}

RC createPageFile (char *fileName) {
    fp = fopen(fileName, "wb+");
    if (fp == NULL) {
        RC_message = "file open error.";
        return RC_FILE_NOT_FOUND;
    }

    char *temp = (char *) malloc(PAGE_SIZE);
    if (temp == NULL) {
        RC_message = "memory malloc is fail.";
        return RC_WRITE_FAILED;
    }
    memset(temp, '\0', PAGE_SIZE);
    fwrite(temp, sizeof(char), PAGE_SIZE, fp);

    fclose(fp);
    RC_message = "create page file is success.";
    return RC_OK;
}

static RC get_page_num(int *pageNum) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        return RC_ERROR;
    }
    int fileSize = ftell(fp);
    if (fileSize == -1) {
        return RC_ERROR;
    }
    *pageNum = fileSize / PAGE_SIZE;
    return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    RC ret = RC_OK;
    fp = fopen(fileName, "rb+");
    if (fp == NULL) {
        RC_message = "file open error.";
        return RC_FILE_NOT_FOUND;
    }

    fHandle->fileName = fileName;
    fHandle->mgmtInfo = fp;
    fHandle->curPagePos = 0;
    // calcuate the total page num = fileSize / pageSize
    int pageNum = 0;
    ret = get_page_num(&pageNum);
    if (ret != RC_OK) {
        RC_message = "get page number is fail.";
        return ret;
    }
    fHandle->totalNumPages = pageNum;

    RC_message = "Open the file and set the filehandler initialization information successfully";
    return ret;
}

RC closePageFile (SM_FileHandle *fHandle) {
    RC ret = RC_OK;
    if (fHandle->mgmtInfo == NULL) {
        RC_message = "file handle is not init";
        ret = RC_FILE_HANDLE_NOT_INIT;
        return ret;
    }

    fclose(fHandle->mgmtInfo);
    fHandle->mgmtInfo = NULL;
    fHandle->fileName = NULL;
    fHandle->totalNumPages = 0;
    fHandle->curPagePos = 0;
    RC_message = "close page file is success.";
    return ret;
}

RC destroyPageFile (char *fileName) {
    RC ret = RC_OK;
    if ((fp = fopen(fileName, "rb+")) == NULL) {
        ret = RC_FILE_NOT_FOUND;
        RC_message = "file open error.";
        return ret;
    }

    fclose(fp);
    remove(fileName);
    RC_message = "destory page file is success.";
    return ret;
}

static RC preChechReadBlockParam(SM_FileHandle *fHandle) {
    RC ret = RC_OK;
    if (fHandle == NULL) {
        ret = RC_ERROR;
        return ret;
    }

    fp = (FILE *)fHandle->mgmtInfo;
    if (fp == NULL) {
        ret = RC_ERROR;
        return ret;
    }
    return ret;
}

/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    RC ret = RC_OK;
    do {
        if ((ret = preChechReadBlockParam(fHandle)) != RC_OK) {
            RC_message = "file handle is not init";
            ret = RC_FILE_HANDLE_NOT_INIT;
            break;
        }

        if (pageNum > (fHandle->totalNumPages - 1) || pageNum < 0) {
            RC_message = "there is no page left";
            ret = RC_READ_NON_EXISTING_PAGE;
            break;
        }
        FILE *currentFP = (FILE *)fHandle->mgmtInfo;
        fseek(currentFP, pageNum * PAGE_SIZE * sizeof(char), SEEK_SET);
        fread(memPage, 1, PAGE_SIZE, currentFP);
        fHandle->curPagePos = pageNum;
        RC_message = "read block is success.";
    } while (0);
    return ret;
}

int getBlockPos(SM_FileHandle *fHandle) {
    if (preChechReadBlockParam(fHandle) != RC_OK) {
        RC_message = "file handler is not init";
        return RC_ERROR;
    }
    if (fHandle->curPagePos < 0) {
        RC_message = "file handler param is error.";
        return RC_ERROR;
    }
    RC_message = "get block pos success.";
    return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int curPagePos = getBlockPos(fHandle);
    if (curPagePos == RC_ERROR) {
        RC_message = "read previous block is fail, the page is not exist.";
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(curPagePos - 1, fHandle, memPage);
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int curPagePos = getBlockPos(fHandle);
    if (curPagePos == RC_ERROR) {
        RC_message = "read previous block is fail, the page is not exist.";
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(curPagePos, fHandle, memPage);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int curPagePos = getBlockPos(fHandle);
    if (curPagePos == RC_ERROR) {
        RC_message = "read previous block is fail, the page is not exist.";
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(curPagePos + 1, fHandle, memPage);
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    RC ret = RC_OK;
    do {
        if (preChechReadBlockParam(fHandle) != RC_OK) {
            RC_message = "file handle is not init";
            ret =  RC_FILE_HANDLE_NOT_INIT;
            break;
        }

        if (memPage == NULL) {
            RC_message = "the mempage is null, can't write block.";
            ret = RC_WRITE_FAILED;
            break;
        }

        ret = ensureCapacity(pageNum + 1, fHandle);
        if (ret != RC_OK) {
            break;
        }
        fHandle->curPagePos = pageNum;
        ret = writeCurrentBlock(fHandle, memPage);
        if (ret != RC_OK) {
            break;
        }
    } while (0);

    RC_message = "write block is success.";
    return ret;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    RC ret = RC_OK;
    do {
        if (preChechReadBlockParam(fHandle) != RC_OK) {
            RC_message = "file handle is not init";
            ret =  RC_FILE_HANDLE_NOT_INIT;
            break;
        }
        if (memPage == NULL) {
            RC_message = "the mempage is null, can't write block.";
            ret = RC_WRITE_FAILED;
            break;
        }
        FILE *currentFP = (FILE *)fHandle->mgmtInfo;
        int currentPosi = fHandle->curPagePos * PAGE_SIZE;
        fseek(currentFP, currentPosi, SEEK_SET);
        int memPageSize = strlen(memPage) + 1;
        int writeSize = memPageSize < PAGE_SIZE ? memPageSize : PAGE_SIZE;
        fwrite(memPage, 1,writeSize, currentFP);
    } while (0);

    RC_message = "Write current block Successfully";
    return ret;
}

RC appendEmptyBlock (SM_FileHandle *fHandle) {
    char *newPage = (char *)malloc(PAGE_SIZE * sizeof(char));
    if (newPage == NULL) {
        RC_message = "memory malloc is fail.";
        return RC_WRITE_FAILED;
    }

    fseek(fp, 0, SEEK_END);
    memset(newPage, '\0', PAGE_SIZE * sizeof(char));
    fwrite(newPage, sizeof(char), PAGE_SIZE, fp);
    free(newPage);
    fHandle->totalNumPages++;

    RC_message = "append empty block is success.";
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {

    if (preChechReadBlockParam(fHandle) != RC_OK ) {
        RC_message = "file handle is not init";
        return RC_FILE_HANDLE_NOT_INIT;
    }

    while (numberOfPages > (fHandle->totalNumPages)) {
        appendEmptyBlock(fHandle);
    }

    return RC_OK;
}

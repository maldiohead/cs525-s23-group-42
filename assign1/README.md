# Storage Manager Describe
   The goal of this assignment is to implement a simple storage manager - a module that is capable of reading blocks from a file on disk into memory and writing blocks from memory to a file on disk

## Project Code Structure:
assign1
-     dberror.c 
      dberror.h 
      storage_mgr.c 
      storage_mgr.h 
      test_assign1_1.c 
      test_helper.h 
      Makefile
      README.md

## Member:
-   Liming Liu
-   Weibo Wang
-   Dongjing Xie
      
## Run This Program:

### 1. Environment
   The project requires linux and gcc compilation environment.
   
### 2. Run Step 
-     make clean
      make
      make run (or exectute ./test_assign1_1 directly)
   
## The Key Function implement (in the storage_mgr.c file):
### createPageFile
- Create a file with the given name and set the initial file size to PAGE_SIZE.
### openPageFile
- Open the file and initialize the structure(SM_FileHandle), and return an error if it fails.
### closePageFile
- Close the file and reinitialize the structure(SM_FileHandle).
### destroyPageFile
- Delete Files.
### readBlock
- Get the context in the position.
### readFirstBlock 
- Get the context in the first block position.
### readLastBlock
- Get the context in the last block position.
### readPreviousBlock
- Get the context in the previous position.
### readCurrentBlock
- Get the context in the current position.
### readNextBlock
- Get the context in the next position.
### writeBlock
- Write the given content to the file, if it is too large, expand it
### writeCurrentBlock
- Write the given content to the file, if it is too large, expand it.

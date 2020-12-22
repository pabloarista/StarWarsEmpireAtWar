#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "pmeg.h"
#include "pfile.h"
#include "plinked_list.h"
#include <wchar.h>
#include <string.h>
#include <assert.h>
/*
meg file definition taken from: https://www.tibed.net/editing/empireatwar/megafiles
The container file format used by Star Wars: Empire at War is called MegaFiles. It uses the file extension MEG or .MEG. Other container file formats are ZIP and RAR, for example. However, the MEG file format does not use compression; all files are stored uncompressed inside the MEG file. A game typically consists of thousands or tens of thousands files, and by placing them into a single file, the installation times of games can be decreased. Also, the loading times of games will decrease because a single big file will be stored in a consecutive area on the hard disk, instead of the random placement that could occur with thousands of small files.

Last update: March 22nd, 2006
Technical details

The file format consists of four parts: the MEG header, the filenames, the file information and the actual file data.
MEG Header

The MEG header contains the number of files in the MEG file. It is stored twice, and it appears they are always equal. However, the first refers to the number of filenames that will follow this record, and the second refers to the number of FileInfo records there are.

MEGHeader: record
    FilenameCount: unsigned int32;         {Number of filenames in the MEG file}
    FileInfoCount: unsigned int32;         {Number of file information records in the MEG file}
end;

Filenames

Immediately after the MEGHeader are filenames. There are FilenameCount consecutive filename records. These filenames is stored in an array called FilenameArray which starts at index 0, for the sake of this explanation.

FilenameRecord: record
    FilenameLength: unsigned int16;        {Number of characters in the filename (type also known as unsigned short)}
    Filename: String[1..FilenameLength];   {Actual filename: a string with length FilenameLength}
end;

File information

After the filenames is information about all the files. There are FileInfoCount consecutive FileInfo records.

FileInfo: record
    CRC32: unsigned int32;         { CRC32 checksum of the filename of this file }
    FileIndex: unsigned int32;     { Index of this file, counts from 0..FileInfoCount-1 }
    FileSize: unsigned int32;      { Size in bytes of this file }
    FileOffset: unsigned int32;    { Offset in the MEG where this file is stored }this file
    FilenameIndex: unsigned int32; { Index into the list of filenames (e.g. into FilenameArray) }
end;

Using the FilenameIndex field is crucial, because the filenames are stored in a different order from the FileInfo records!
Actual files

Following all the file information, the actual files are stored. These can be located by using the FileOffset and FileSize fields of the FileInfo record.
Example code

Example code in Python is available for parsing the MegaFiles format. Once you have Python installed on your machine, use the program by running "megafiles.py Cinematics.MEG list" to obtain a file list of the MEG file Cinematics.MEG. You can extract all files using "megafiles.py Cinematics.MEG dump". In order to keep the example simple, all names like DATA\FOO.XML will be converted into DATA_FOO.XML: the directory structure is not built, but instead put in to the filenames.

Example code: megafiles.py
Credits

Created by Koen van de Sande
Mike Lankamp deserves credit for thinking of CRC32 for the filename - this is actually very similar to the MIX files of the old C&C: Tiberian Dawn and Red Alert 1 MIX files, who only stored a checksum for the filename.
*/

static void destroy_str(void** val);
static struct plinked_list* split_path(char* path);
static bool did_create_path(struct plinked_list* paths, char* root_path);
static int pmeg_process_internal(char* full_path);
void DLL_EXPORT pmeg_process(char* full_path, int* code) {
    *code = pmeg_process_internal(full_path);
}
int pmeg_process_internal(char* full_path) {
    if(!full_path) return -1;

    struct pfile* pf = pfile_create(full_path);
    if(!pf) return -1;

    printf("opening file %s\n", (*pf).file_name);

    FILE* fp = fopen((*pf).file_name, "rb");
//TODO:check if correct file type by extension and contents

    if(!fp) {
        printf("error opening file %s\n", (*pf).file_name);
        perror(0);
        pfile_destroy(&pf);
        return -1;
    }//if

    (*pf).fp = fp;

    int seek_res = fseek(fp, 0, SEEK_END);
    long n;
    if(!seek_res) n = ftell(fp);
    else n = -1;
    if(n < 0 || ferror(fp)) {
        perror("error checking file size");
        pfile_destroy(&pf);
        return 0;
    }
    rewind(fp);
    printf("file size is:\t%ld\n", n);

    unsigned int pos = 0;
    unsigned int file_name_count;
    pos += sizeof(file_name_count);

    size_t bytes_read = fread(&file_name_count, sizeof(file_name_count), 1, fp);
    if(!bytes_read) {
        pfile_destroy(&pf);
        perror("error reading file!\n");
        return -1;
    }//if
    printf("number of files:\t%u\n", file_name_count);
    /*pos += sizeof(file_name_count);
    fseek(fp, pos, SEEK_SET);*/

    bool did_create_file_names = pfile_create_file_names(pf, file_name_count);
    if(!did_create_file_names) {
        pfile_destroy(&pf);
        perror("out of memory!\n");
        return -1;
    }//if

    unsigned int file_info_count;
    pos += sizeof(file_info_count);
    bytes_read = fread(&file_info_count, sizeof(file_info_count), 1, fp);
    if(!bytes_read) {
        pfile_destroy(&pf);
        perror("error reading file!\n");
        return -1;
    }//if
    printf("number of file information records:\t%u\n", file_info_count);

    unsigned int current = -1;

    while (++current < file_name_count) {
        unsigned short file_name_len;
        pos += sizeof(file_name_len);
        bytes_read = fread(&file_name_len, sizeof(file_name_len), 1, fp);
        if(!bytes_read) {
            pfile_destroy(&pf);
            perror("error reading file!\n");
            return -1;
        }//if
        //printf("current file name length:\t%u\n", file_name_len);

        char* file_name = (char*)malloc(sizeof *file_name * (file_name_len + 1));
        if(!file_name) {
            perror("out of memory!");
            pfile_destroy(&pf);
            return -1;
        }
        pos += file_name_len;
        bytes_read = fread(file_name, sizeof *file_name, file_name_len, fp);
        if(!bytes_read) {
            perror("error reading file!\n");
            free(file_name);
            pfile_destroy(&pf);
            return -1;
        }//if
        *(file_name + file_name_len) = 0;
        //printf("file name #%u\t%s\n", current, file_name);

        *((*pf).file_names + current) = file_name;
    }//while
/*
    int i;
    for (i = 0; i < 37; ++i) {
        printf("file name #%i\t%s\n", i + 1, *((*pf).file_names + i));
    }
*/
    current = 0;

    while (++current <= file_name_count) {
        unsigned int crc32;
        unsigned int file_index;
        unsigned int file_size;
        unsigned int file_offset;
        unsigned int file_name_index;
        const size_t SIZE = sizeof(unsigned int);

        if(!fread(&crc32, SIZE, 1, fp)
            || !fread(&file_index, SIZE, 1, fp)
            || !fread(&file_size, SIZE, 1, fp)
            || !fread(&file_offset, SIZE, 1, fp)
            || !fread(&file_name_index, SIZE, 1, fp)) {
            perror("error reading file!");
            pfile_destroy(&pf);
            return -1;
        }//if

        pos += SIZE * 5;
        char* file_name = *((*pf).file_names + file_name_index);

        printf("processing file #%d: %s (%u)\n", current, file_name, file_size);

        struct plinked_list* paths = split_path(file_name);

        if (plinked_list_is_empty(paths)) {
            plinked_list_destroy(&paths);
            pfile_destroy(&pf);
            printf("error splitting paths for %s\n", file_name);
            return -1;
        }//if
        bool did_create_paths = did_create_path(paths, (*pf).root_path);
        plinked_list_destroy(&paths);
        if (!did_create_paths) {
            pfile_destroy(&pf);
            return -1;
        }//if
        char* buffer = (char*)malloc(sizeof *buffer * (file_size + 1));
        if(!buffer) {
            pfile_destroy(&pf);
            perror(0);
            return -1;
        }//if
        rewind(fp);
        fseek(fp, file_offset, SEEK_SET);
        bytes_read = fread(buffer, sizeof *buffer, file_size, fp);

        int ferr = ferror(fp);
        int fend = feof(fp);
        bool read_eq = bytes_read == file_size;
        if(ferr || fend || !read_eq || !bytes_read) {
            if(ferr) puts("ferror!");
            if(fend) puts("end of file reached!");
            if(!read_eq) puts("read NOT equal!");
            if(!bytes_read) puts("NO bytes read!");
            free(buffer);
            pfile_destroy(&pf);
            perror(0);
            return -1;
        }//if
        *(buffer + file_size) = 0;
        size_t n_path = strlen((*pf).root_path) + strlen(file_name);
        char* path_write = (char*)malloc(sizeof *path_write * (n_path + 1));
        if(!path_write) {
            free(buffer);
            perror("out of memory!");
            pfile_destroy(&pf);
            return -1;
        }//if

        *path_write = 0;
        strcpy(path_write, (*pf).root_path);
        strcat(path_write, file_name);
        *(path_write + n_path) = 0;
/*
        printf("open to write file: %s (len=%d)\n", path_write, n_path);
*/
        FILE *fp_w = fopen(path_write, "wb");
        free(path_write);
        path_write = 0;
        size_t bytes_written;
        if(!fp_w) bytes_written = 0;
        else bytes_written = fwrite(buffer, sizeof *buffer, file_size, fp_w);
        if(!bytes_written) {
            free(buffer);
            pfile_destroy(&pf);
            if(!fp_w) {
                printf("error creating file %s\n", file_name);
                perror(0);
            }//if
            else perror("error write data to file");
            if(fp_w) fclose(fp_w);
            return -1;
        }//if
        fclose(fp_w);
        free(buffer);
        buffer = 0;
        rewind(fp);
        fseek(fp, pos, SEEK_SET);
    }//while

    pfile_destroy(&pf);
    puts("process completes without errors");

    return 0;
}
void destroy_str(void** val) {
    if(*val) {
        free(*val);
        *val = 0;
    }//if
}
struct plinked_list* split_path(char* path) {
    char* current = path;
    struct plinked_list* paths = 0;
    while(current) {
        char* tmp = strchr(current, '\\');
        if (tmp) {
            size_t n = tmp - current;
            assert(n > 0);
            char* slice = (char*)malloc(sizeof *slice * (n + 1));
            if(!slice) {
                plinked_list_destroy(&paths);
                return 0;
            }//if
            strncpy(slice, current, n);
            *(slice + n) = 0;

            struct pnode* node = pnode_create(slice, 0);
            if(!node) {
                free(slice);
                plinked_list_destroy(&paths);
                return 0;
            }//if
            if(!paths) {
                paths = plinked_list_create(node, destroy_str);
                if(!paths) {
                    pnode_destroy(&node, destroy_str);
                    return 0;
                }//if
            } else plinked_list_did_add_node(paths, node);
            current = tmp + 1;
        } else current = 0;
    }//while

    return paths;
}
bool did_create_path(struct plinked_list* paths, char* root_path) {
    if(plinked_list_is_empty(paths) || !root_path || !strcmp(root_path, "")) return false;

    ssize_t n = 0;
    struct pnode* c = (*paths).head;
    while(c) {
        char* val = (char*)(*c).val;
        n = n + strlen(val);
        c = (*c).next;
    }//while

    if(!n) return false;

    n = n + strlen(root_path);
    assert(n > 0);
    char* path = (char*)malloc(sizeof *path * (n + 1 + (*paths).count));
    if(!path) {
        perror(0);
        return false;
    }//if

    strncpy(path, root_path, n);

    while (!plinked_list_is_empty(paths)) {
        struct pnode* node = (*paths).head;
        if(node && (*node).val) {
            strcat(path, (char*)(*node).val);
            size_t len = strlen(path) + 1;
            assert(len > 0);
            wchar_t* dir = (wchar_t*)malloc(sizeof *dir * len);
            if(!dir) {
                free(path);
                perror("out of memory!");
                return false;
            }
//#pragma warning(suppress : 4996)
            mbstowcs(dir, path, len);
            unsigned long dir_attr = GetFileAttributesW(dir);

            bool dir_exists = dir_attr != INVALID_FILE_ATTRIBUTES && dir_attr & FILE_ATTRIBUTE_DIRECTORY;
            if (!dir_exists) {
                bool did_create = CreateDirectoryW(dir, 0);
                if (!did_create) {
                    unsigned long err = GetLastError();
                    if (err != ERROR_ALREADY_EXISTS) {
                        wprintf(L"error creating directory %s\n", dir);
                        free(dir);
                        free(path);
                        return false;
                    }//if
                }//if
            }//if

            free(dir);
        }//if
        bool did_remove = plinked_list_did_remove_head(paths);
        if(!did_remove) {
            free(path);
            return false;
        }//if

        strcat(path, "\\");
    }//while

    return true;
}
/*
another meg file definition taken from: https://modtools.petrolution.net/docs/MegFileFormat
Mega File Format
Introduction

Mega Files (extension: .MEG) in Petroglyph's games are used to store the collection of game files. By packing these files into a single large file, the operating-system overhead of storing and opening each individual file is removed and it helps avoid fragmentation.
Format #1

Each Mega File begins with a header, followed by the Filename Table, the File Table and finally, the file data. All fields are in little-endian format.


Header:
  +0000h  numFilenames  uint32   ; Number of filenames in the Filename Table
  +0004h  numFiles      uint32   ; Number of files in the File Table

Filename Table record:
  +0000h  length        uint16   ; Length of the filename, in characters
  +0004h  name          length   ; The ASCII filename

File Table record:
  +0000h  crc           uint32   ; CRC-32 of the filename
  +0004h  index         uint32   ; Index of this record in the table
  +0008h  size          uint32   ; Size of the file, in bytes
  +000Ch  start         uint32   ; Start of the file, in bytes , from the start of the Mega File
  +0010h  name          uint32   ; Index in the Filename Table of the filename

Format #2

This format is an extension on format #1 by adding extra fields in the header.

Each Mega File begins with a header, followed by the Filename Table, the File Table and finally, the file data. All fields are in little-endian format.


Header:
  +0000h  id1           uint32   ; Unknown field, always 0xFFFFFFFF
  +0004h  id2           uint32   ; Unknown field, always 0x3F7D70A4
  +0008h  dataStart     uint32   ; Offset in file of start of data
  +000Ch  numFilenames  uint32   ; Number of filenames in the Filename Table
  +0010h  numFiles      uint32   ; Number of files in the File Table

Filename Table record:
  +0000h  length        uint16   ; Length of the filename, in characters
  +0004h  name          length   ; The ASCII filename

File Table record:
  +0000h  crc           uint32   ; CRC-32 of the filename
  +0004h  index         uint32   ; Index of this record in the table
  +0008h  size          uint32   ; Size of the file, in bytes
  +000Ch  start         uint32   ; Start of the file, in bytes, from the start of the Mega File
  +0010h  name          uint32   ; Index in the Filename Table of the filename

Format #3

This format is an extension on format #2 by adding extra fields in the header and reformatting the file table record.

Each Mega File begins with a header, followed by the Filename Table, the File Table and finally, the file data. All fields are in little-endian format.

Mega Files in this format can be encrypted. This is indicated by the flags field in the header being 0x8FFFFFFF. Encrypted MegaFiles encrypt the following fragments:

    The Filename Table. This is encrypted as single blob of data.
    Each File Table record. Each record is individually encrypted, save for the flags field. If encrypted, a record's flags field will be 1, otherwise 0. Note that if the record is encrypted, its contents (after the flags field) is padded to 32 bytes.
    Every individual file's data is encrypted as a single blob of data. Note that each blob's size is rounded up to the next multiple of the AES block size.

Each encrypted fragment is encrypted using 128-bit AES in CBC mode, using the same key and IV. There is a section below listing the keys and IVs used by various games.


Header:
  +0000h  flags         uint32   ; Flags field, 0xFFFFFFFF or 0x8FFFFFFF (unencrypted file, encrypted file resp.)
  +0004h  id            uint32   ; Unknown field, always 0x3F7D70A4
  +0008h  dataStart     uint32   ; Offset in file of start of data
  +000Ch  numFilenames  uint32   ; Number of filenames in the Filename Table
  +0010h  numFiles      uint32   ; Number of files in the File Table
  +0014h  filenamesSize uint32   ; Size, in bytes, of the Filename Table

Filename Table record:
  +0000h  length        uint16   ; Length of the filename, in characters
  +0004h  name          length   ; The ASCII filename

File Table record:
  +0000h  flags         uint16   ; 1 if entry is encrypted, 0 otherwise.
  +0002h  crc           uint32   ; CRC-32 of the filename
  +0006h  index         uint32   ; Index of this record in the table
  +000Ah  size          uint32   ; Size of the file, in bytes
  +000Eh  start         uint32   ; Start of the file, in bytes, from the start of the Mega File
  +0012h  name          uint16   ; Index in the Filename Table of the filename
  +0014h  (padding)     14 bytes ; If this entry is encrypted, the record data (everything after the flags) is padded to 32 bytes

Notes

    The number of files and number of filenames are always equal (so far).
    The filenames are NOT zero-terminated.
    The File Table is sorted on the CRC (in ascending order).

Using multiple Mega Files

It is common or just possible for Petroglyph's games to have multiple Mega Files, and even Mega Files for user mods. All used Mega Files are listed in MegaFiles.xml in a game directory. Petroglyph's games read this file and, load all Mega Files and merge their File Table to create one Master File Table. If a file occurs in multiple Mega Files, the file in the Mega File listed last in MegaFiles.xml will be used.
Encryption/Decryption Keys

Some games using the Mega File format use the encryption capability offered with version 3. To be able to read and write such files, the AES key must be known. Listed below are the known keys for various games.

    Grey Goo:
    Key: 63223401b27efb502ec657b134a92561 (i.e. MD5("{CAF1CCE6-CC1D-40CE-AF2D-792BC446FD87}{83733AEE-CB3D-426E-BE9C-8CCB92E35B75}"))
    IV: 954fd996438b8fd035a5c7ffb6f6066b (i.e. MD5("Goo"))
    8-Bit Armies:
    Key: 1CB3FAFE676920DC6B12E15B232DAD6D (i.e. MD5("{CAF8CCE6-CC1D-40CE-AF2D-792BC4987D87}{83733AEE-CB3D-426E-B33C-8CC224E35B75}"))
    IV: 6CF6B9ABE7878212F5DFAEE6CF8A1E18 (i.e. MD5("SimpleRTS"))


*/

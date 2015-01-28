//
// this is ini v1.0.0
// 
// project home: https://github.com/ntessore/ini
// 
// ini is in the public domain
//

#pragma once

enum
{
    INI_SUCCESS = 0,
    INI_ERR_ALLOC,
    INI_ERR_FILE,
    INI_ERR_LINE_LEN,
    INI_ERR_GROUP,
    INI_ERR_VALUE
};

typedef struct
{
    char* name;
    char* value;
    int   line;
} ini_item;

typedef struct
{
    char*     name;
    int       line;
    int       size;
    ini_item* items;
} ini_group;

typedef struct
{
    char*      name;
    int        size;
    ini_group* groups;
} ini_file;

ini_file* ini_read(const char* file, int* err);
void ini_free(ini_file* ini);

int ini_errno(int err);
int ini_errline(int err);
const char* ini_strerror(int err);

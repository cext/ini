// 
// this is ini v1.0.1
// 
// project home: https://github.com/ntessore/ini
// 
// ini is in the public domain
// 

#ifndef INI_H_
#define INI_H_

// ini errno definitions
enum ini_err
{
    INI_SUCCESS = 0,    // no error
    INI_ERR_ALLOC,      // memory allocation error
    INI_ERR_FILE,       // file i/o error
    INI_ERR_LINE_LEN,   // line is too long
    INI_ERR_GROUP,      // error in group definition
    INI_ERR_VALUE       // missing value assignment
};

// item in ini file
typedef struct
{
    char* name;         // name of item
    char* value;        // value of item
    int   line;         // line where item was defined
} ini_item;

// group in ini file
typedef struct
{
    char*     name;     // name of group
    int       line;     // line where group was defined
    int       size;     // number of items
    ini_item* items;    // array of items
} ini_group;

// ini file
typedef struct
{
    char*      name;    // name of file
    int        size;    // number of groups
    ini_group* groups;  // array of groups
} ini_file;

// read ini file
ini_file* ini_read(const char* file, int* err);

// free allocated ini_file object
void ini_free(ini_file* ini);

// get error code of error
int ini_errno(int err);

// get line number of error
int ini_errline(int err);

// get string representation of error
const char* ini_strerror(int err);

#endif // INI_H_

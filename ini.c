// 
// this is ini v1.0.1
// 
// project home: https://github.com/ntessore/ini
// 
// ini is in the public domain
// 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "ini.h"

#ifndef LINE_LEN
#define LINE_LEN 1024
#endif

// whitespace characters
static const char* WS = " \t\n\v\f\r";

// assignment characters
static const char* ASSIGN = "=:";

// line-ending characters
static const char* EOL = ";#\n";

// trim characters from beginning of string
static void ltrim(char* str, const char* trim)
{
    size_t len = strlen(str);
    size_t beg = strspn(str, trim);
    if(beg > 0)
        for(size_t i = beg; i <= len; ++i)
            str[i-beg] = str[i];
}

// trim characters from end of string
static void rtrim(char* str, const char* trim)
{
    size_t len = strlen(str);
    while(len > 0 && strchr(trim, str[len-1]))
        str[--len] = '\0';
}

// split str using any of the characters spl, or return NULL if unsuccessful
static char* split(char* str, const char* spl)
{
    char* pos = str + strcspn(str, spl);
    if(*pos == '\0')
        return NULL;
    *pos = '\0';
    return pos + 1;
}

// copy a string to newly allocated memory
char* copystr(const char* str)
{
    char* cpy = malloc(strlen(str) + 1);
    if(cpy == NULL)
        return NULL;
    return strcpy(cpy, str);
}

int ini_errno(int err)
{
    return err & 0xff;
}

int ini_errline(int err)
{
    return err >> 8;
}

const char* ini_strerror(int err)
{
    int no = ini_errno(err);
    
    switch(no)
    {
    case INI_SUCCESS:
        return "success";
    case INI_ERR_ALLOC:
        return strerror(errno);
    case INI_ERR_FILE:
        return strerror(errno);
    case INI_ERR_LINE_LEN:
        return "line too long";
    case INI_ERR_GROUP:
        return "missing closing \"]\" character for group";
    case INI_ERR_VALUE:
        return "line does not assign a value";
    default:
        return "unknown error";
    }
}

ini_file* ini_read(const char* file, int* err)
{
    FILE* fp;
    char buf[LINE_LEN];
    int len, line;
    int eno;
    char* name;
    char* value;
    void* g;
    void* i;
    
    ini_file* ini;
    ini_group* grp;
    ini_item* itm;
    
    // start with first line
    line = 0;
    
    // start with no error
    eno = INI_SUCCESS;
    
    // try to open file
    fp = fopen(file, "r");
    if(!fp)
        { eno = INI_ERR_FILE; goto end; }
    
    // allocate ini data
    ini = malloc(sizeof(ini_file));
    if(ini == NULL)
        { eno = INI_ERR_ALLOC; goto end; }
    
    // initialize file
    ini->name = copystr(file);
    ini->size = 1;
    ini->groups = malloc(sizeof(ini_group));
    
    // make memory was allocated
    if(ini->name == NULL || ini->groups == NULL)
        { eno = INI_ERR_ALLOC; goto end; }
    
    // the initial group
    grp = ini->groups;
    
    // initialize group
    grp->name = NULL;
    grp->size = 0;
    grp->items = NULL;
    grp->line = 0;
    
    // read file line by line
    while(fgets(buf, sizeof(buf), fp))
    {
        // next line
        line += 1;
        
        // get length until end-of-line token
        len = strcspn(buf, EOL);
        
        // make sure a whole line was read (i.e. line ends with EOL character)
        if(buf[len] == '\0' && !feof(fp))
            { eno = INI_ERR_LINE_LEN; goto end; }
        
        // terminate string at EOL
        buf[len] = '\0';
        
        // trim whitespace from both sides of line
        rtrim(buf, WS);
        ltrim(buf, WS);
        
        // get current length of line
        len = strlen(buf);
        
        // skip empty lines
        if(buf[0] == '\0')
            continue;
        
        // check if line starts group
        if(buf[0] == '[')
        {
            // make sure group name is closed
            if(buf[len-1] != ']')
                { eno = INI_ERR_GROUP; goto end; }
            
            // group name starts after [ and ends before ]
            name = buf + 1;
            buf[len-1] = '\0';
            
            // trim whitespace from group name
            rtrim(name, WS);
            ltrim(name, WS);
            
             // add one group to file
            g = realloc(ini->groups, (ini->size + 1)*sizeof(ini_group));
            if(g == NULL)
                { eno = INI_ERR_ALLOC; goto end; }
            ini->groups = g;
            ini->size += 1;
            
            // point to new group
            grp = &ini->groups[ini->size - 1];
            
            // set up new group
            grp->name = copystr(name);
            grp->size = 0;
            grp->items = NULL;
            grp->line = line;
            
            // make sure allocations were ok
            if(grp->name == NULL)
                { eno = INI_ERR_ALLOC; goto end; }
        }
        else
        {
            // name starts at beginning of line
            name = buf;
            
            // split line into name and value using assignment characters
            value = split(buf, ASSIGN);
            
            // make sure a value was assigned
            if(!value)
                { eno = INI_ERR_VALUE; goto end; }
            
            // trim whitespace from name and value
            rtrim(name, WS);
            ltrim(value, WS);
            
            // add one item to group
            i = realloc(grp->items, (grp->size + 1)*sizeof(ini_item));
            if(i == NULL)
                { eno = INI_ERR_ALLOC; goto end; }
            grp->items = i;
            grp->size += 1;
            
            // point to new item
            itm = &grp->items[grp->size - 1];
            
            // set up new item
            itm->name = copystr(name);
            itm->value = copystr(value);
            itm->line = line;
            
            // make sure allocations were ok
            if(itm->name == NULL || itm->value == NULL)
                { eno = INI_ERR_ALLOC; goto end; }
        }
    }
    
    // check abort condition for errors
    if(ferror(fp))
        { eno = INI_ERR_FILE; goto end; }
    
    // ignore last group if empty
    if(ini->groups[ini->size - 1].size == 0)
        ini->size -= 1;
    
end:
    // try to close ini file
    if(fp)
        fclose(fp);
    
    // clean up data if error occurred
    if(eno != INI_SUCCESS)
    {
        ini_free(ini);
        ini = NULL;
    }
    
    // set error code if given
    if(err != NULL)
        *err = (eno == INI_SUCCESS) ? eno : (line << 8) | (eno & 0xff);
    
    return ini;
}

void ini_free(ini_file* ini)
{
    ini_group* grp;
    ini_item* itm;
    
    if(ini != NULL)
    {
        free(ini->name);
        for(grp = ini->groups; grp != ini->groups + ini->size; ++grp)
        {
            free(grp->name);
            for(itm = grp->items; itm != grp->items + grp->size; ++itm)
            {
                free(itm->name);
                free(itm->value);
            }
            free(grp->items);
        }
        free(ini->groups);
        free(ini);
    }
}

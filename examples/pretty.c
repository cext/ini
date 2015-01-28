//
// ini example: pretty
// 
// read ini file and print it back, with line numbers and cleaned up
//

#include <stdio.h>

#include "../ini.h"

int main(int argc, char* argv[])
{
    const char* file;
    int err;
    
    ini_file* ini;
    ini_group* grp;
    ini_item* itm;
    
    if(argc < 2)
    {
        fprintf(stderr, "usage: pretty <file>\n");
        return 1;
    }
    
    file = argv[1];
    
    ini = ini_read(file, &err);
    
    if(err != INI_SUCCESS)
    {
        int eno = ini_errno(err);
        int line = ini_errline(err);
        const char* str = ini_strerror(err);
        
        fprintf(stderr, "%s:", file);
        if(line > 0) fprintf(stderr, "%d:", line);
        fprintf(stderr, " error: %s\n", str);
        
        return eno;
    }
    
    printf("         | \n");
    for(grp = ini->groups; grp != ini->groups + ini->size; ++grp)
    {
        if(grp->name != NULL)
            printf("%8d | [%s]\n", grp->line, grp->name);
        for(itm = grp->items; itm != grp->items + grp->size; ++itm)
            printf("%8d | %s = %s\n", itm->line, itm->name, itm->value);
        printf("         | \n");
    }
    
    ini_free(ini);
    
    return 0;
}

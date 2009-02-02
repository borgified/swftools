/* main.c

   Flash ActionScript 3.0 compiler

   Part of the swftools package.

   Copyright (c) 2008 Matthias Kramm <kramm@quiss.org>
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "../rfxswf.h"
#include "../os.h"
#include "files.h"
#include "tokenizer.h"
#include "parser.tab.h"
#include "parser.h"
#include "compiler.h"

void test_lexer(char*filename)
{
    char*fullfilename = enter_file(filename, 0);
    FILE*fi = fopen(fullfilename, "rb");
    as3_set_in(fi);
    while(1) {
        int token = as3_lex();
        if(token==T_EOF)
            break;
        if(token>=32 && token<256) {
            printf("'%c'\n", token);
        } else {
            printf("%s\n", token2string(token, a3_lval));
        }
    }
}

int main(int argn, char*argv[])
{
    char*filename = 0;
    char buf[512];
    
    if(argn<=1) {
        fprintf(stderr, "please supply a filename\n");
        exit(1);
    }
    filename=argv[1];
    
    if(argn>2 && !strcmp(argv[2], "-lex")) {
        test_lexer(filename);
        return 0;
    }

    //extern int avm2_debug;
    //avm2_debug = 1;
    
    as3_add_include_dir(getcwd(buf, 512));
    as3_parse_file(filename);
    void*code = as3_getcode();

    SWF swf;
    memset(&swf, 0, sizeof(swf));
    swf.fileVersion = 9;
    swf.frameRate = 0x2500;
    swf.movieSize.xmin = swf.movieSize.ymin = 0;
    swf.movieSize.xmax = 20*20;
    swf.movieSize.ymax = 10*20;
    TAG*tag = swf.firstTag = swf_InsertTag(0, ST_DOABC);
    swf_WriteABC(tag, code);

    if(as3_getglobalclass()) {
        tag = swf_InsertTag(tag, ST_SYMBOLCLASS);
        swf_SetU16(tag, 1);
        swf_SetU16(tag, 0);
        swf_SetString(tag, as3_getglobalclass());
    } else {
        printf("Warning: no global public MovieClip subclass\n");
    }

    tag = swf_InsertTag(tag, ST_SHOWFRAME);
    tag = swf_InsertTag(tag, ST_END);

    swf_FreeABC(code);

    int f = open("abc.swf",O_RDWR|O_CREAT|O_TRUNC|O_BINARY,0644);
    swf_WriteSWF(f,&swf);
    close(f);

    swf_FreeTags(&swf);

    return 0;
}

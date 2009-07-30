/* ScummC
 * Copyright (C) 2008  Alban Bedel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/**
 * @file scvm_string.c
 * @ingroup scvm
 * @brief SCVM strings
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "scc_fd.h"
#include "scc_util.h"
#include "scc_cost.h"
#include "scc_box.h"
#include "scc_char.h"
#include "scvm_res.h"
#include "scvm_thread.h"
#include "scvm.h"


int scvm_getc(unsigned char* str, unsigned* c, int* arg) {
    int len = 0;
    if(!str[0]) return 0;
    if(c) *c = str[0];
    if(str[0] == SCVM_CHAR_ESCAPE) {
        int type = str[1];
        if((type < 1 || type > 3) && type != 8) {
            if(arg) arg[1] = SCC_GET_16LE(str,2);
            len += 2;
        }
        if(arg) arg[0] = type;
        len += 1;
    }
    return len+1;
}

int scvm_strlen(unsigned char* str) {
    int len = 0, r;
    while((r = scvm_getc(str,NULL,NULL)) > 0)
        len += r;
    return len;
}

int scvm_init_string_dc(scvm_t* vm,scvm_string_dc_t* dc,unsigned chset_no) {
    scc_charmap_t* chset = scvm_load_res(vm,SCVM_RES_CHARSET,chset_no);
    if(!chset) return -1;
    memset(dc,0,sizeof(*dc));
    dc->chset = chset;
    memcpy(dc->pal,dc->chset->pal,sizeof(dc->pal));
    return 0;
}

void scvm_string_dc_copy(scvm_string_dc_t* to, const scvm_string_dc_t* from) {
    memcpy(to,from,sizeof(*to));
}


int scvm_string_get_escape_size(scvm_t* vm, scvm_string_dc_t* dc,
                                unsigned esc, int arg,
                                unsigned* p_width, unsigned* p_height) {
    unsigned w = 0, h = 0;
    int val;
    unsigned char tmp_str[64];
    unsigned char* sub_str = NULL;
    scvm_verb_t* verb;

    switch(esc) {
    case SCVM_CHAR_NEW_LINE:
    case SCVM_CHAR_KEEP:
    case SCVM_CHAR_WAIT:
        break;
    case SCVM_CHAR_INT_VAR:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        snprintf((char*)tmp_str,sizeof(tmp_str),"%d",val);
        sub_str = tmp_str;
        break;
    case SCVM_CHAR_VERB:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        if((verb = scvm_get_verb(vm,val,0)))
            sub_str = verb->name;
        break;
    case SCVM_CHAR_NAME:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        if(val <= 0xF) {
            if(val < vm->num_actor)
                sub_str = vm->actor[val].name;
        } else
            scvm_get_object_name(vm,val,&sub_str);
        break;
    case SCVM_CHAR_STRING_VAR:
        sub_str = scvm_thread_get_string_var(vm,vm->current_thread,arg);
        break;
    }

    if(sub_str && scvm_string_get_size(vm,dc,sub_str,&w,&h) < 0)
        return -1;

    if(p_width)  *p_width  = w;
    if(p_height) *p_height = h;
    return 0;
}

int scvm_string_get_char_size(scvm_t* vm, scvm_string_dc_t* dc,
                              unsigned c, int* arg,
                              unsigned* p_width, unsigned* p_height) {
    scc_char_t *ch;
    if(c == SCVM_CHAR_ESCAPE)
        return scvm_string_get_escape_size(vm,dc,arg[0],arg[1],
                                           p_width,p_height);

    if(c > dc->chset->max_char) return -1;
    ch = &dc->chset->chars[c];
    if(p_width)  *p_width  = ch->x + ch->w;
    if(p_height) *p_height = ch->y + ch->h;
    return 0;
}

int scvm_string_get_line_size(scvm_t* vm, scvm_string_dc_t* dc,
                              unsigned char* str,
                              unsigned* p_width, unsigned* p_height) {
    unsigned c, line_w = 0, line_h = 0;
    int pos = 0, r, arg[SCVM_CHAR_MAX_ARGS];

    while((r = scvm_getc(str+pos,&c,arg)) > 0) {
        unsigned cw = 0, ch = 0;
        if(c == SCVM_CHAR_ESCAPE &&
           (arg[0] == SCVM_CHAR_NEW_LINE ||
            arg[0] == SCVM_CHAR_KEEP ||
            arg[0] == SCVM_CHAR_WAIT))
            break;
        if(!scvm_string_get_char_size(vm,dc,c,arg,&cw,&ch)) {
            line_w += cw;
            if(ch > line_h) line_h = ch;
        }
        pos += r;
    }

    if(p_width)  *p_width  = line_w;
    if(p_height) *p_height = line_h;
    return pos;
}

int scvm_string_get_size(scvm_t* vm, scvm_string_dc_t* dc,
                         unsigned char* str,
                         unsigned* p_width, unsigned* p_height) {
    unsigned line_w, line_h, str_w = 0, str_h = 0;
    int pos = 0, r;

    while(1) {
        r = scvm_string_get_line_size(vm,dc,str+pos,&line_w,&line_h);
        if (r <  0) return r;
        pos += r;

        if(line_w > str_w) str_w = line_w;

        if(str[pos] == SCVM_CHAR_ESCAPE && str[pos+1] == SCVM_CHAR_NEW_LINE) {
            str_h += dc->chset->height;
            pos   += 2;
            continue;
        }

        str_h += line_h;
        break;
    }

    if(p_width)  *p_width  = str_w;
    if(p_height) *p_height = str_h;
    return pos;
}

int scvm_string_dc_draw(scvm_string_dc_t* dc, unsigned c,
                        uint8_t* dst, int dst_stride,
                        int dx, int dy, int clip_w, int clip_h) {
    scc_char_t* ch;
    uint8_t* src;
    unsigned x, y;

    if(c > dc->chset->max_char) return -1;
    ch  = dc->chset->chars+c;
    src = ch->data;
    dx += dc->pen_x + ch->x;
    dy += dc->pen_y + ch->y;
    if(dy >= 0)
        dst += dy*dst_stride;
    else
        src += -dy*ch->w;
    for(y = dy < 0 ? -dy : 0 ; y < ch->h && y+dy < clip_h ; y++) {
        for(x = dx < 0 ? -dx : 0 ; x < ch->w && x+dx < clip_w ; x++) {
            unsigned color = src[x];
            if(!color || color >= sizeof(dc->pal)) continue;
            dst[dx+x] = dc->pal[color-1];
        }
        dst += dst_stride;
        src += ch->w;
    }

    return 0;
}

int scvm_string_draw_escape(scvm_t* vm, scvm_string_dc_t* dc,
                            unsigned esc, int arg,
                            uint8_t* dst, int dst_stride,
                            int dx, int dy, int clip_w, int clip_h) {
    int val;
    char tmp_str[64];
    unsigned char* sub_str = NULL;
    scvm_verb_t* verb;

    switch(esc) {
    case SCVM_CHAR_NEW_LINE:
    case SCVM_CHAR_KEEP:
    case SCVM_CHAR_WAIT:
        break;
    case SCVM_CHAR_INT_VAR:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        snprintf(tmp_str,sizeof(tmp_str),"%d",val);
        sub_str = tmp_str;
        break;
    case SCVM_CHAR_VERB:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        if((verb = scvm_get_verb(vm,val,0)))
            sub_str = verb->name;
        break;
    case SCVM_CHAR_NAME:
        if(scvm_thread_read_var(vm,vm->current_thread,arg,&val))
            break;
        if(val <= 0xF) {
            if(val < vm->num_actor)
                sub_str = vm->actor[val].name;
        } else
            scvm_get_object_name(vm,val,&sub_str);
        break;
    case SCVM_CHAR_STRING_VAR:
        sub_str = scvm_thread_get_string_var(vm,vm->current_thread,arg);
        break;
    }

    if(sub_str)
        return scvm_string_draw(vm,dc,sub_str,dst,dst_stride,
                                dx,dy,clip_w,clip_h);

    return 0;
}

int scvm_string_draw_char(scvm_t* vm, scvm_string_dc_t* dc,
                          unsigned c, int* arg,
                          uint8_t* dst, int dst_stride,
                          int dx, int dy, int clip_w, int clip_h) {
    if(c == SCVM_CHAR_ESCAPE)
        return scvm_string_draw_escape(vm,dc,arg[0],arg[1],dst,dst_stride,
                                       dx,dy,clip_w,clip_h);

    if(scvm_string_dc_draw(dc,c,dst,dst_stride,dx,dy,clip_w,clip_h))
        return -1;

    dc->pen_x += dc->chset->chars[c].x+dc->chset->chars[c].w;
    return 0;
}

int scvm_string_draw_line(scvm_t* vm, scvm_string_dc_t* dc,
                          unsigned char* str,
                          uint8_t* dst, int dst_stride,
                          int dx, int dy, int clip_w, int clip_h) {

    unsigned c;
    int pos = 0, r, arg[SCVM_CHAR_MAX_ARGS];

    if(dc->flags & SCVM_STRING_CENTER) {
        unsigned line_w = 0;
        scvm_string_dc_t tmp_dc;
        memcpy(&tmp_dc,dc,sizeof(*dc));
        if((r = scvm_string_get_line_size(vm,&tmp_dc,str,&line_w,NULL)) < 0)
            return r;
        dc->pen_x -= line_w/2;
    }

    while((r = scvm_getc(str+pos,&c,arg)) > 0) {
        if(c == SCVM_CHAR_ESCAPE &&
           (arg[0] == SCVM_CHAR_KEEP ||
            arg[0] == SCVM_CHAR_WAIT ||
            arg[0] == SCVM_CHAR_NEW_LINE))
                break;
        scvm_string_draw_char(vm,dc,c,arg,dst,dst_stride,dx,dy,clip_w,clip_h);
        pos += r;
    }

    return pos;
}

int scvm_string_draw(scvm_t* vm, scvm_string_dc_t* dc,
                     unsigned char* str,
                     uint8_t* dst, int dst_stride,
                     int dx, int dy, int clip_w, int clip_h) {
    int r, pos = 0;
    int start_pen_x = dc->pen_x;

    while(1) {
        r = scvm_string_draw_line(vm,dc,str+pos,dst,dst_stride,
                                  dx,dx,clip_w,clip_h);
        if(r < 0) return r;
        pos += r;

        if(str[pos] == SCVM_CHAR_ESCAPE && str[pos+1] == SCVM_CHAR_NEW_LINE) {
            dc->pen_x  = start_pen_x;
            dc->pen_y += dc->chset->height;
            pos       += 2;
            continue;
        }
        break;
    }

    return pos;
}
/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
 *                                         <sebastian.fatjo@ridgerun.com>
 *
 * This file is part of GstShark.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>
#include <stdlib.h>

#include "gstparser.h"
#include "gstsharkclient.h"

extern trace_information * trace_inf;

typedef struct
{
    gchar * next_location;
    gchar * line_end;
    
    const parser_handler_desc * parser_desc_list;
    gint parser_desc_list_len;
    parser_handler_function no_match_handler;
    
} parser_info;

typedef enum {
    FILE_PROTOCOL,
    TCP_PROTOCOL,
    MAX_PROTOCOL
} protocol_type;


parser_info parser_mem;
parser_info * parser = &parser_mem;

static gchar * protocol_list[] = {
    [FILE_PROTOCOL] = "file://",
    [TCP_PROTOCOL]  = "tcp://",
};


static gboolean parse_strcmp(const gchar * ref, gchar ** cmp_string)
{
    gchar* string;

    string = *cmp_string;

    while (*ref == *string && '\0' != *ref)
    {
        ref++;
        string++;
    }
    /* Verify if the loop reaches the null character */
    if ('\0' == *ref)
    {
        *cmp_string = string;
        return TRUE;
    }
    return FALSE;
}

static gboolean parser_get_protocol(protocol_type * type, gchar ** line)
{
    gint protocol_type_idx;
    gboolean cmp_res;
    gchar* string = *line;

    for (protocol_type_idx = 0; protocol_type_idx < MAX_PROTOCOL; ++protocol_type_idx)
    {
        cmp_res = parse_strcmp(protocol_list[protocol_type_idx],&string);
        if (TRUE == cmp_res)
        {
            *line = string;
            *type = protocol_type_idx;
            return TRUE;
        }
    }
    return FALSE;
}
#if 1
void tcp_parser_handler(gchar * line)
{
    gchar * line_end;
    gchar * host_name;
    gchar * port_name;
    gsize str_len;

    host_name = line;
    line_end = line;
    while (('\0' != *line_end) &&
        (':' != *line_end))
    {
        ++line_end;
    }

    if (*line_end == '\0')
    {
        str_len = strlen(host_name);

        trace_inf->host_name = g_malloc(str_len + 1);

        strcpy(trace_inf->host_name,host_name);
        /* End of the line, finish parser process */
        return;
    }
    if (*line_end == ':')
    {
        /* Get the port value */
        *line_end = '\0';

        str_len = strlen(host_name);

        trace_inf->host_name = g_malloc(str_len + 1);

        strcpy(trace_inf->host_name,host_name);

        ++line_end;
        port_name = line_end;
        //while ('\0' != *line_end)
        //{
            //++line_end;
        //}

        //if (*line_end == '\0')
        //{
            /* TODO: verify if is a numeric string */
            trace_inf->port_number = atoi(port_name);
            /* End of the line, finish parser process */
            //~ end_of_line = TRUE;
            return;
        //}
        /* if *line_end == ';' */
        //*line_end = '\0';
        //trace_inf->port_number = atoi(port_name);
        //line = line_end + 1;
    }
    /* if *line_end == ';' */
    //*line_end = '\0';

    //str_len = strlen(host_name);
    //trace_inf->host_name = g_malloc(str_len + 1);
    //strcpy(trace_inf->host_name,host_name);
    //line = line_end + 1;
}
#endif
void file_parser_handler(gchar * line)
{
    gsize  str_len;

    str_len = strlen(line);
    trace_inf->dir_name = g_malloc(str_len + 1);
    strcpy(trace_inf->dir_name,line);
}

void parser_register_callbacks(
    const parser_handler_desc * parser_handler_desc_list,
    guint list_len,
    parser_handler_function no_match_handler)
{
    parser->parser_desc_list = parser_handler_desc_list;
    parser->parser_desc_list_len = list_len;
    parser->no_match_handler = no_match_handler;
}


void parse_option(gchar * line)
{

    protocol_type type;
    gboolean parser_prot_res;
    gchar * line_end;
    gchar * next_location;
    guint str_len;
    
    /* Compute the end of the line */
    str_len = strlen(line);
    
    line_end = line + str_len;

    /* Search next location */
    next_location = line;
    while ((next_location != line_end) && (';' != *next_location))
    {
        ++next_location;
    }

    if (';' == *next_location)
    {
        *next_location = '\0';
        next_location++;
    }
    else
    {
        next_location = NULL;
    }
    
    
    do
    {
        parser_prot_res = parser_get_protocol(&type, &line);

        if(FALSE == parser_prot_res)
        {
            /* TODO */
        }

        switch (type)
        {
            case FILE_PROTOCOL:
                file_parser_handler(line);

                break;
            case TCP_PROTOCOL:
                tcp_parser_handler(line);

                break;
            default:
                break;
        }
        line = next_location;

        if (next_location == NULL)
        {
            break;
        }

        while ((next_location != line_end) && (';' != *next_location))
        {
            ++next_location;
        }

        if (';' == *next_location)
        {
            *next_location = '\0';
            next_location++;
        }
        else
        {
            next_location = NULL;
        }
    } while (NULL == next_location);
}

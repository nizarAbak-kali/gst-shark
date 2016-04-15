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


parser_info parser_mem;
parser_info * parser = &parser_mem;


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

void parser_register_callbacks(
    const parser_handler_desc * parser_handler_desc_list,
    guint list_len,
    parser_handler_function no_match_handler)
{
    parser->parser_desc_list = parser_handler_desc_list;
    parser->parser_desc_list_len = list_len;
    parser->no_match_handler = no_match_handler;
}


void parser_line(gchar * line)
{
    gboolean cmp_res;
    gchar * line_end;
    gchar * next_location;
    guint str_len;
    guint list_idx;

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
        for (list_idx = 0; list_idx < parser->parser_desc_list_len; ++list_idx)
        {
            cmp_res = parse_strcmp(parser->parser_desc_list[list_idx].location,&line);
            if (TRUE == cmp_res)
            {
                parser->parser_desc_list[list_idx].parser_handler(line);

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
            }
        }
        /* TODO: if location is not defined */
    } while (line != NULL);
}

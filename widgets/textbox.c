/*
 * textbox.c - text box widget
 *
 * Copyright © 2007 Aldo Cortesi <aldo@nullcube.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "util.h"
#include "widget.h"
#include "xutil.h"
#include "screen.h"

extern AwesomeConf globalconf;

typedef struct
{
    char *text;
    int width;
    Alignment align;
    XColor fg;
    XColor bg;
} Data;

static int
textbox_draw(Widget *widget, DrawCtx *ctx, int offset,
             int used __attribute__ ((unused)))
{
    Data *d = widget->data;

    if(d->width)
        widget->area.width = d->width;
    else
        widget->area.width = draw_textwidth(widget->font, d->text);

    widget->area.height = widget->statusbar->height;

    if(!widget->user_supplied_x)
        widget->area.x = widget_calculate_offset(widget->statusbar->width,
                                                 widget->area.width,
                                                 offset,
                                                 widget->alignment);
    if(!widget->user_supplied_y)
        widget->area.y = 0;

    draw_text(ctx, widget->area.x, widget->area.y, widget->area.width,
              widget->statusbar->height, d->align, 0, widget->font,
              d->text, d->fg, d->bg);

    return widget->area.width;
}

static void
textbox_tell(Widget *widget, char *command)
{
    char *tok;
    int i, color;
    ssize_t command_len = a_strlen(command) + 1;
    char* text = p_new(char, command_len);

    Data *d = widget->data;
    if (d->text)
        p_delete(&d->text);

    for(tok = strtok(command, " "), i = color = 0; tok; tok = strtok(NULL, " "), i++)
    {
        if(*tok == '#' && i < 2)
        {
            switch(i)
            {
                case 0:
                    d->fg = initxcolor(get_phys_screen(widget->statusbar->screen),
                                       tok);
                    break;
                case 1:
                    d->bg = initxcolor(get_phys_screen(widget->statusbar->screen),
                                       tok);
                    break;
            };
            color++;
        }
        else
        {
            if(i > color)
                a_strcat(text, command_len, " ");
            a_strcat(text, command_len, tok);
        }
    }

    d->text = a_strdup(text);
    p_delete(&text);
}

Widget *
textbox_new(Statusbar *statusbar, cfg_t *config)
{
    Widget *w;
    Data *d;
    char *buf;

    w = p_new(Widget, 1);
    widget_common_new(w, statusbar, config);
    w->draw = textbox_draw;
    w->tell = textbox_tell;

    w->data = d = p_new(Data, 1);

    if((buf = cfg_getstr(config, "fg")))
        d->fg = initxcolor(statusbar->screen, buf);
    else
        d->fg = globalconf.screens[statusbar->screen].colors_normal[ColFG];

    if((buf = cfg_getstr(config, "bg")))
        d->bg = initxcolor(get_phys_screen(statusbar->screen), buf);
    else
        d->bg = globalconf.screens[statusbar->screen].colors_normal[ColBG];

    d->width = cfg_getint(config, "width");
    d->align = draw_get_align(cfg_getstr(config, "align"));

    if((buf = cfg_getstr(config, "font")))
        w->font = XftFontOpenName(globalconf.display, get_phys_screen(statusbar->screen), buf);

    if(!w->font)
        w->font = globalconf.screens[statusbar->screen].font;

    d->text = a_strdup(cfg_getstr(config, "text"));
    return w;
}
// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80

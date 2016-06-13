/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *  Copyright (C) 2014-2015 - Jean-André Santoni
 *  Copyright (C) 2016      - Andrés Suárez
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nk_menu.h"

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <file/file_path.h>
#include <string/stdstring.h>
#include <lists/string_list.h>
#include <lists/dir_list.h>
#include <playlist.h>

#include "../../menu_driver.h"
#include "../../menu_hash.h"

static float ratio[] = {0.25f, 0.75f};

static bool playlist_icons_loaded;

static struct string_list *files  = NULL;
static playlist_t *tmp_playlist = NULL;

struct nk_image playlist_icons[100];

static void load_playlist_icons(nk_menu_handle_t *nk, const char* icon, unsigned index)
{
   char path[PATH_MAX_LENGTH] = {0};
   char buf[PATH_MAX_LENGTH] = {0};

   fill_pathname_application_special(path, sizeof(path),
         APPLICATION_SPECIAL_DIRECTORY_ASSETS_NUKLEAR_ICONS);

   fill_pathname_join(buf, path, icon, sizeof(buf));
   strlcat(buf, ".png", sizeof(buf));
   playlist_icons[index] = nk_common_image_load(buf);
}

void nk_wnd_library(nk_menu_handle_t *nk, const char* title, unsigned width, unsigned height)
{
   unsigned i;
   static char buf[PATH_MAX_LENGTH];
   static struct nk_panel left_col;
   static struct nk_panel right_col;
   struct nk_panel layout;

   struct nk_context *ctx = &nk->ctx;
   const int id  = NK_WND_LIBRARY;
   settings_t *settings = config_get_ptr();

   struct nk_style *style;
   struct nk_style_button *button;
   struct nk_style_window *window;
   style = &ctx->style;
   button = &style->button;
   window = &style->window;

   if (!files)
      files = dir_list_new(settings->directory.playlist, "lpl", true, true);

   window->padding = nk_vec2(0,0);
   window->spacing = nk_vec2(0,0);
   window->group_border_color = nk_rgba(62,62,62,255);
   window->fixed_background = nk_style_item_color(nk_rgba(46,47,47,255));
   button->border = 0.0f;

   if (nk_begin(ctx, &layout, title, nk_rect(0, 0, width, height),
         NK_WINDOW_NO_SCROLLBAR))
   {
      window->padding = nk_vec2(0,0);
      window->spacing = nk_vec2(0,0);
      window->fixed_background = nk_style_item_color(nk_rgba(67,67,67,255));
      button->normal = nk_style_item_color(nk_rgba(67,67,67,255));
      button->hover = nk_style_item_color(nk_rgba(92,91,223,255));
      button->border_color = nk_rgba(67,67,67,255);
      button->text_normal = nk_rgba(230,229,229,255);
      button->text_hover = nk_rgba(255,255,255,255);

      nk_layout_row(ctx, NK_DYNAMIC, height, 2, ratio);

      nk_group_begin(ctx, &left_col, "Playlists", NK_WINDOW_NO_SCROLLBAR);
      {
         nk_layout_row_dynamic(ctx, 24, 1);

         for (i = 0; i < files->size; i++)
         {
            strlcpy(buf, files->elems[i].data, sizeof(buf));
            path_remove_extension(buf);
            if (!playlist_icons_loaded || nk->context_reset)
               load_playlist_icons(nk, path_basename(buf), i);
            if (nk_button_image_label(ctx, playlist_icons[i], path_basename(buf),
               NK_TEXT_CENTERED, NK_BUTTON_DEFAULT))
               tmp_playlist = playlist_init(files->elems[i].data, 100);;
         }
         playlist_icons_loaded = true;
         nk_group_end(ctx);
      }

      window->fixed_background = nk_style_item_color(nk_rgba(62,62,62,255));
      button->normal = nk_style_item_color(nk_rgba(62,62,62,255));
      button->border_color = nk_rgba(62,62,62,255) ;

      nk_group_begin(ctx, &right_col, "Content", 0);
      {
         window->padding = nk_vec2(8,8);
         window->spacing = nk_vec2(4,4);

         nk_layout_row_dynamic(ctx, 128, 2);
         for (i = 0; i < playlist_size(tmp_playlist); i++)
         {
            const char *entry_path;
            const char *entry_label;
            playlist_get_index(tmp_playlist, i, &entry_path, &entry_label, NULL, NULL, NULL, NULL);
            if (nk_button_image_label(ctx, nk->icons.file, entry_label, NK_TEXT_CENTERED, NK_BUTTON_DEFAULT))
            {
               RARCH_LOG ("do stuff\n");
            }
         }
         nk_group_end(ctx);
      }
   }

   /* save position and size to restore after context reset */
   nk_wnd_set_state(nk, id, nk_window_get_position(ctx), nk_window_get_size(ctx));
   nk_end(ctx);
}

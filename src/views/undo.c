/*
    This file is part of darktable,
    copyright (c) 2009--2010 johannes hanika.
    copyright (c) 2012 tobias ellinghaus.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "views/undo.h"
#include <glib.h>

typedef struct dt_undo_item_t
{
  dt_view_t *view;
  dt_undo_type_t type;
  dt_undo_data_t *data;
  void (*undo) (dt_view_t *view, dt_undo_type_t type, dt_undo_data_t *data);
} dt_undo_item_t;

dt_undo_t *dt_undo_init(void)
{
  dt_undo_t * udata = malloc(sizeof (dt_undo_t));
  udata->undo_list = NULL;
  udata->redo_list = NULL;
  return udata;
}

void dt_undo_cleanup(void)
{
  dt_undo_clear(darktable.undo, DT_UNDO_ALL);
}

void dt_undo_record(dt_undo_t *udata, dt_view_t *view, dt_undo_type_t type, dt_undo_data_t *data, void (*undo) (dt_view_t *view, dt_undo_type_t type, dt_undo_data_t *item))
{
  dt_undo_item_t *item = g_malloc(sizeof (dt_undo_item_t));

  item->view = view;
  item->type = type;
  item->data = data;
  item->undo = undo;

  udata->undo_list = g_list_prepend(udata->undo_list, (gpointer)item);

  // recording an undo data invalidate all the redo
  g_list_free_full(udata->redo_list,&g_free);
  udata->redo_list = NULL;
}

void dt_undo_do_redo(dt_undo_t *udata, uint32_t filter)
{
  GList *l = g_list_first(udata->redo_list);

  // check for first item that is matching the given pattern

  while (l)
  {
    dt_undo_item_t *item = (dt_undo_item_t*)l->data;
    if (item->type & filter)
    {
      //  first remove element from _redo_list
      udata->redo_list = g_list_remove(udata->redo_list, item);

      //  callback with redo data
      item->undo(item->view, item->type, item->data);

      //  add old position back into the undo list
      udata->undo_list = g_list_prepend(udata->undo_list, item);
      break;
    }
    l=g_list_next(l);
  };
}

void dt_undo_do_undo(dt_undo_t *udata, uint32_t filter)
{
  GList *l = g_list_first(udata->undo_list);

  // check for first item that is matching the given pattern

  while (l)
  {
    dt_undo_item_t *item = (dt_undo_item_t*)l->data;
    if (item->type & filter)
    {
      //  first remove element from _undo_list
      udata->undo_list = g_list_remove(udata->undo_list, item);

      //  callback with undo data
      item->undo(item->view, item->type, item->data);

      //  add element into the redo list as filed with our previous position (before undo)

      udata->redo_list = g_list_prepend(udata->redo_list, item);
      break;
    }
    l=g_list_next(l);
  };
}

static void dt_undo_clear_list(GList **list, uint32_t filter)
{
  GList *l = g_list_first(*list);

  // check for first item that is matching the given pattern

  while (l)
  {
    dt_undo_item_t *item = (dt_undo_item_t*)l->data;
    if (item->type & filter)
    {
      //  remove this element
      g_free(item->data);
      *list = g_list_remove(*list, item);
    }
    l=g_list_next(l);
  };
}

void dt_undo_clear(dt_undo_t *udata, uint32_t filter)
{
  dt_undo_clear_list(&udata->undo_list, filter);
  dt_undo_clear_list(&udata->redo_list, filter);
  udata->undo_list = NULL;
  udata->redo_list = NULL;
}

// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.sh
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-space on;

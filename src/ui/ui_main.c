/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "player.h"
#include "setting.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>

static int ui_poll(GtkWidget *widget);

static GtkWidget *main_win;

static struct player_music_info *old_info;

static pthread_mutex_t poll_mutex = PTHREAD_MUTEX_INITIALIZER;

void ui_load()
{
	GtkWidget *window;
	GtkWidget *fixed;
	GtkWidget *cover_container;

	struct ui_skin *a_skin;

	gtk_init(NULL, NULL);

	a_skin = ui_get_current_skin();

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_app_paintable(window, TRUE);

	/* remove the window decorate */
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

	gtk_window_set_default_size(GTK_WINDOW(window), a_skin->width, a_skin->height);

	/* remove the taskbar */
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

	gtk_window_set_keep_below(GTK_WINDOW(window), TRUE);

	gtk_widget_set_uposition(window, cfg_get_pos_x(), cfg_get_pos_y());

	fixed = gtk_fixed_new();

	gtk_container_add(GTK_CONTAINER(window), fixed);

	cover_container = gtk_event_box_new();

	gtk_event_box_set_visible_window(GTK_EVENT_BOX(cover_container), FALSE);
	
	/* init cover skin */
	ui_cover_init(fixed, cover_container);

	/* init player control skin */
	ui_player_control_init(fixed);

	/* init label skin */
	ui_player_info_init(fixed);

	ui_menu_init(cover_container);

	ui_set_transparent(window, NULL, NULL);

	ui_update(NULL);

	g_timeout_add(1000, (GSourceFunc)ui_poll, NULL);

	ui_poll(NULL);

	gtk_widget_show_all(window);

	main_win = window;

	gtk_main();
}


void ui_update(const char *skin_name)
{
	struct player_music_info *info;

	info = player_get_music_info();

	/* change skin */
	if(skin_name)
	{
		/* destroy current window */
		gtk_widget_destroy(main_win);

		/* exit gtk loop */
		gtk_main_quit();

		/* reload skin */
		ui_skin_load(skin_name);

		/* draw window */
		ui_load();
	}

	/* refresh control */
	ui_player_control_update();

	/* refresh ui info */
	ui_player_info_update(info);

	/* refresh cover */
	ui_cover_update(info);
	//player_music_info_free(info);
	
	old_info = info;
}

static int ui_poll(GtkWidget *widget)
{
	struct player_music_info *new_info;

	pthread_mutex_lock(&poll_mutex);

	new_info = player_get_music_info();

	if(old_info == NULL)
	{
		old_info = new_info;

		pthread_mutex_unlock(&poll_mutex);

		return TRUE;
	}

	if(new_info->id == old_info->id)
	{
		player_music_info_free(new_info);

		pthread_mutex_unlock(&poll_mutex);

		return TRUE;
	}

	ui_update(NULL);

	player_music_info_free(old_info);

	old_info = new_info;

	pthread_mutex_unlock(&poll_mutex);

	return TRUE;
}


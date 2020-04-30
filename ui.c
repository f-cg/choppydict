#include "gdk/gdkkeysyms.h"
#include "storage.h"
#include <pthread.h>
#include <gtk/gtk.h>
#include "get_word.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

void markup(GtkWidget *word_label, word_storage_t *ws)
{
	char *markup;
	if (!strcmp(ws->phonetic_symbol1, ws->phonetic_symbol2)) {
		const char *format =
			"<span>[\%s]</span>\n<span><b>\%s</b></span>\n<span>%s</span>\n<span>%s</span>";
		markup = g_markup_printf_escaped(format, ws->phonetic_symbol1,
						 ws->meaning, ws->chart1,
						 ws->chart2);
	} else {
		const char *format =
			"<span>gb:[\%s]\tus:[%s]</span>\n<span><b>\%s</b></span>\n<span>%s</span>\n<span>%s</span>";
		markup = g_markup_printf_escaped(format, ws->phonetic_symbol1,
						 ws->phonetic_symbol2,
						 ws->meaning, ws->chart1,
						 ws->chart2);
	}
	gtk_label_set_markup(GTK_LABEL(word_label), markup);
	g_free(markup);
}
void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	switch (event->keyval) {
	case GDK_KEY_Q:
	case GDK_KEY_q:
		exit(0);
	}
}

static void activate(GtkApplication *app, gpointer user_data)
{
	/* 这种获取鼠标指针位置的方法已经过时了
	GdkDisplay *display = gdk_display_get_default();
	GdkDeviceManager *device_manager =
		gdk_display_get_device_manager(display);
	GdkDevice *device =
		gdk_device_manager_get_client_pointer(device_manager);
	int x, y;
	gdk_device_get_position(device, NULL, &x, &y);
	printf("x= %d, y=%d\n", x, y);
	*/
	/* 这种方法代替上一种方法
	GdkDisplay *display = gdk_display_get_default();
	GdkSeat* seat = gdk_display_get_default_seat(display);
	GdkDevice *device = gdk_seat_get_keyboard(seat);
	int x, y;
	gdk_device_get_position(device, NULL, &x, &y);
	printf("x= %d, y=%d\n", x, y);
	*/

	word_storage_t *ws = user_data;
	printf("ws:%s\n", ws->word_m);
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *word_label;
	GtkWidget *url_btn;
	/** GtkWidget *mean_label; */
	/** GtkWidget *flowbox; */

	for (int i = 0; ws->meaning[i]; i++) {
		if (ws->meaning[i] == ':')
			ws->meaning[i] = ' ';
	}
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), ws->word_m);
	/** gtk_widget_set_size_request(window,100,20); */
	gtk_window_set_default_size(GTK_WINDOW(window), 150, 20);
	/** gtk_window_move(GTK_WINDOW(window), x, y); */
	// 设置窗口位置在鼠标指针处
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_window_set_keep_above(GTK_WINDOW(window), 1);
	g_signal_connect(G_OBJECT(window), "key_press_event",
			 G_CALLBACK(on_key_press), NULL);

	word_label = gtk_label_new(ws->word_m);
	/** gtk_label_set_markup(GTK_LABEL(word_label), meaning); */
	/** flowbox = gtk_flow_box_new(); */
	/** gtk_container_add(GTK_CONTAINER(flowbox), word_label); */
	markup(word_label, ws);
	/** system("xdg-open \"http://dict.cn/http\""); */
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start(GTK_BOX(box), word_label, FALSE, FALSE, 0);
#ifdef URLBTN
	char url[sizeof(DICTCN) + strlen(ws->word_m)];
	memcpy(url, DICTCN, DICTCNSZ);
	strcpy(url + DICTCNSZ - 1, ws->word_m);
	url_btn = gtk_link_button_new(ws->word_m);
	printf("url:%s\n", url);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(url_btn), url);
	gtk_box_pack_start(GTK_BOX(box), url_btn, FALSE, FALSE, 0);
#endif
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
	GtkApplication *app;
	int status, iret;
	char *word;
	pthread_t voice_thread;
	word = get_selected_word();
	if (!word)
		return -1;
	word_storage_t *ws = get_ws(word);
	if (!ws) {
		printf("word:%s\n", word);
		return -1;
	}
	printf("original word:%s, ws->word:%s\n", word, ws->word_m);

	iret = pthread_create(&voice_thread, NULL, (void(*))ws_voice,
			      (void *)ws);

	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);

	g_signal_connect(app, "activate", G_CALLBACK(activate), ws);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	/** pthread_join(voice_thread, NULL); */
	/** destroy(ws); */

	return status;
}

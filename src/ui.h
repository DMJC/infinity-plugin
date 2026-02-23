#ifndef __INFINITY_UI__
#define __INFINITY_UI__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

gboolean ui_init(gint32 width, gint32 height);
void ui_use_gtk(void);
void ui_use_qt(void);
void ui_quit(void);
void ui_present(const guint16 *pixels, gint32 width, gint32 height);
void ui_resize(gint32 width, gint32 height);
void ui_toggle_fullscreen(void);
void ui_exit_fullscreen_if_needed(void);

void display_notify_resize(gint32 width, gint32 height);
void display_notify_close(void);
void display_notify_visibility(gboolean is_visible);

void *ui_get_gtk_widget(void);
void *ui_get_qt_widget(void);

gboolean ui_gtk_init(gint32 width, gint32 height);
void ui_gtk_quit(void);
void ui_gtk_present(const guint16 *pixels, gint32 width, gint32 height);
void ui_gtk_resize(gint32 width, gint32 height);
void ui_gtk_toggle_fullscreen(void);
void ui_gtk_exit_fullscreen_if_needed(void);
void *ui_gtk_get_widget(void);

gboolean ui_qt_init(gint32 width, gint32 height);
void ui_qt_quit(void);
void ui_qt_present(const guint16 *pixels, gint32 width, gint32 height);
void ui_qt_resize(gint32 width, gint32 height);
void ui_qt_toggle_fullscreen(void);
void ui_qt_exit_fullscreen_if_needed(void);
void *ui_qt_get_widget(void);

#ifdef __cplusplus
}
#endif

#endif /* __INFINITY_UI__ */

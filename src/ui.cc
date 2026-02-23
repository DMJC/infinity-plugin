#include "ui.h"

enum class UiBackend {
	Gtk,
	Qt
};

static UiBackend backend = UiBackend::Gtk;

void ui_use_gtk(void)
{
	backend = UiBackend::Gtk;
}

void ui_use_qt(void)
{
	backend = UiBackend::Qt;
}

gboolean ui_init(gint32 width, gint32 height)
{
	return backend == UiBackend::Qt ? ui_qt_init(width, height) : ui_gtk_init(width, height);
}

void ui_quit(void)
{
	if (backend == UiBackend::Qt) {
		ui_qt_quit();
	} else {
		ui_gtk_quit();
	}
}

void ui_present(const guint16 *pixels, gint32 width, gint32 height)
{
	if (backend == UiBackend::Qt) {
		ui_qt_present(pixels, width, height);
	} else {
		ui_gtk_present(pixels, width, height);
	}
}

void ui_resize(gint32 width, gint32 height)
{
	if (backend == UiBackend::Qt) {
		ui_qt_resize(width, height);
	} else {
		ui_gtk_resize(width, height);
	}
}

void ui_toggle_fullscreen(void)
{
	if (backend == UiBackend::Qt) {
		ui_qt_toggle_fullscreen();
	} else {
		ui_gtk_toggle_fullscreen();
	}
}

void ui_exit_fullscreen_if_needed(void)
{
	if (backend == UiBackend::Qt) {
		ui_qt_exit_fullscreen_if_needed();
	} else {
		ui_gtk_exit_fullscreen_if_needed();
	}
}

void *ui_get_gtk_widget(void)
{
	ui_use_gtk();
	return ui_gtk_get_widget();
}

void *ui_get_qt_widget(void)
{
	ui_use_qt();
	return ui_qt_get_widget();
}

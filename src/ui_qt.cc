#include "ui.h"
#include "input.h"

#include <QApplication>
#include <QCloseEvent>
#include <QHideEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPointer>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWidget>
#include <QtGlobal>
#include <QEventLoop>

#include <mutex>
#include <vector>

namespace {

class InfinityWindow final : public QWidget {
public:
	InfinityWindow() {
		setWindowTitle(QStringLiteral("Infinity"));
		setMinimumSize(200, 150);
		setFocusPolicy(Qt::StrongFocus);
	}

	void update_frame(const guint16 *pixels, gint32 width, gint32 height) {
		if (pixels == nullptr || width <= 0 || height <= 0) {
			return;
		}

		{
			QMutexLocker locker(&frame_mutex_);
			frame_width_ = width;
			frame_height_ = height;
			frame_pixels_.assign(pixels, pixels + (width * height));
		}
		QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
	}

	void set_fullscreen(bool enabled) {
		if (enabled) {
			showFullScreen();
		} else {
			showNormal();
		}
	}

	bool is_fullscreen() const {
		return isFullScreen();
	}

protected:
	void paintEvent(QPaintEvent *) override {
		QPainter painter(this);
		std::vector<guint16> frame_pixels;
		gint32 frame_width = 0;
		gint32 frame_height = 0;
		{
			QMutexLocker locker(&frame_mutex_);
			frame_pixels = frame_pixels_;
			frame_width = frame_width_;
			frame_height = frame_height_;
		}
		if (!frame_pixels.empty() && frame_width > 0 && frame_height > 0) {
			QImage frame_copy(reinterpret_cast<const uchar *>(frame_pixels.data()),
					 frame_width,
					 frame_height,
					 QImage::Format_RGB16);
			painter.drawImage(rect(), frame_copy);
		}
	}

	void resizeEvent(QResizeEvent *event) override {
		const qreal ratio = devicePixelRatioF();
		const gint32 pixel_width = qRound(event->size().width() * ratio);
		const gint32 pixel_height = qRound(event->size().height() * ratio);
		display_notify_resize(pixel_width, pixel_height);
		QWidget::resizeEvent(event);
	}

	void showEvent(QShowEvent *event) override {
		display_notify_visibility(TRUE);
		QWidget::showEvent(event);
	}

	void hideEvent(QHideEvent *event) override {
		display_notify_visibility(FALSE);
		QWidget::hideEvent(event);
	}

	void closeEvent(QCloseEvent *event) override {
		display_notify_close();
		QWidget::closeEvent(event);
	}

	void keyPressEvent(QKeyEvent *event) override {
		switch (event->key()) {
		case Qt::Key_Right:
			infinity_queue_key(INFINITY_KEY_RIGHT);
			break;
		case Qt::Key_Left:
			infinity_queue_key(INFINITY_KEY_LEFT);
			break;
		case Qt::Key_Up:
			infinity_queue_key(INFINITY_KEY_UP);
			break;
		case Qt::Key_Down:
			infinity_queue_key(INFINITY_KEY_DOWN);
			break;
		case Qt::Key_Z:
			infinity_queue_key(INFINITY_KEY_PREV);
			break;
		case Qt::Key_X:
			infinity_queue_key(INFINITY_KEY_PLAY);
			break;
		case Qt::Key_C:
			infinity_queue_key(INFINITY_KEY_PAUSE);
			break;
		case Qt::Key_V:
			infinity_queue_key(INFINITY_KEY_STOP);
			break;
		case Qt::Key_B:
			infinity_queue_key(INFINITY_KEY_NEXT);
			break;
		case Qt::Key_F11:
			infinity_queue_key(INFINITY_KEY_FULLSCREEN);
			break;
		case Qt::Key_Escape:
			infinity_queue_key(INFINITY_KEY_EXIT_FULLSCREEN);
			break;
		case Qt::Key_F12:
			infinity_queue_key(INFINITY_KEY_NEXT_PALETTE);
			break;
		case Qt::Key_Space:
			infinity_queue_key(INFINITY_KEY_NEXT_EFFECT);
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			infinity_queue_key(INFINITY_KEY_TOGGLE_INTERACTIVE);
			break;
		default:
			break;
		}
		QWidget::keyPressEvent(event);
	}

private:
	QMutex frame_mutex_;
	std::vector<guint16> frame_pixels_;
	gint32 frame_width_ = 0;
	gint32 frame_height_ = 0;
};

QPointer<InfinityWindow> window_instance;
bool standalone_window = false;
std::mutex window_mutex;

void process_events() {
	if (QApplication::instance() == nullptr) {
		return;
	}
	QApplication::processEvents(QEventLoop::AllEvents, 1);
}

} // namespace

gboolean ui_qt_init(gint32 width, gint32 height)
{
	if (QApplication::instance() == nullptr) {
		return FALSE;
	}

	QPointer<InfinityWindow> window;
	bool is_standalone = false;
	{
		std::lock_guard<std::mutex> lock(window_mutex);
		if (window_instance.isNull()) {
			window_instance = new InfinityWindow();
			standalone_window = true;
		}
		window = window_instance;
		is_standalone = standalone_window;
	}
	if (window.isNull()) {
		return FALSE;
	}
	window->resize(width, height);
	if (!window.isNull() && is_standalone) {
		window->show();
		window->raise();
	}
	process_events();
	return TRUE;
}

void ui_qt_quit(void)
{
	std::lock_guard<std::mutex> lock(window_mutex);
	if (window_instance.isNull()) {
		return;
	}
	if (standalone_window) {
		window_instance->close();
		delete window_instance.data();
	}
	window_instance = nullptr;
	standalone_window = false;
}

void ui_qt_present(const guint16 *pixels, gint32 width, gint32 height)
{
	QPointer<InfinityWindow> window;
	{
		std::lock_guard<std::mutex> lock(window_mutex);
		window = window_instance;
	}
	if (window.isNull()) {
		return;
	}
	window->update_frame(pixels, width, height);
	process_events();
}

void ui_qt_resize(gint32 width, gint32 height)
{
	QPointer<InfinityWindow> window;
	{
		std::lock_guard<std::mutex> lock(window_mutex);
		window = window_instance;
	}
	if (window.isNull()) {
		return;
	}
	const qreal ratio = window->devicePixelRatioF();
	const gint32 logical_width = qRound(width / ratio);
	const gint32 logical_height = qRound(height / ratio);
	window->resize(logical_width, logical_height);
	process_events();
}

void ui_qt_toggle_fullscreen(void)
{
	QPointer<InfinityWindow> window;
	{
		std::lock_guard<std::mutex> lock(window_mutex);
		window = window_instance;
	}
	if (window.isNull()) {
		return;
	}
	window->set_fullscreen(!window->is_fullscreen());
	process_events();
	const qreal ratio = window->devicePixelRatioF();
	display_notify_resize(qRound(window->width() * ratio),
			      qRound(window->height() * ratio));
}

void ui_qt_exit_fullscreen_if_needed(void)
{
	QPointer<InfinityWindow> window;
	{
		std::lock_guard<std::mutex> lock(window_mutex);
		window = window_instance;
	}
	if (window.isNull()) {
		return;
	}
	if (window->is_fullscreen()) {
		window->set_fullscreen(false);
		process_events();
		const qreal ratio = window->devicePixelRatioF();
		display_notify_resize(qRound(window->width() * ratio),
				      qRound(window->height() * ratio));
	}
}

void *ui_qt_get_widget(void)
{
	std::lock_guard<std::mutex> lock(window_mutex);
	if (QApplication::instance() == nullptr) {
		return nullptr;
	}
	if (window_instance.isNull()) {
		window_instance = new InfinityWindow();
		standalone_window = false;
	}
	/*
	 * Create the widget instance but do not show a top-level window here.
	 * Audacious can probe both widget hooks, and eager creation would show
	 * both GTK and Qt windows at load time.
	 */
	return window_instance.data();
}

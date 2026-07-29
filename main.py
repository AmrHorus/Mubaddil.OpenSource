from __future__ import annotations
import ctypes
import sys
import traceback
from typing import Any
from PySide6.QtCore import Qt, Slot, Signal, QObject, QTimer
from PySide6.QtWidgets import QApplication, QMessageBox
from PySide6.QtGui import QFont, QPalette, QColor
from core import MubaddilCore, ConfigManager, Direction
from ui import MainWindow, SuggestionDialog, HistoryDialog, RejectedDialog, SystemTray, Theme


class CoreBridge(QObject):
    suggestion_ready = Signal(dict)
    corrected_ready = Signal(dict)
    rejected_ready = Signal(dict)

    def __init__(self, core: MubaddilCore) -> None:
        super().__init__()
        self._core = core
        self._core.set_ui_callback(self._on_core_event)

    def _on_core_event(self, event_type: str, data: dict[str, Any]) -> None:
        if event_type == "suggestion":
            self.suggestion_ready.emit(data)
        elif event_type == "corrected":
            self.corrected_ready.emit(data)
        elif event_type == "rejected":
            self.rejected_ready.emit(data)


class MubaddilApp:
    def __init__(self) -> None:
        self._core = MubaddilCore()
        self._bridge = CoreBridge(self._core)
        self._window = MainWindow()
        self._tray = SystemTray()
        self._config = ConfigManager()
        self._current_dialog = None
        self._target_hwnd = 0

        self._setup_connections()
        self._setup_tray()
        self._load_settings()

    def _setup_connections(self) -> None:
        # UI signals
        self._window.toggle_monitoring_requested.connect(self._on_toggle_monitoring)
        self._window.show_history_requested.connect(self._on_show_history)
        self._window.clear_history_requested.connect(self._on_clear_history)
        self._window.show_rejected_requested.connect(self._on_show_rejected)
        self._window.setting_changed.connect(self._on_setting_changed)
        self._window.close_requested.connect(self._on_close)

        # Tray signals
        self._tray.show_window.connect(self._show_window)
        self._tray.toggle_monitoring.connect(self._on_toggle_monitoring)
        self._tray.quit_app.connect(self._on_close)

        # Bridge signals (من Qt thread الآن!)
        self._bridge.suggestion_ready.connect(self._on_suggestion, Qt.ConnectionType.QueuedConnection)
        self._bridge.corrected_ready.connect(self._on_corrected, Qt.ConnectionType.QueuedConnection)
        self._bridge.rejected_ready.connect(self._on_rejected, Qt.ConnectionType.QueuedConnection)

    def _setup_tray(self) -> None:
        self._tray.show()
        self._tray.set_monitoring_status(self._core.is_monitoring())

    def _load_settings(self) -> None:
        self._window.set_settings_state(self._config.all())

    @Slot()
    def _on_toggle_monitoring(self) -> None:
        active = self._core.toggle_monitoring()
        self._window.set_monitoring_status(active)
        self._tray.set_monitoring_status(active)
        self._tray.show_notification(
            "مُبَدِّلْ",
            "✅ المراقبة نشطة" if active else "⏸ المراقبة متوقفة"
        )

    @Slot()
    def _on_show_history(self) -> None:
        dialog = HistoryDialog(self._core.get_history(), self._window)
        dialog.exec()

    @Slot()
    def _on_clear_history(self) -> None:
        self._core.clear_history()
        self._window.update_stats(0, 0, 0)
        self._window.reset_last_correction()
        self._tray.show_notification("مُبَدِّلْ", "🗑 تم مسح السجل")

    @Slot()
    def _on_show_rejected(self) -> None:
        dialog = RejectedDialog(self._core.get_rejected_words(), self._window)
        dialog.exec()

    @Slot(str, object)
    def _on_setting_changed(self, key: str, value: Any) -> None:
        self._config.set(key, value)

    @Slot()
    def _on_close(self) -> None:
        self._core.stop()
        self._tray.hide()
        QApplication.instance().quit()

    @Slot()
    def _show_window(self) -> None:
        self._window.showNormal()
        self._window.raise_()
        self._window.activateWindow()

    @Slot(dict)
    def _on_suggestion(self, data: dict[str, Any]) -> None:
        print(f"[UI] Showing suggestion: {data['original']} -> {data['suggested']}")
        self._target_hwnd = data.get("hwnd", 0)

        if self._current_dialog is not None:
            try:
                self._current_dialog.close()
            except RuntimeError:
                pass
            self._current_dialog = None

        dialog = SuggestionDialog(
            data["original"],
            data["suggested"],
            data["direction"].value,
            self._window,
        )
        dialog.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose)
        dialog.accepted.connect(self._on_suggestion_accepted)
        dialog.rejected.connect(self._on_suggestion_rejected)
        dialog.destroyed.connect(self._on_dialog_destroyed)
        self._current_dialog = dialog
        dialog.show()

    @Slot(dict)
    def _on_corrected(self, data: dict[str, Any]) -> None:
        counts = self._core.get_counts()
        self._window.update_stats(counts["total"], counts["en_to_ar"], counts["ar_to_en"])
        self._window.update_last_correction(
            data["original"], data["suggested"],
            data["direction"].value, "الآن"
        )

    @Slot(dict)
    def _on_rejected(self, data: dict[str, Any]) -> None:
        pass

    @Slot()
    def _on_dialog_destroyed(self) -> None:
        self._current_dialog = None

    def _on_suggestion_accepted(self, original: str, suggested: str, direction: str) -> None:
        dir_enum = Direction.EN_TO_AR if direction == "en_to_ar" else Direction.AR_TO_EN
        self._core.accept_correction(original, suggested, dir_enum, self._target_hwnd)

    def _on_suggestion_rejected(self, word: str) -> None:
        self._core.reject_correction(word)
        if self._target_hwnd:
            ctypes.windll.user32.SetForegroundWindow(self._target_hwnd)

    def run(self) -> None:
        if self._core.start():
            self._window.set_monitoring_status(True)
            self._tray.set_monitoring_status(True)
        else:
            QMessageBox.critical(
                self._window, "خطأ",
                "فشل تثبيت خطاف لوحة المفاتيح.\n"
                "تأكد من تشغيل البرنامج كمسؤول."
            )

        self._window.show()

        print("\n" + "=" * 60)
        print(" مُبَدِّلْ")
        print("  ✅ اكتب كلمة ثم اضغط مسافة للتصحيح")
        print("  ✅ شاهد الـ console للـ debugging")
        print("=" * 60 + "\n")


def main() -> None:
    try:
        ctypes.windll.shcore.SetProcessDpiAwareness(1)
    except Exception:
        pass

    app = QApplication(sys.argv)
    app.setApplicationName("مُبَدِّلْ")
    app.setStyle("Fusion")

    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(Theme.BG_DARK))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(Theme.TEXT_PRIMARY))
    palette.setColor(QPalette.ColorRole.Base, QColor(Theme.BG_CARD))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(Theme.BG_CARD_ALT))
    palette.setColor(QPalette.ColorRole.Text, QColor(Theme.TEXT_PRIMARY))
    palette.setColor(QPalette.ColorRole.Button, QColor(Theme.BG_CARD))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(Theme.TEXT_PRIMARY))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(Theme.ACCENT))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(Theme.TEXT_PRIMARY))
    app.setPalette(palette)

    font = QFont("Segoe UI", 10)
    font.setStyleHint(QFont.StyleHint.SansSerif)
    app.setFont(font)

    try:
        mubaddil = MubaddilApp()
        mubaddil.run()
        sys.exit(app.exec())
    except Exception as e:
        print(f"❌ خطأ فادح: {e}")
        traceback.print_exc()
        input("\nاضغط Enter للخروج...")


if __name__ == "__main__":
    main()

from __future__ import annotations

from typing import Any, Optional

from PySide6.QtCore import Qt, QPropertyAnimation, QEasingCurve, QTimer, Signal, QObject
from PySide6.QtGui import QColor, QFont, QPalette, QAction
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QFrame, QCheckBox, QDialog,
    QScrollArea, QGraphicsDropShadowEffect, QSystemTrayIcon, QMenu,
    QTextEdit, QStyle,
)


class Theme:
    BG_DARK = "#0d0d1a"
    BG_CARD = "#1a1a2e"
    BG_CARD_ALT = "#12122a"
    ACCENT = "#6C3CE1"
    ACCENT_LIGHT = "#8B5CF6"
    TEXT_PRIMARY = "#FFFFFF"
    TEXT_SECONDARY = "#A0A0B8"
    TEXT_MUTED = "#555566"
    SUCCESS = "#4CAF50"
    SUCCESS_LIGHT = "#66BB6A"
    ERROR = "#EF5350"
    ERROR_LIGHT = "#FF7043"
    WARNING = "#FFA726"
    INFO = "#2196F3"
    BORDER = "#2a2a4a"


class Card(QFrame):
    def __init__(self, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.setObjectName("card")
        self.setStyleSheet(f"""
            #card {{
                background-color: {Theme.BG_CARD};
                border-radius: 12px;
                border: 1px solid {Theme.BORDER};
            }}
        """)
        self.setContentsMargins(16, 16, 16, 16)
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(20)
        shadow.setColor(QColor(0, 0, 0, 80))
        shadow.setOffset(0, 4)
        self.setGraphicsEffect(shadow)


class GradientButton(QPushButton):
    def __init__(self, text: str, color_start: str = Theme.ACCENT,
                 color_end: str = Theme.ACCENT_LIGHT, parent: Optional[QWidget] = None) -> None:
        super().__init__(text, parent)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setMinimumHeight(36)
        self.setStyleSheet(f"""
            GradientButton {{
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 {color_start}, stop:1 {color_end});
                color: white; border: none; border-radius: 8px;
                padding: 8px 20px; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;
            }}
            GradientButton:hover {{
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 {color_end}, stop:1 {color_start});
            }}
        """)


class StatusIndicator(QWidget):
    def __init__(self, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.setFixedSize(80, 32)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(8, 4, 8, 4)
        layout.setSpacing(6)
        self._dot = QLabel("●")
        self._dot.setStyleSheet(f"color: {Theme.SUCCESS}; font-size: 14px;")
        layout.addWidget(self._dot)
        self._text = QLabel("نشط")
        self._text.setStyleSheet(f"color: {Theme.SUCCESS}; font-family: 'Segoe UI'; font-size: 11px;")
        layout.addWidget(self._text)
        self.setStyleSheet(f"StatusIndicator {{ background-color: {Theme.BG_CARD}; border-radius: 16px; border: 1px solid {Theme.BORDER}; }}")

    def set_active(self, active: bool) -> None:
        color = Theme.SUCCESS if active else Theme.ERROR
        self._dot.setStyleSheet(f"color: {color}; font-size: 14px;")
        self._text.setText("نشط" if active else "متوقف")
        self._text.setStyleSheet(f"color: {color}; font-family: 'Segoe UI'; font-size: 11px;")


class StatLabel(QWidget):
    def __init__(self, title: str, value: str = "0", color: str = Theme.TEXT_SECONDARY,
                 parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        self._title = QLabel(title)
        self._title.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 11px;")
        layout.addWidget(self._title)
        self._value = QLabel(value)
        self._value.setStyleSheet(f"color: {color}; font-family: 'Segoe UI'; font-size: 11px; font-weight: bold;")
        layout.addWidget(self._value)
        layout.addStretch()

    def set_value(self, value: str) -> None:
        self._value.setText(value)


class SuggestionDialog(QDialog):
    accepted = Signal(str, str, str)
    rejected = Signal(str)

    def __init__(self, original: str, suggested: str, direction: str,
                 parent: Optional[QWidget] = None) -> None:
        super().__init__(parent, Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint)
        self._original, self._suggested, self._direction = original, suggested, direction
        self._closed = False
        self._setup_ui()
        self._center()
        QTimer.singleShot(8000, self._reject)
        self._animate_in()

    def _setup_ui(self) -> None:
        self.setFixedSize(480, 300)
        self.setStyleSheet(f"SuggestionDialog {{ background-color: {Theme.BG_DARK}; border-radius: 16px; border: 1px solid {Theme.BORDER}; }}")
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(30); shadow.setColor(QColor(0, 0, 0, 120)); shadow.setOffset(0, 8)
        self.setGraphicsEffect(shadow)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 24, 24, 24)
        layout.setSpacing(12)

        header = QHBoxLayout()
        title = QLabel("❓ هل تقصد؟")
        title.setStyleSheet(f"color: {Theme.TEXT_PRIMARY}; font-family: 'Segoe UI'; font-size: 22px; font-weight: bold;")
        header.addWidget(title)
        dir_text = "🇸🇦 عربي" if self._direction == "en_to_ar" else "🇺🇸 إنجليزي"
        dir_color = Theme.SUCCESS if self._direction == "en_to_ar" else Theme.INFO
        dir_label = QLabel(dir_text)
        dir_label.setStyleSheet(f"color: {dir_color}; background-color: {Theme.BG_CARD}; border-radius: 6px; padding: 4px 12px; font-family: 'Segoe UI'; font-size: 11px; font-weight: bold;")
        header.addWidget(dir_label)
        layout.addLayout(header)

        typed_card = Card()
        tl = QHBoxLayout(typed_card)
        tl.setContentsMargins(12, 10, 12, 10)
        tlbl = QLabel("✏️ كتبت")
        tlbl.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 11px; font-weight: bold;")
        tl.addWidget(tlbl)
        ttxt = QLabel(self._original)
        ttxt.setStyleSheet(f"color: {Theme.ERROR}; font-family: 'Segoe UI'; font-size: 18px; font-weight: bold;")
        tl.addWidget(ttxt)
        tl.addStretch()
        layout.addWidget(typed_card)

        layout.addWidget(QLabel("⬇"))
        layout.itemAt(layout.count() - 1).widget().setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-size: 16px;")
        layout.itemAt(layout.count() - 1).widget().setAlignment(Qt.AlignmentFlag.AlignCenter)

        sugg_card = Card()
        sugg_card.setStyleSheet(f"#card {{ background-color: #1b2d1b; border-radius: 12px; border: 1px solid {Theme.SUCCESS}; }}")
        sl = QHBoxLayout(sugg_card)
        sl.setContentsMargins(12, 10, 12, 10)
        slbl = QLabel("💡 تقصد")
        slbl.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 11px; font-weight: bold;")
        sl.addWidget(slbl)
        stxt = QLabel(self._suggested)
        stxt.setStyleSheet(f"color: {Theme.SUCCESS_LIGHT}; font-family: 'Segoe UI'; font-size: 20px; font-weight: bold;")
        sl.addWidget(stxt)
        sl.addStretch()
        layout.addWidget(sugg_card)

        bl = QHBoxLayout()
        bl.setSpacing(12)
        by = GradientButton("✅ نعم", Theme.SUCCESS, Theme.SUCCESS_LIGHT)
        by.setMinimumWidth(100); by.clicked.connect(self._accept)
        bl.addWidget(by)
        bn = GradientButton("❌ لا", Theme.ERROR, Theme.ERROR_LIGHT)
        bn.setMinimumWidth(100); bn.clicked.connect(self._reject)
        bl.addWidget(bn)
        layout.addLayout(bl)

        hint = QLabel("⌨️ Enter: نعم  |  ESC: لا")
        hint.setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-family: 'Segoe UI'; font-size: 9px;")
        hint.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(hint)

    def _center(self) -> None:
        screen = QApplication.primaryScreen().geometry()
        self.move((screen.width() - self.width()) // 2, (screen.height() - self.height()) // 2)

    def _animate_in(self) -> None:
        self.setWindowOpacity(0.0)
        self.anim = QPropertyAnimation(self, b"windowOpacity")
        self.anim.setDuration(300); self.anim.setStartValue(0.0); self.anim.setEndValue(1.0)
        self.anim.setEasingCurve(QEasingCurve.Type.OutCubic)
        self.anim.start()

    def _accept(self) -> None:
        if self._closed: return
        self._closed = True
        self.accepted.emit(self._original, self._suggested, self._direction)
        self._animate_out()

    def _reject(self) -> None:
        if self._closed: return
        self._closed = True
        self.rejected.emit(self._original)
        self._animate_out()

    def _animate_out(self) -> None:
        self.anim_out = QPropertyAnimation(self, b"windowOpacity")
        self.anim_out.setDuration(200); self.anim_out.setStartValue(1.0); self.anim_out.setEndValue(0.0)
        self.anim_out.setEasingCurve(QEasingCurve.Type.InCubic)
        self.anim_out.finished.connect(self.close)
        self.anim_out.start()

    def keyPressEvent(self, event) -> None:
        if event.key() in (Qt.Key.Key_Return, Qt.Key.Key_Enter, Qt.Key.Key_Y):
            self._accept()
        elif event.key() in (Qt.Key.Key_Escape, Qt.Key.Key_N):
            self._reject()
        else:
            super().keyPressEvent(event)


class HistoryDialog(QDialog):
    def __init__(self, history: list, parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("📜 سجل التصحيحات")
        self.setMinimumSize(500, 400)
        self.setStyleSheet(f"HistoryDialog {{ background-color: {Theme.BG_DARK}; }}")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)

        if not history:
            lbl = QLabel("لا توجد تصحيحات بعد.")
            lbl.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 14px;")
            lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
            layout.addWidget(lbl)
            return

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setStyleSheet("border: none; background: transparent;")
        container = QWidget()
        cl = QVBoxLayout(container)
        cl.setSpacing(8)

        for i, entry in enumerate(history[:20], 1):
            arrow = "🇺🇸→🇸🇦" if entry.direction.value == "en_to_ar" else "🇸🇦→🇺🇸"
            card = Card()
            cdl = QVBoxLayout(card)
            cdl.setContentsMargins(12, 10, 12, 10)
            l1 = QLabel(f"{i}. [{entry.timestamp}] {entry.original} → {entry.converted}")
            l1.setStyleSheet(f"color: {Theme.TEXT_PRIMARY}; font-family: 'Segoe UI'; font-size: 12px;")
            cdl.addWidget(l1)
            l2 = QLabel(f"   {arrow}")
            l2.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 11px;")
            cdl.addWidget(l2)
            cl.addWidget(card)

        cl.addStretch()
        scroll.setWidget(container)
        layout.addWidget(scroll)
        btn = GradientButton("إغلاق")
        btn.clicked.connect(self.accept)
        layout.addWidget(btn, alignment=Qt.AlignmentFlag.AlignCenter)


class RejectedDialog(QDialog):
    def __init__(self, words: list[str], parent: Optional[QWidget] = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("🚫 الكلمات المرفوضة")
        self.setMinimumSize(400, 300)
        self.setStyleSheet(f"RejectedDialog {{ background-color: {Theme.BG_DARK}; }}")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)

        if not words:
            lbl = QLabel("لا توجد كلمات مرفوضة.")
            lbl.setStyleSheet(f"color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 14px;")
            lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
            layout.addWidget(lbl)
            return

        text = QTextEdit()
        text.setReadOnly(True)
        text.setStyleSheet(f"QTextEdit {{ background-color: {Theme.BG_CARD}; color: {Theme.TEXT_PRIMARY}; border: 1px solid {Theme.BORDER}; border-radius: 8px; padding: 12px; font-family: 'Segoe UI'; font-size: 12px; }}")
        lines = ["🚫 الكلمات المرفوضة", "=" * 35, ""] + [f"{i}. {w}" for i, w in enumerate(words, 1)]
        text.setText("\n".join(lines))
        layout.addWidget(text)
        btn = GradientButton("إغلاق")
        btn.clicked.connect(self.accept)
        layout.addWidget(btn, alignment=Qt.AlignmentFlag.AlignCenter)


class SystemTray(QObject):
    show_window = Signal()
    toggle_monitoring = Signal()
    quit_app = Signal()

    def __init__(self, parent: Optional[QObject] = None) -> None:
        super().__init__(parent)
        self._tray = QSystemTrayIcon(self)
        # تعيين أيقونة افتراضية
        icon = QApplication.style().standardIcon(QStyle.StandardPixmap.SP_ComputerIcon)
        self._tray.setIcon(icon)
        self._tray.setToolTip("مُبَدِّلْ")
        self._menu = QMenu()
        self._menu.setStyleSheet(f"""
            QMenu {{ background-color: {Theme.BG_CARD}; color: {Theme.TEXT_PRIMARY}; border: 1px solid {Theme.BORDER}; border-radius: 8px; padding: 8px; }}
            QMenu::item {{ padding: 8px 24px; border-radius: 6px; }}
            QMenu::item:selected {{ background-color: #252540; }}
        """)
        self._action_show = QAction("إظهار", self)
        self._action_show.triggered.connect(self.show_window.emit)
        self._menu.addAction(self._action_show)
        self._action_toggle = QAction("⏸ إيقاف", self)
        self._action_toggle.triggered.connect(self.toggle_monitoring.emit)
        self._menu.addAction(self._action_toggle)
        self._menu.addSeparator()
        self._action_quit = QAction("❌ خروج", self)
        self._action_quit.triggered.connect(self.quit_app.emit)
        self._menu.addAction(self._action_quit)
        self._tray.setContextMenu(self._menu)
        self._tray.activated.connect(self._on_activated)

    def show(self) -> None: self._tray.show()
    def hide(self) -> None: self._tray.hide()

    def set_monitoring_status(self, active: bool) -> None:
        self._action_toggle.setText("▶ تشغيل" if not active else "⏸ إيقاف")

    def show_notification(self, title: str, message: str) -> None:
        self._tray.showMessage(title, message, QSystemTrayIcon.MessageIcon.Information, 3000)

    def _on_activated(self, reason: QSystemTrayIcon.ActivationReason) -> None:
        if reason == QSystemTrayIcon.ActivationReason.DoubleClick:
            self.show_window.emit()


class MainWindow(QMainWindow):
    toggle_monitoring_requested = Signal()
    show_history_requested = Signal()
    clear_history_requested = Signal()
    show_rejected_requested = Signal()
    setting_changed = Signal(str, object)
    close_requested = Signal()

    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("مُبَدِّلْ 0.0.0")
        self.setMinimumSize(460, 560)
        self.setMaximumSize(520, 650)
        self._setup_window()
        self._setup_ui()

    def _setup_window(self) -> None:
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowMinimizeButtonHint)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self._central = QWidget()
        self._central.setStyleSheet(f"QWidget {{ background-color: {Theme.BG_DARK}; border-radius: 16px; border: 1px solid {Theme.BORDER}; }}")
        self.setCentralWidget(self._central)
        shadow = QGraphicsDropShadowEffect(self._central)
        shadow.setBlurRadius(40); shadow.setColor(QColor(0, 0, 0, 150)); shadow.setOffset(0, 10)
        self._central.setGraphicsEffect(shadow)

    def _setup_ui(self) -> None:
        layout = QVBoxLayout(self._central)
        layout.setContentsMargins(20, 16, 20, 16)
        layout.setSpacing(12)
        self._build_title_bar(layout)
        self._build_stats(layout)
        self._build_settings(layout)
        self._build_last(layout)
        self._build_buttons(layout)
        self._build_hint(layout)

    def _build_title_bar(self, layout: QVBoxLayout) -> None:
        header = QHBoxLayout()
        header.setSpacing(12)
        tc = QVBoxLayout(); tc.setSpacing(2)
        title = QLabel("مُبَدِّلْ")
        title.setStyleSheet(f"color: {Theme.ACCENT}; font-family: 'Segoe UI'; font-size: 28px; font-weight: bold;")
        tc.addWidget(title)
        sub = QLabel("تصحيح لوحة المفاتيح")
        sub.setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-family: 'Segoe UI'; font-size: 11px;")
        tc.addWidget(sub)
        header.addLayout(tc)
        header.addStretch()

        self._status = StatusIndicator()
        header.addWidget(self._status)

        self._toggle_btn = QPushButton("⏸")
        self._toggle_btn.setFixedSize(32, 32)
        self._toggle_btn.setStyleSheet(f"QPushButton {{ background-color: {Theme.ERROR}; color: white; border: none; border-radius: 16px; font-size: 12px; font-weight: bold; }} QPushButton:hover {{ background-color: {Theme.ERROR_LIGHT}; }}")
        self._toggle_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._toggle_btn.clicked.connect(self.toggle_monitoring_requested.emit)
        header.addWidget(self._toggle_btn)

        min_btn = QPushButton("−")
        min_btn.setFixedSize(28, 28)
        min_btn.setStyleSheet(f"QPushButton {{ background-color: {Theme.BG_CARD}; color: {Theme.TEXT_SECONDARY}; border: 1px solid {Theme.BORDER}; border-radius: 14px; font-size: 14px; font-weight: bold; }} QPushButton:hover {{ background-color: #252540; color: {Theme.TEXT_PRIMARY}; }}")
        min_btn.clicked.connect(self.showMinimized)
        header.addWidget(min_btn)

        close_btn = QPushButton("✕")
        close_btn.setFixedSize(28, 28)
        close_btn.setStyleSheet(f"QPushButton {{ background-color: {Theme.BG_CARD}; color: {Theme.TEXT_SECONDARY}; border: 1px solid {Theme.BORDER}; border-radius: 14px; font-size: 12px; font-weight: bold; }} QPushButton:hover {{ background-color: {Theme.ERROR}; color: white; }}")
        close_btn.clicked.connect(self.close_requested.emit)
        header.addWidget(close_btn)
        layout.addLayout(header)

    def _build_stats(self, layout: QVBoxLayout) -> None:
        card = Card()
        cl = QVBoxLayout(card); cl.setSpacing(8)
        t = QLabel("📊 الإحصائيات")
        t.setStyleSheet(f"color: {Theme.TEXT_PRIMARY}; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;")
        cl.addWidget(t)
        self._stat_total = StatLabel("التصحيحات:", "0", Theme.TEXT_SECONDARY)
        cl.addWidget(self._stat_total)
        self._stat_en_ar = StatLabel("🇺🇸→🇸🇦:", "0", Theme.INFO)
        cl.addWidget(self._stat_en_ar)
        self._stat_ar_en = StatLabel("🇸🇦→🇺🇸:", "0", Theme.WARNING)
        cl.addWidget(self._stat_ar_en)
        layout.addWidget(card)

    def _build_settings(self, layout: QVBoxLayout) -> None:
        card = Card()
        card.setStyleSheet(f"#card {{ background-color: {Theme.BG_CARD_ALT}; border-radius: 12px; border: 1px solid {Theme.BORDER}; }}")
        cl = QVBoxLayout(card); cl.setSpacing(10)
        t = QLabel("⚙️ الإعدادات")
        t.setStyleSheet(f"color: {Theme.TEXT_PRIMARY}; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;")
        cl.addWidget(t)

        chk_style = f"QCheckBox {{ color: {Theme.TEXT_SECONDARY}; font-family: 'Segoe UI'; font-size: 12px; spacing: 8px; }} QCheckBox::indicator {{ width: 18px; height: 18px; border-radius: 4px; border: 1px solid {Theme.BORDER}; background-color: #151525; }} QCheckBox::indicator:checked {{ background-color: {Theme.ACCENT}; border-color: {Theme.ACCENT}; }}"

        self._chk_switch = QCheckBox("تبديل اللغة تلقائياً مع التصحيح")
        self._chk_switch.setChecked(True)
        self._chk_switch.setStyleSheet(chk_style)
        self._chk_switch.stateChanged.connect(lambda s: self.setting_changed.emit("switch_keyboard", bool(s)))
        cl.addWidget(self._chk_switch)

        self._chk_dialog = QCheckBox("عرض نافذة التأكيد")
        self._chk_dialog.setChecked(True)
        self._chk_dialog.setStyleSheet(chk_style)
        self._chk_dialog.stateChanged.connect(lambda s: self.setting_changed.emit("show_dialog", bool(s)))
        cl.addWidget(self._chk_dialog)
        layout.addWidget(card)

    def _build_last(self, layout: QVBoxLayout) -> None:
        card = Card()
        cl = QVBoxLayout(card); cl.setSpacing(6)
        t = QLabel("🔄 آخر تصحيح")
        t.setStyleSheet(f"color: {Theme.TEXT_PRIMARY}; font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;")
        cl.addWidget(t)
        self._last_label = QLabel("لا يوجد")
        self._last_label.setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-family: 'Segoe UI'; font-size: 13px;")
        cl.addWidget(self._last_label)
        layout.addWidget(card)

    def _build_buttons(self, layout: QVBoxLayout) -> None:
        bl = QHBoxLayout(); bl.setSpacing(8)
        bh = GradientButton("📜 السجل", Theme.INFO, "#42A5F5")
        bh.setMinimumWidth(90); bh.clicked.connect(self.show_history_requested.emit)
        bl.addWidget(bh)
        bc = GradientButton("🗑 مسح", Theme.ERROR, Theme.ERROR_LIGHT)
        bc.setMinimumWidth(90); bc.clicked.connect(self.clear_history_requested.emit)
        bl.addWidget(bc)
        br = GradientButton("📝 المرفوضة", Theme.WARNING, "#FFB74D")
        br.setMinimumWidth(90); br.clicked.connect(self.show_rejected_requested.emit)
        bl.addWidget(br)
        layout.addLayout(bl)

    def _build_hint(self, layout: QVBoxLayout) -> None:
        hint = QLabel("💡 اكتب الكلمة ثم اضغط مسافة للتصحيح")
        hint.setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-family: 'Segoe UI'; font-size: 10px;")
        hint.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(hint)

    def set_monitoring_status(self, active: bool) -> None:
        self._status.set_active(active)
        self._toggle_btn.setText("▶" if not active else "⏸")
        c = Theme.SUCCESS if not active else Theme.ERROR
        h = "#66BB6A" if not active else Theme.ERROR_LIGHT
        self._toggle_btn.setStyleSheet(f"QPushButton {{ background-color: {c}; color: white; border: none; border-radius: 16px; font-size: 12px; font-weight: bold; }} QPushButton:hover {{ background-color: {h}; }}")

    def update_stats(self, total: int, en_to_ar: int, ar_to_en: int) -> None:
        self._stat_total.set_value(str(total))
        self._stat_en_ar.set_value(str(en_to_ar))
        self._stat_ar_en.set_value(str(ar_to_en))

    def update_last_correction(self, original: str, converted: str, direction: str, timestamp: str) -> None:
        arrow = "🇺🇸→🇸🇦" if direction == "en_to_ar" else "🇸🇦→🇺🇸"
        self._last_label.setText(f"{original} → {converted}  ({arrow})  [{timestamp}]")
        self._last_label.setStyleSheet(f"color: {Theme.INFO}; font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;")

    def reset_last_correction(self) -> None:
        self._last_label.setText("لا يوجد")
        self._last_label.setStyleSheet(f"color: {Theme.TEXT_MUTED}; font-family: 'Segoe UI'; font-size: 13px;")

    def get_settings_state(self) -> dict[str, bool]:
        return {"switch_keyboard": self._chk_switch.isChecked(), "show_dialog": self._chk_dialog.isChecked()}

    def set_settings_state(self, settings: dict[str, Any]) -> None:
        self._chk_switch.setChecked(settings.get("switch_keyboard", True))
        self._chk_dialog.setChecked(settings.get("show_dialog", True))

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            self._drag_pos = event.globalPosition().toPoint()
            event.accept()

    def mouseMoveEvent(self, event) -> None:
        if event.buttons() == Qt.MouseButton.LeftButton:
            self.move(self.pos() + event.globalPosition().toPoint() - self._drag_pos)
            self._drag_pos = event.globalPosition().toPoint()
            event.accept()

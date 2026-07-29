# core.py - قلب برنامج مُبَدِّلْ (مصحح ومجرب) – نسخة الإنتاج
"""
core.py - Mubaddil Core Engine
تحسينات:
- إعادة كتابة LanguageDetector بخوارزميات متقدمة
- إعادة كتابة SuggestionEngine بمنطق ثقة دقيق
- استبدال keybd_event بـ SendInput (Windows API الحديث)
- إضافة تسجيل احترافي (logging)
- تحسين التخزين المؤقت (LRU)
- إصلاح جميع أخطاء خريطة المفاتيح
- تحسين سلامة الخيوط
"""

from __future__ import annotations

import ctypes
import ctypes.wintypes
import json
import logging
import threading
import time
from collections import OrderedDict
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from typing import Any, ClassVar, Optional, Protocol, final

# ─── إعداد التسجيل ───
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger("MubaddilCore")

# ─── Win32 API ───
user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

WH_KEYBOARD_LL = 13
WM_KEYDOWN = 0x0100
WM_SYSKEYDOWN = 0x0104
WM_KEYUP = 0x0101
WM_SYSKEYUP = 0x0105
WM_INPUTLANGCHANGEREQUEST = 0x0050

VK_BACK = 0x08
VK_SPACE = 0x20
VK_RETURN = 0x0D
VK_TAB = 0x09
VK_CONTROL = 0x11
VK_MENU = 0x12
VK_SHIFT = 0x10
VK_ESCAPE = 0x1B
VK_DELETE = 0x2E
VK_LCONTROL = 0xA2
VK_RCONTROL = 0xA3
VK_LSHIFT = 0xA0
VK_RSHIFT = 0xA1
VK_LMENU = 0xA4
VK_RMENU = 0xA5

CF_UNICODETEXT = 13
GMEM_MOVEABLE = 0x0002

# ─── هياكل Windows ───
class KBDLLHOOKSTRUCT(ctypes.Structure):
    _fields_ = [
        ("vkCode", ctypes.wintypes.DWORD),
        ("scanCode", ctypes.wintypes.DWORD),
        ("flags", ctypes.wintypes.DWORD),
        ("time", ctypes.wintypes.DWORD),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong)),
    ]

# ─── SendInput ───
PUL = ctypes.POINTER(ctypes.c_ulong)

class KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", ctypes.wintypes.WORD),
        ("wScan", ctypes.wintypes.WORD),
        ("dwFlags", ctypes.wintypes.DWORD),
        ("time", ctypes.wintypes.DWORD),
        ("dwExtraInfo", ctypes.wintypes.DWORD),
    ]

class MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx", ctypes.wintypes.LONG),
        ("dy", ctypes.wintypes.LONG),
        ("mouseData", ctypes.wintypes.DWORD),
        ("dwFlags", ctypes.wintypes.DWORD),
        ("time", ctypes.wintypes.DWORD),
        ("dwExtraInfo", ctypes.wintypes.DWORD),
    ]

class HARDWAREINPUT(ctypes.Structure):
    _fields_ = [
        ("uMsg", ctypes.wintypes.DWORD),
        ("wParamL", ctypes.wintypes.WORD),
        ("wParamH", ctypes.wintypes.WORD),
    ]

class INPUT_UNION(ctypes.Union):
    _fields_ = [
        ("ki", KEYBDINPUT),
        ("mi", MOUSEINPUT),
        ("hi", HARDWAREINPUT),
    ]

class INPUT(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.wintypes.DWORD),
        ("union", INPUT_UNION),
    ]

INPUT_KEYBOARD = 1
KEYEVENTF_KEYDOWN = 0x0000
KEYEVENTF_KEYUP = 0x0002
KEYEVENTF_SCANCODE = 0x0008

SendInput = user32.SendInput
SendInput.argtypes = [ctypes.wintypes.UINT, ctypes.POINTER(INPUT), ctypes.c_int]
SendInput.restype = ctypes.wintypes.UINT

def _create_key_input(vk: int, key_up: bool = False) -> INPUT:
    inp = INPUT()
    inp.type = INPUT_KEYBOARD
    inp.union.ki.wVk = vk
    inp.union.ki.dwFlags = KEYEVENTF_KEYUP if key_up else 0
    return inp

def _send_key(vk: int, press: bool = True) -> None:
    inp = _create_key_input(vk, not press)
    SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))

# ─── Enums ───
class Language(Enum):
    ENGLISH = auto()
    ARABIC = auto()
    UNKNOWN = auto()

class Layout(Enum):
    EN = "en"
    AR = "ar"
    UNKNOWN = "unknown"

class Direction(Enum):
    EN_TO_AR = "en_to_ar"
    AR_TO_EN = "ar_to_en"

# ─── KeyDefinition ───
@dataclass(frozen=True, slots=True)
class KeyDefinition:
    vk_code: int
    english_normal: str
    english_shift: str
    arabic_normal: str
    arabic_shift: str

# ─── KeyboardMapper (مصحح بالكامل للتخطيط العربي السعودي) ───
class KeyboardMapper:
    """خريطة المفاتيح الكاملة – معتمدة على تخطيط Windows Arabic (101)."""

    _MAP: ClassVar[dict[int, KeyDefinition]] = {
        # الأرقام
        0x30: KeyDefinition(0x30, "0", ")", "٠", ")"),
        0x31: KeyDefinition(0x31, "1", "!", "١", "!"),
        0x32: KeyDefinition(0x32, "2", "@", "٢", "@"),
        0x33: KeyDefinition(0x33, "3", "#", "٣", "#"),
        0x34: KeyDefinition(0x34, "4", "$", "٤", "$"),
        0x35: KeyDefinition(0x35, "5", "%", "٥", "%"),
        0x36: KeyDefinition(0x36, "6", "^", "٦", "^"),
        0x37: KeyDefinition(0x37, "7", "&", "٧", "&"),
        0x38: KeyDefinition(0x38, "8", "*", "٨", "*"),
        0x39: KeyDefinition(0x39, "9", "(", "٩", "("),
        # الحروف العربية
        0x51: KeyDefinition(0x51, "q", "Q", "ض", "َ"),
        0x57: KeyDefinition(0x57, "w", "W", "ص", "ً"),
        0x45: KeyDefinition(0x45, "e", "E", "ث", "ُ"),
        0x52: KeyDefinition(0x52, "r", "R", "ق", "ٌ"),
        0x54: KeyDefinition(0x54, "t", "T", "ف", "ل"),
        0x59: KeyDefinition(0x59, "y", "Y", "غ", "إ"),
        0x55: KeyDefinition(0x55, "u", "U", "ع", "'"),
        0x49: KeyDefinition(0x49, "i", "I", "ه", "÷"),
        0x4F: KeyDefinition(0x4F, "o", "O", "خ", "×"),
        0x50: KeyDefinition(0x50, "p", "P", "ح", "؛"),
        0xDB: KeyDefinition(0xDB, "[", "{", "ج", "<"),
        0xDD: KeyDefinition(0xDD, "]", "}", "د", ">"),
        0x41: KeyDefinition(0x41, "a", "A", "ش", "ِ"),
        0x53: KeyDefinition(0x53, "s", "S", "س", "ٍ"),
        0x44: KeyDefinition(0x44, "d", "D", "ي", "]"),
        0x46: KeyDefinition(0x46, "f", "F", "ب", "["),
        0x47: KeyDefinition(0x47, "g", "G", "ل", "ل"),
        0x48: KeyDefinition(0x48, "h", "H", "ا", "أ"),
        0x4A: KeyDefinition(0x4A, "j", "J", "ت", "ـ"),
        0x4B: KeyDefinition(0x4B, "k", "K", "ن", "،"),
        0x4C: KeyDefinition(0x4C, "l", "L", "م", "/"),
        0xBA: KeyDefinition(0xBA, ";", ":", "ك", ":"),
        0xDE: KeyDefinition(0xDE, "'", '"', "ط", '"'),
        0x5A: KeyDefinition(0x5A, "z", "Z", "ئ", "~"),
        0x58: KeyDefinition(0x58, "x", "X", "ء", "ْ"),
        0x43: KeyDefinition(0x43, "c", "C", "ؤ", "}"),
        0x56: KeyDefinition(0x56, "v", "V", "ر", "{"),
        0x42: KeyDefinition(0x42, "b", "B", "لا", "آ"),
        0x4E: KeyDefinition(0x4E, "n", "N", "ى", "'"),
        0x4D: KeyDefinition(0x4D, "m", "M", "ة", "'"),
        0xBC: KeyDefinition(0xBC, ",", "<", "و", ","),
        0xBE: KeyDefinition(0xBE, ".", ">", "ز", "."),
        0xBF: KeyDefinition(0xBF, "/", "?", "ظ", "؟"),
        # رموز إضافية
        0x20: KeyDefinition(0x20, " ", " ", " ", " "),
        0xBD: KeyDefinition(0xBD, "-", "_", "-", "_"),
        0xBB: KeyDefinition(0xBB, "=", "+", "=", "+"),
        0xDC: KeyDefinition(0xDC, "\\", "|", "\\", "|"),
        0xC0: KeyDefinition(0xC0, "`", "~", "ذ", "ّ"),
    }

    _EN_TO_AR: ClassVar[dict[str, str]] = {}
    _AR_TO_EN: ClassVar[dict[str, str]] = {}
    _ARABIC_SET: ClassVar[set[str]] = set()
    _ENGLISH_SET: ClassVar[set[str]] = set()
    _initialized: ClassVar[bool] = False

    def __init__(self) -> None:
        if not KeyboardMapper._initialized:
            self._build_maps()
            KeyboardMapper._initialized = True

    def _build_maps(self) -> None:
        for kd in KeyboardMapper._MAP.values():
            if kd.english_normal and kd.arabic_normal:
                KeyboardMapper._EN_TO_AR[kd.english_normal] = kd.arabic_normal
                KeyboardMapper._AR_TO_EN[kd.arabic_normal] = kd.english_normal
                KeyboardMapper._ARABIC_SET.add(kd.arabic_normal)
                KeyboardMapper._ENGLISH_SET.add(kd.english_normal)
            if kd.english_shift and kd.arabic_shift:
                KeyboardMapper._EN_TO_AR[kd.english_shift] = kd.arabic_shift
                KeyboardMapper._AR_TO_EN[kd.arabic_shift] = kd.english_shift
                KeyboardMapper._ARABIC_SET.add(kd.arabic_shift)
                KeyboardMapper._ENGLISH_SET.add(kd.english_shift)

    @classmethod
    def _ensure_initialized(cls) -> None:
        if not cls._initialized:
            cls()

    @classmethod
    def convert_en_to_ar(cls, text: str) -> str:
        cls._ensure_initialized()
        return "".join(cls._EN_TO_AR.get(ch, ch) for ch in text)

    @classmethod
    def convert_ar_to_en(cls, text: str) -> str:
        cls._ensure_initialized()
        return "".join(cls._AR_TO_EN.get(ch, ch) for ch in text)

    @classmethod
    def is_arabic_char(cls, char: str) -> bool:
        cls._ensure_initialized()
        return char in cls._ARABIC_SET

    @classmethod
    def is_english_char(cls, char: str) -> bool:
        cls._ensure_initialized()
        return char in cls._ENGLISH_SET


# ─── WordCache (LRU) ───
@final
class WordCache:
    def __init__(self, max_size: int = 2000) -> None:
        self._max_size = max_size
        self._cache: OrderedDict[str, str] = OrderedDict()
        self._lock = threading.Lock()

    def get(self, word: str, direction: Direction) -> Optional[str]:
        key = f"{word}|{direction.value}"
        with self._lock:
            if key in self._cache:
                self._cache.move_to_end(key)
                return self._cache[key]
            return None

    def set(self, word: str, direction: Direction, result: str) -> None:
        key = f"{word}|{direction.value}"
        with self._lock:
            if key in self._cache:
                self._cache.move_to_end(key)
            else:
                if len(self._cache) >= self._max_size:
                    self._cache.popitem(last=False)
                self._cache[key] = result

    def clear(self) -> None:
        with self._lock:
            self._cache.clear()


# ─── LanguageDetector ───
@final
class LanguageDetector:
    ARABIC_RANGE = range(0x0600, 0x06FF + 1)
    ARABIC_PRESENTATION = range(0xFB50, 0xFDFF + 1)
    ENGLISH_LETTERS = set("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
    DIGITS = set("0123456789")
    PUNCTUATION = set(".,!?;:()[]{}<>-_\"/\\'`~@#$%^&*+=|")

    @classmethod
    def _is_arabic_char(cls, ch: str) -> bool:
        code = ord(ch)
        return (cls.ARABIC_RANGE.start <= code <= cls.ARABIC_RANGE.stop or
                cls.ARABIC_PRESENTATION.start <= code <= cls.ARABIC_PRESENTATION.stop)

    @classmethod
    def _is_english_letter(cls, ch: str) -> bool:
        return ch in cls.ENGLISH_LETTERS

    @classmethod
    def analyze(cls, word: str) -> dict[str, Any]:
        if not word:
            return {
                "ar_count": 0, "en_count": 0, "digit_count": 0,
                "punct_count": 0, "other_count": 0,
                "ar_ratio": 0.0, "en_ratio": 0.0,
                "dominant": Language.UNKNOWN, "is_mixed": False,
                "has_digits": False, "has_punct": False,
                "is_likely_url": False, "is_likely_email": False,
                "is_likely_code": False,
            }

        ar_count = en_count = digit_count = punct_count = 0
        for ch in word:
            if cls._is_arabic_char(ch):
                ar_count += 1
            elif cls._is_english_letter(ch):
                en_count += 1
            elif ch.isdigit():
                digit_count += 1
            elif ch in cls.PUNCTUATION:
                punct_count += 1
        other_count = len(word) - ar_count - en_count - digit_count - punct_count

        total = len(word)
        ar_ratio = ar_count / total if total > 0 else 0.0
        en_ratio = en_count / total if total > 0 else 0.0

        if ar_count > en_count and ar_count > 0:
            dominant = Language.ARABIC
        elif en_count > ar_count and en_count > 0:
            dominant = Language.ENGLISH
        else:
            dominant = Language.UNKNOWN

        is_mixed = (ar_count > 0 and en_count > 0)

        is_likely_url = word.startswith(("http://", "https://", "www.")) or ("." in word and "/" in word)
        is_likely_email = "@" in word and "." in word
        is_likely_code = any(ch in "{}[]();=" for ch in word)

        return {
            "ar_count": ar_count,
            "en_count": en_count,
            "digit_count": digit_count,
            "punct_count": punct_count,
            "other_count": other_count,
            "ar_ratio": ar_ratio,
            "en_ratio": en_ratio,
            "dominant": dominant,
            "is_mixed": is_mixed,
            "has_digits": digit_count > 0,
            "has_punct": punct_count > 0,
            "is_likely_url": is_likely_url,
            "is_likely_email": is_likely_email,
            "is_likely_code": is_likely_code,
        }


# ─── WordValidator ───
@final
class WordValidator:
    _COMMON_ENGLISH: ClassVar[frozenset[str]] = frozenset({
        "the", "be", "to", "of", "and", "a", "in", "that", "have", "i",
        "it", "for", "not", "on", "with", "he", "as", "you", "do", "at",
        "this", "but", "his", "by", "from", "they", "we", "say", "her", "she",
        "or", "an", "will", "my", "one", "all", "would", "there", "their",
        "what", "hello", "hi", "hey", "ok", "okay", "yes", "no", "please",
        "thanks", "good", "bad", "new", "old", "big", "small", "long", "short",
    })
    _COMMON_ARABIC: ClassVar[frozenset[str]] = frozenset({
        "في", "من", "على", "إلى", "عن", "أن", "إن", "كان", "قد", "لا",
        "ما", "مع", "هو", "هي", "نحن", "أنا", "أنت", "هم", "كتاب", "بيت",
        "بين", "منذ", "حتى", "ثم", "إذا", "لأن", "هذا", "ذلك", "تلك",
        "الله", "محمد", "علي", "أحمد", "عمر", "خالد", "سعود", "عربي",
        "شكرا", "مرحبا", "سلام", "صباح", "مساء", "ليلة", "يوم", "شهر", "سنة",
    })
    _rejected: ClassVar[set[str]] = set()
    _rejected_lock: ClassVar[threading.Lock] = threading.Lock()

    @classmethod
    def is_common_english(cls, word: str) -> bool:
        return word.lower() in cls._COMMON_ENGLISH

    @classmethod
    def is_common_arabic(cls, word: str) -> bool:
        return word in cls._COMMON_ARABIC

    @classmethod
    def is_rejected(cls, word: str) -> bool:
        with cls._rejected_lock:
            return word.lower() in cls._rejected or word in cls._rejected

    @classmethod
    def add_rejected(cls, word: str) -> None:
        with cls._rejected_lock:
            cls._rejected.add(word.lower())
            cls._rejected.add(word)

    @classmethod
    def get_rejected_words(cls) -> list[str]:
        with cls._rejected_lock:
            return sorted(cls._rejected)

    @classmethod
    def clear_rejected(cls) -> None:
        with cls._rejected_lock:
            cls._rejected.clear()


# ─── ConverterEngine ───
@final
class ConverterEngine:
    def __init__(self, cache: Optional[WordCache] = None) -> None:
        self._cache = cache or WordCache()

    def convert(self, word: str, direction: Direction) -> str:
        if not word:
            return word
        cached = self._cache.get(word, direction)
        if cached is not None:
            return cached
        if direction == Direction.EN_TO_AR:
            result = KeyboardMapper.convert_en_to_ar(word)
        else:
            result = KeyboardMapper.convert_ar_to_en(word)
        self._cache.set(word, direction, result)
        return result

    def convert_en_to_ar(self, word: str) -> str:
        return self.convert(word, Direction.EN_TO_AR)

    def convert_ar_to_en(self, word: str) -> str:
        return self.convert(word, Direction.AR_TO_EN)


# ─── SuggestionEngine ───
@dataclass
class SuggestionResult:
    original: str
    suggested: str
    direction: Direction
    confidence: float
    should_show: bool


@final
class SuggestionEngine:
    def __init__(self, converter: ConverterEngine) -> None:
        self._converter = converter
        self._detector = LanguageDetector()

    def suggest(self, word: str, current_layout: Layout) -> SuggestionResult:
        if not word or len(word) < 2:
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
        if WordValidator.is_rejected(word):
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)

        analysis = self._detector.analyze(word)
        if analysis["has_digits"] or analysis["has_punct"] or analysis["is_likely_code"]:
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
        if analysis["is_likely_url"] or analysis["is_likely_email"]:
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)

        ar_ratio = analysis["ar_ratio"]
        en_ratio = analysis["en_ratio"]

        # إذا كانت الكلمة عربية بالكامل
        if ar_ratio > 0 and en_ratio == 0:
            # إذا كان التخطيط عربيًا، فلا حاجة للتصحيح
            if current_layout == Layout.AR:
                return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
            else:
                # التخطيط إنجليزي لكن الكلمة عربية -> نحتاج تحويل AR_TO_EN (لتصحيح التخطيط)
                return self._suggest_arabic(word)
        # إذا كانت الكلمة إنجليزية بالكامل
        elif en_ratio > 0 and ar_ratio == 0:
            if current_layout == Layout.EN:
                return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
            else:
                # التخطيط عربي لكن الكلمة إنجليزية -> نحتاج تحويل EN_TO_AR
                return self._suggest_english(word)
        # مختلطة
        else:
            # إذا كان التخطيط عربياً والأغلب عربي، نحاول التحويل إلى عربي
            if current_layout == Layout.AR:
                if ar_ratio > en_ratio:
                    converted = self._converter.convert_ar_to_en(word)
                    if converted != word and self._is_plausible(converted, Language.ENGLISH):
                        return SuggestionResult(word, converted, Direction.AR_TO_EN, 0.6, True)
                else:
                    converted = self._converter.convert_en_to_ar(word)
                    if converted != word and self._is_plausible(converted, Language.ARABIC):
                        return SuggestionResult(word, converted, Direction.EN_TO_AR, 0.6, True)
            else:
                # التخطيط إنجليزي
                if en_ratio > ar_ratio:
                    converted = self._converter.convert_en_to_ar(word)
                    if converted != word and self._is_plausible(converted, Language.ARABIC):
                        return SuggestionResult(word, converted, Direction.EN_TO_AR, 0.6, True)
                else:
                    converted = self._converter.convert_ar_to_en(word)
                    if converted != word and self._is_plausible(converted, Language.ENGLISH):
                        return SuggestionResult(word, converted, Direction.AR_TO_EN, 0.6, True)
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)

    def _suggest_english(self, word: str) -> SuggestionResult:
        # الكلمة إنجليزية لكن التخطيط عربي -> نحول EN_TO_AR
        if WordValidator.is_common_english(word):
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
        converted = self._converter.convert_en_to_ar(word)
        if converted == word:
            return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)
        if self._is_plausible(converted, Language.ARABIC):
            confidence = self._compute_confidence(converted, Language.ARABIC, word)
            return SuggestionResult(word, converted, Direction.EN_TO_AR, confidence, True)
        return SuggestionResult(word, word, Direction.EN_TO_AR, 0.0, False)

    def _suggest_arabic(self, word: str) -> SuggestionResult:
        # الكلمة عربية لكن التخطيط إنجليزي -> نحول AR_TO_EN
        if WordValidator.is_common_arabic(word):
            return SuggestionResult(word, word, Direction.AR_TO_EN, 0.0, False)
        converted = self._converter.convert_ar_to_en(word)
        if converted == word:
            return SuggestionResult(word, word, Direction.AR_TO_EN, 0.0, False)
        if self._is_plausible(converted, Language.ENGLISH):
            confidence = self._compute_confidence(converted, Language.ENGLISH, word)
            return SuggestionResult(word, converted, Direction.AR_TO_EN, confidence, True)
        return SuggestionResult(word, word, Direction.AR_TO_EN, 0.0, False)

    def _is_plausible(self, word: str, target_lang: Language) -> bool:
        if not word:
            return False
        if target_lang == Language.ARABIC:
            ar_count = sum(1 for ch in word if LanguageDetector._is_arabic_char(ch))
            return ar_count / len(word) > 0.6
        else:
            en_count = sum(1 for ch in word if LanguageDetector._is_english_letter(ch))
            return en_count / len(word) > 0.6

    def _compute_confidence(self, converted: str, target_lang: Language, original: str) -> float:
        base = 0.5
        if target_lang == Language.ARABIC:
            if WordValidator.is_common_arabic(converted):
                base += 0.3
            if len(original) >= 4:
                base += 0.1
        else:
            if WordValidator.is_common_english(converted):
                base += 0.3
            if len(original) >= 4:
                base += 0.1
        return min(base, 0.95)


# ─── ClipboardManager ───
@final
class ClipboardManager:
    @staticmethod
    def set_text(text: str) -> bool:
        try:
            if user32.OpenClipboard(None):
                user32.EmptyClipboard()
                wide = (text + '\x00').encode('utf-16le')
                h_mem = kernel32.GlobalAlloc(GMEM_MOVEABLE, len(wide))
                if h_mem:
                    ptr = kernel32.GlobalLock(h_mem)
                    if ptr:
                        ctypes.memmove(ptr, wide, len(wide))
                        kernel32.GlobalUnlock(h_mem)
                        user32.SetClipboardData(CF_UNICODETEXT, h_mem)
                user32.CloseClipboard()
                return True
            logger.error("فشل فتح الحافظة")
        except Exception as e:
            logger.exception("خطأ في ClipboardManager.set_text")
        return False

    @staticmethod
    def paste() -> None:
        _send_key(VK_CONTROL, True)
        _send_key(ord('V'), True)
        _send_key(ord('V'), False)
        _send_key(VK_CONTROL, False)


# ─── KeyboardSimulator ───
@final
class KeyboardSimulator:
    @staticmethod
    def backspaces(count: int, delay: float = 0.012) -> None:
        for _ in range(count):
            _send_key(VK_BACK, True)
            _send_key(VK_BACK, False)
            time.sleep(delay)

    @staticmethod
    def space() -> None:
        _send_key(VK_SPACE, True)
        _send_key(VK_SPACE, False)


# ─── LayoutManager ───
@final
class LayoutManager:
    LANG_ARABIC = 0x01
    LANG_ENGLISH = 0x09

    @staticmethod
    def get_current_layout() -> Layout:
        try:
            hwnd = user32.GetForegroundWindow()
            tid = user32.GetWindowThreadProcessId(hwnd, None)
            hkl = user32.GetKeyboardLayout(tid)
            lid = hkl & 0xFFFF
            primary = lid & 0xFF
            if primary == LayoutManager.LANG_ARABIC:
                return Layout.AR
            elif primary == LayoutManager.LANG_ENGLISH:
                return Layout.EN
            return Layout.UNKNOWN
        except Exception as e:
            logger.exception("خطأ في get_current_layout")
            return Layout.UNKNOWN

    @staticmethod
    def switch_layout(layout: Layout) -> bool:
        try:
            hwnd = user32.GetForegroundWindow()
            if layout == Layout.AR:
                hkl = user32.LoadKeyboardLayoutW("00000401", 0x1)
            elif layout == Layout.EN:
                hkl = user32.LoadKeyboardLayoutW("00000409", 0x1)
            else:
                return False
            user32.PostMessageW(hwnd, WM_INPUTLANGCHANGEREQUEST, 0, hkl)
            time.sleep(0.1)
            return True
        except Exception as e:
            logger.exception("خطأ في switch_layout")
            return False


# ─── ConfigManager ───
@final
class ConfigManager:
    _instance: ClassVar[Optional["ConfigManager"]] = None
    _lock = threading.Lock()

    DEFAULT_SETTINGS: ClassVar[dict[str, Any]] = {
        "switch_keyboard": True,
        "show_dialog": True,
        "auto_correct": False,
        "min_word_length": 2,
    }

    def __new__(cls) -> "ConfigManager":
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
                    cls._instance._initialized = False
        return cls._instance

    def __init__(self) -> None:
        if self._initialized:
            return
        self._initialized = True
        self._settings_file = Path.home() / ".mubaddil_v20.json"
        self._settings: dict[str, Any] = {}
        self._load()

    def _load(self) -> None:
        try:
            if self._settings_file.exists():
                with open(self._settings_file, "r", encoding="utf-8") as f:
                    loaded = json.load(f)
                for k, v in self.DEFAULT_SETTINGS.items():
                    self._settings[k] = loaded.get(k, v)
            else:
                self._settings = dict(self.DEFAULT_SETTINGS)
        except (json.JSONDecodeError, OSError) as e:
            logger.warning(f"ملف الإعدادات تالف، استخدم الإعدادات الافتراضية. الخطأ: {e}")
            self._settings = dict(self.DEFAULT_SETTINGS)
            self.save()

    def save(self) -> None:
        try:
            with open(self._settings_file, "w", encoding="utf-8") as f:
                json.dump(self._settings, f, ensure_ascii=False, indent=2)
        except Exception as e:
            logger.exception("فشل حفظ الإعدادات")

    def get(self, key: str, default: Any = None) -> Any:
        return self._settings.get(key, default)

    def set(self, key: str, value: Any) -> None:
        self._settings[key] = value
        self.save()

    def all(self) -> dict[str, Any]:
        return dict(self._settings)


# ─── CorrectionHistory ───
@dataclass
class HistoryEntry:
    original: str
    converted: str
    direction: Direction
    timestamp: str


@final
class CorrectionHistory:
    def __init__(self, max_size: int = 50) -> None:
        self._entries: list[HistoryEntry] = []
        self._max_size = max_size
        self._lock = threading.Lock()
        self._counts = {"en_to_ar": 0, "ar_to_en": 0, "total": 0}

    def add(self, original: str, converted: str, direction: Direction) -> None:
        entry = HistoryEntry(original, converted, direction, time.strftime("%H:%M:%S"))
        with self._lock:
            self._entries.insert(0, entry)
            if len(self._entries) > self._max_size:
                self._entries.pop()
            self._counts["total"] += 1
            self._counts[direction.value] += 1

    def get_all(self) -> list[HistoryEntry]:
        with self._lock:
            return list(self._entries)

    def get_counts(self) -> dict[str, int]:
        with self._lock:
            return dict(self._counts)

    def clear(self) -> None:
        with self._lock:
            self._entries.clear()
            self._counts = {"en_to_ar": 0, "ar_to_en": 0, "total": 0}


# ─── KeyboardMonitor ───
class KeyEventCallback(Protocol):
    def __call__(self, char: str) -> None: ...


@final
class KeyboardMonitor:
    def __init__(self) -> None:
        self._hook_handle: Optional[int] = None
        self._hook_proc = None
        self._running = False
        self._callback: Optional[KeyEventCallback] = None

    def set_callback(self, callback: KeyEventCallback) -> None:
        self._callback = callback

    def start(self) -> bool:
        if self._hook_handle is not None:
            return True

        try:
            self._hook_proc = ctypes.WINFUNCTYPE(
                ctypes.c_int, ctypes.c_int, ctypes.wintypes.WPARAM,
                ctypes.POINTER(KBDLLHOOKSTRUCT)
            )(self._hook_callback)

            self._hook_handle = user32.SetWindowsHookExW(
                WH_KEYBOARD_LL, self._hook_proc, None, 0
            )

            if not self._hook_handle:
                err = ctypes.get_last_error()
                logger.error(f"SetWindowsHookExW فشل. كود الخطأ: {err}")
                return False

            self._running = True
            logger.info("✅ Hook تم تثبيته بنجاح")
            return True

        except Exception as e:
            logger.exception("استثناء في start()")
            return False

    def stop(self) -> None:
        if self._hook_handle:
            user32.UnhookWindowsHookEx(self._hook_handle)
            self._hook_handle = None
        self._hook_proc = None
        self._running = False
        logger.info("🛑 Hook تم إزالته")

    def is_running(self) -> bool:
        return self._running and self._hook_handle is not None

    def _hook_callback(self, n_code: int, w_param: int, l_param: Any) -> int:
        if n_code < 0:
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

        if w_param not in (WM_KEYDOWN, WM_SYSKEYDOWN):
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

        kb = l_param.contents
        vk = kb.vkCode

        control_keys = {
            VK_CONTROL, VK_MENU, VK_SHIFT, VK_ESCAPE, VK_DELETE,
            VK_LCONTROL, VK_RCONTROL, VK_LSHIFT, VK_RSHIFT, VK_LMENU, VK_RMENU,
            VK_BACK, VK_RETURN, VK_TAB,
            0x14, 0x5B, 0x5C,
        }
        if vk in control_keys:
            if vk == VK_ESCAPE and self._callback:
                self._callback('\x1b')
            elif vk == VK_BACK and self._callback:
                self._callback('\b')
            elif vk == VK_RETURN and self._callback:
                self._callback('\r')
            elif vk == VK_TAB and self._callback:
                self._callback('\t')
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

        if (user32.GetKeyState(VK_CONTROL) & 0x8000) != 0:
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)
        if (user32.GetKeyState(VK_MENU) & 0x8000) != 0 and w_param == WM_SYSKEYDOWN:
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

        if not self._running or not self._callback:
            return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

        try:
            kb_state = (ctypes.c_ubyte * 256)()
            if not user32.GetKeyboardState(ctypes.byref(kb_state)):
                return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)

            kb_state[vk] |= 0x80
            if vk in (VK_LSHIFT, VK_RSHIFT):
                kb_state[VK_SHIFT] |= 0x80
            elif vk in (VK_LCONTROL, VK_RCONTROL):
                kb_state[VK_CONTROL] |= 0x80
            elif vk in (VK_LMENU, VK_RMENU):
                kb_state[VK_MENU] |= 0x80

            hwnd = user32.GetForegroundWindow()
            tid = user32.GetWindowThreadProcessId(hwnd, None)
            hkl = user32.GetKeyboardLayout(tid)

            buf = (ctypes.c_wchar * 8)()
            ret = user32.ToUnicodeEx(vk, kb.scanCode, kb_state, buf, 8, 0, hkl)

            if ret > 0:
                char = buf[0]
                if char and (char.isprintable() or ord(char) >= 0x0600):
                    self._callback(char)
            elif ret < 0:
                user32.ToUnicodeEx(vk, kb.scanCode, kb_state, buf, 8, 0, hkl)

        except Exception as e:
            logger.exception("خطأ في hook_callback")

        return user32.CallNextHookEx(self._hook_handle, n_code, w_param, l_param)


# ─── CorrectionApplier ───
@final
class CorrectionApplier:
    def __init__(self) -> None:
        self._config = ConfigManager()

    def apply(self, original: str, suggested: str, direction: Direction, target_hwnd: int = 0) -> bool:
        def _do_apply():
            try:
                if target_hwnd:
                    user32.SetForegroundWindow(target_hwnd)
                    time.sleep(0.05)

                if self._config.get("switch_keyboard", True):
                    if direction == Direction.EN_TO_AR:
                        LayoutManager.switch_layout(Layout.AR)
                    else:
                        LayoutManager.switch_layout(Layout.EN)
                    time.sleep(0.06)

                KeyboardSimulator.backspaces(len(original))
                time.sleep(0.04)

                ClipboardManager.set_text(suggested)
                time.sleep(0.03)
                ClipboardManager.paste()
                time.sleep(0.03)

                KeyboardSimulator.space()
            except Exception as e:
                logger.exception("خطأ في تطبيق التصحيح")

        threading.Thread(target=_do_apply, daemon=True).start()
        return True


# ─── CoreEventCallback ───
class CoreEventCallback(Protocol):
    def __call__(self, event_type: str, data: dict[str, Any]) -> None: ...


# ─── MubaddilCore ───
@final
class MubaddilCore:
    def __init__(self) -> None:
        KeyboardMapper()
        self._config = ConfigManager()
        self._history = CorrectionHistory()
        self._converter = ConverterEngine()
        self._suggestion = SuggestionEngine(self._converter)
        self._monitor = KeyboardMonitor()
        self._applier = CorrectionApplier()
        self._current_word = ""
        self._word_lock = threading.Lock()
        self._last_correction_time = 0.0
        self._correction_lock = threading.Lock()
        self._ui_callback: Optional[CoreEventCallback] = None
        self._monitor.set_callback(self._on_key)
        self._last_target_hwnd: int = 0

    def set_ui_callback(self, callback: CoreEventCallback) -> None:
        self._ui_callback = callback

    def start(self) -> bool:
        result = self._monitor.start()
        if result:
            logger.info("✅ MubaddilCore بدأ بنجاح")
        else:
            logger.error("❌ MubaddilCore فشل في البدء")
        return result

    def stop(self) -> None:
        self._monitor.stop()

    def is_monitoring(self) -> bool:
        return self._monitor.is_running()

    def toggle_monitoring(self) -> bool:
        if self._monitor.is_running():
            self._monitor.stop()
            return False
        else:
            return self._monitor.start()

    def get_history(self) -> list[HistoryEntry]:
        return self._history.get_all()

    def get_counts(self) -> dict[str, int]:
        return self._history.get_counts()

    def clear_history(self) -> None:
        self._history.clear()

    def get_rejected_words(self) -> list[str]:
        return WordValidator.get_rejected_words()

    def clear_rejected(self) -> None:
        WordValidator.clear_rejected()

    def get_config(self) -> ConfigManager:
        return self._config

    def _on_key(self, char: str) -> None:
        with self._word_lock:
            if char == ' ':
                word = self._current_word.strip()
                self._current_word = ""
                self._last_target_hwnd = user32.GetForegroundWindow()
                if word:
                    logger.debug(f"Word completed: '{word}'")
                    threading.Thread(target=self._process_word, args=(word,), daemon=True).start()
            elif char in ('\r', '\t', '\n'):
                word = self._current_word.strip()
                self._current_word = ""
                self._last_target_hwnd = user32.GetForegroundWindow()
                if word:
                    logger.debug(f"Word completed (enter/tab): '{word}'")
                    threading.Thread(target=self._process_word, args=(word,), daemon=True).start()
            elif char == '\x1b':
                self._current_word = ""
                logger.debug("Buffer cleared (Escape)")
            elif char == '\b':
                if self._current_word:
                    self._current_word = self._current_word[:-1]
            elif char.isprintable() or ord(char) >= 0x0600:
                self._current_word += char
            else:
                self._current_word = ""

    def _process_word(self, word: str) -> None:
        with self._correction_lock:
            now = time.time()
            if now - self._last_correction_time < 0.6:
                logger.debug(f"Debounced: '{word}'")
                return
            self._last_correction_time = now

        layout = LayoutManager.get_current_layout()
        logger.debug(f"Processing word='{word}' layout={layout.value}")

        result = self._suggestion.suggest(word, layout)
        logger.debug(f"Suggestion: original='{result.original}' suggested='{result.suggested}' "
                     f"direction={result.direction.value} confidence={result.confidence} should_show={result.should_show}")

        if result.should_show and result.suggested != word:
            if self._ui_callback:
                logger.debug("Sending suggestion to UI")
                self._ui_callback("suggestion", {
                    "original": result.original,
                    "suggested": result.suggested,
                    "direction": result.direction,
                    "confidence": result.confidence,
                    "hwnd": self._last_target_hwnd,
                })
            else:
                logger.warning("No UI callback registered!")
        else:
            logger.debug(f"No suggestion for '{word}'")

    def accept_correction(self, original: str, suggested: str, direction: Direction, target_hwnd: int = 0) -> None:
        logger.debug(f"Accepting correction: '{original}' -> '{suggested}'")
        success = self._applier.apply(original, suggested, direction, target_hwnd or self._last_target_hwnd)
        if success:
            self._history.add(original, suggested, direction)
            if self._ui_callback:
                self._ui_callback("corrected", {
                    "original": original,
                    "suggested": suggested,
                    "direction": direction,
                })
        else:
            logger.warning("Correction application failed")

    def reject_correction(self, word: str) -> None:
        logger.debug(f"Rejecting correction for: '{word}'")
        WordValidator.add_rejected(word)
        if self._ui_callback:
            self._ui_callback("rejected", {"word": word})
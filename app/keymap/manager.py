"""
Mobadel Keymap Manager
Handles bidirectional keyboard layout mapping between English QWERTY and Arabic 101.
"""

import json
from pathlib import Path
from typing import Dict, Optional
from dataclasses import dataclass, field


@dataclass(frozen=True)
class KeyMapping:
    """Represents a single key mapping between layouts."""
    vk_code: str
    en_normal: str
    en_shift: str
    ar_normal: str
    ar_shift: str


class KeymapManager:
    """
    Manages bidirectional keyboard layout mappings.
    
    Provides methods to convert text between English and Arabic keyboard layouts
    based on physical key positions.
    """
    
    _instance: Optional['KeymapManager'] = None
    _initialized: bool = False
    
    def __new__(cls) -> 'KeymapManager':
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self) -> None:
        if KeymapManager._initialized:
            return
        
        self._en_to_ar: Dict[str, str] = {}
        self._ar_to_en: Dict[str, str] = {}
        self._arabic_chars: set = set()
        self._english_chars: set = set()
        self._modifiers: Dict[str, str] = {}
        
        self._load_keymap()
        KeymapManager._initialized = True
    
    def _load_keymap(self) -> None:
        """Load keyboard mapping from JSON file."""
        keymap_path = Path(__file__).parent.parent.parent / 'data' / 'keymaps' / 'en_ar.json'
        
        if not keymap_path.exists():
            # Fallback to inline mapping if file doesn't exist
            self._build_fallback_maps()
            return
        
        try:
            with open(keymap_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            mappings = data.get('mappings', {})
            en_to_ar_map = mappings.get('en_to_ar_map', {})
            ar_to_en_map = mappings.get('ar_to_en_map', {})
            
            self._en_to_ar = en_to_ar_map
            self._ar_to_en = ar_to_en_map
            self._modifiers = data.get('modifiers', {})
            
            # Build character sets
            self._arabic_chars = set(ar_to_en_map.keys())
            self._english_chars = set(en_to_ar_map.keys())
            
        except (json.JSONDecodeError, IOError) as e:
            print(f"Warning: Could not load keymap file: {e}")
            self._build_fallback_maps()
    
    def _build_fallback_maps(self) -> None:
        """Build fallback keyboard mappings if file loading fails."""
        # Saudi Arabic 101 keyboard layout
        self._en_to_ar = {
            'q': 'ض', 'w': 'ص', 'e': 'ث', 'r': 'ق', 't': 'ف',
            'y': 'غ', 'u': 'ع', 'i': 'ه', 'o': 'خ', 'p': 'ح',
            '[': 'ج', ']': 'د', 'a': 'ش', 's': 'س', 'd': 'ي',
            'f': 'ب', 'g': 'ل', 'h': 'ا', 'j': 'ت', 'k': 'ن',
            'l': 'م', ';': 'ك', "'": 'ط', 'z': 'ئ', 'x': 'ء',
            'c': 'ؤ', 'v': 'ر', 'b': 'لا', 'n': 'ى', 'm': 'ة',
            ',': 'و', '.': 'ز', '/': 'ظ', '-': '-', '=': '=',
            '\\': '\\', '`': 'ذ',
            'Q': 'َ', 'W': 'ً', 'E': 'ُ', 'R': 'ٌ', 'T': 'ل',
            'Y': 'إ', 'U': "'", 'I': '÷', 'O': '×', 'P': '؛',
            '{': '<', '}': '>', 'A': 'ِ', 'S': 'ٍ', 'D': ']',
            'F': '[', 'G': 'ل', 'H': 'أ', 'J': 'ـ', 'K': ',',
            'L': '/', ':': ':', '"': '"', 'Z': '~', 'X': 'ْ',
            'C': '}', 'V': '{', 'B': 'آ', 'N': "'", 'M': "'",
            '<': ',', '>': '.', '?': '?', '_': '_', '+': '+',
            '|': '|', '~': 'ّ',
            '0': '٠', '1': '١', '2': '٢', '3': '٣', '4': '٤',
            '5': '٥', '6': '٦', '7': '٧', '8': '٨', '9': '٩',
        }
        
        self._ar_to_en = {v: k for k, v in self._en_to_ar.items()}
        self._arabic_chars = set(self._ar_to_en.keys())
        self._english_chars = set(self._en_to_ar.keys())
    
    def convert_en_to_ar(self, text: str) -> str:
        """
        Convert text typed with English layout to Arabic.
        
        Args:
            text: Text typed assuming English layout
            
        Returns:
            Text converted to Arabic characters
        """
        return ''.join(self._en_to_ar.get(ch, ch) for ch in text)
    
    def convert_ar_to_en(self, text: str) -> str:
        """
        Convert text typed with Arabic layout to English.
        
        Args:
            text: Text typed assuming Arabic layout
            
        Returns:
            Text converted to English characters
        """
        return ''.join(self._ar_to_en.get(ch, ch) for ch in text)
    
    def is_arabic_char(self, char: str) -> bool:
        """Check if a character is in the Arabic keyboard mapping."""
        return char in self._arabic_chars
    
    def is_english_char(self, char: str) -> bool:
        """Check if a character is in the English keyboard mapping."""
        return char in self._english_chars
    
    def get_modifier_vk(self, modifier_name: str) -> Optional[str]:
        """Get the virtual key code for a modifier."""
        return self._modifiers.get(modifier_name)
    
    @property
    def en_to_ar_map(self) -> Dict[str, str]:
        """Get the English to Arabic mapping dictionary."""
        return self._en_to_ar.copy()
    
    @property
    def ar_to_en_map(self) -> Dict[str, str]:
        """Get the Arabic to English mapping dictionary."""
        return self._ar_to_en.copy()
    
    @classmethod
    def reset(cls) -> None:
        """Reset the singleton instance (useful for testing)."""
        cls._instance = None
        cls._initialized = False

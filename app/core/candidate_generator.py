"""
Mobadel Candidate Generator
Generates correction candidates for typed words.
"""

from typing import List, Dict, Optional, NamedTuple
from dataclasses import dataclass
import re


@dataclass
class CorrectionCandidate:
    """Represents a potential correction candidate."""
    original: str
    mapped_arabic: str  # What it would be if typed on English layout but meant Arabic
    mapped_english: str  # What it would be if typed on Arabic layout but meant English
    direction: str  # 'en_to_ar' or 'ar_to_en' or 'unknown'
    
    def to_dict(self) -> Dict:
        return {
            "original": self.original,
            "mapped_arabic": self.mapped_arabic,
            "mapped_english": self.mapped_english,
            "direction": self.direction,
        }


class BypassPatterns:
    """Regex patterns for text that should bypass correction."""
    
    # URL patterns
    URL = re.compile(
        r'^(https?://|www\.)[^\s]+$',
        re.IGNORECASE
    )
    
    # Email pattern
    EMAIL = re.compile(
        r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$',
        re.IGNORECASE
    )
    
    # File paths (Windows and Unix)
    FILE_PATH = re.compile(
        r'^([a-zA-Z]:\\|[/.]).*[\\/.][^\s]*$'
    )
    
    # Programming identifiers
    CAMEL_CASE = re.compile(r'^[a-z][a-zA-Z0-9]*[A-Z][a-zA-Z0-9]*$')
    PASCAL_CASE = re.compile(r'^[A-Z][a-zA-Z0-9]*$')
    SNAKE_CASE = re.compile(r'^[a-z]+(_[a-z0-9]+)+$')
    CONSTANT_CASE = re.compile(r'^[A-Z](_[A-Z0-9]+)*$')
    
    # Special prefixes
    PREFIX_HASH = re.compile(r'^#[a-zA-Z0-9_]+$')  # Hashtags
    PREFIX_DOLLAR = re.compile(r'^\$[a-zA-Z0-9_]+$')  # Variables
    PREFIX_AT = re.compile(r'^@[a-zA-Z0-9_]+$')  # Usernames
    
    # Pure numeric / special patterns
    PURE_NUMERIC = re.compile(r'^[0-9]+$')
    UUID = re.compile(
        r'^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$'
    )
    HEX_COLOR = re.compile(r'^#[0-9a-fA-F]{6}$')
    
    # Mixed alphanumeric with special chars (likely code)
    CODE_LIKE = re.compile(r'^[a-zA-Z0-9_{}[\]()<>;:\'"\\|&*+=~`!@#$%^?-]+$')


class CandidateGenerator:
    """
    Generates correction candidates for typed words.
    
    Creates multiple candidate corrections based on keyboard layout
    mappings and applies bypass rules to filter out text that shouldn't
    be corrected (URLs, emails, code, etc.).
    """
    
    def __init__(self, keymap_manager=None):
        """
        Initialize the candidate generator.
        
        Args:
            keymap_manager: KeymapManager instance for layout conversion
        """
        self._keymap = keymap_manager
        self._bypass_patterns = BypassPatterns()
    
    def should_bypass(self, word: str) -> bool:
        """
        Check if a word should bypass correction entirely.
        
        Args:
            word: The word to check
            
        Returns:
            True if the word should not be corrected
        """
        if len(word) < 2:
            return True
        
        # Check all bypass patterns
        if self._bypass_patterns.URL.match(word):
            return True
        if self._bypass_patterns.EMAIL.match(word):
            return True
        if self._bypass_patterns.FILE_PATH.match(word):
            return True
        if self._bypass_patterns.UUID.match(word):
            return True
        if self._bypass_patterns.HEX_COLOR.match(word):
            return True
        if self._bypass_patterns.PURE_NUMERIC.match(word):
            return True
        
        # Check programming patterns
        if self._bypass_patterns.CAMEL_CASE.match(word):
            return True
        if self._bypass_patterns.PASCAL_CASE.match(word):
            return True
        if self._bypass_patterns.SNAKE_CASE.match(word):
            return True
        if self._bypass_patterns.CONSTANT_CASE.match(word):
            return True
        
        # Check special prefixes
        if self._bypass_patterns.PREFIX_HASH.match(word):
            return True
        if self._bypass_patterns.PREFIX_DOLLAR.match(word):
            return True
        if self._bypass_patterns.PREFIX_AT.match(word):
            return True
        
        # Check if it looks like code
        if self._bypass_patterns.CODE_LIKE.match(word) and any(c in word for c in '{}[]();=<>'):
            return True
        
        return False
    
    def generate_candidates(self, word: str) -> List[CorrectionCandidate]:
        """
        Generate correction candidates for a word.
        
        Args:
            word: The typed word
            
        Returns:
            List of CorrectionCandidate objects
        """
        candidates = []
        
        # First check if we should bypass entirely
        if self.should_bypass(word):
            return candidates
        
        # Get conversions from keymap
        if self._keymap is not None:
            mapped_arabic = self._keymap.convert_en_to_ar(word)
            mapped_english = self._keymap.convert_ar_to_en(word)
        else:
            # Fallback inline conversion if no keymap
            mapped_arabic = self._convert_en_to_ar_fallback(word)
            mapped_english = self._convert_ar_to_en_fallback(word)
        
        # Only add candidates if conversion actually changed something
        if mapped_arabic != word:
            candidates.append(CorrectionCandidate(
                original=word,
                mapped_arabic=mapped_arabic,
                mapped_english=word,  # No change expected
                direction='en_to_ar'
            ))
        
        if mapped_english != word:
            candidates.append(CorrectionCandidate(
                original=word,
                mapped_arabic=word,  # No change expected
                mapped_english=mapped_english,
                direction='ar_to_en'
            ))
        
        return candidates
    
    def _convert_en_to_ar_fallback(self, text: str) -> str:
        """Fallback English to Arabic conversion."""
        mapping = {
            'q': 'ض', 'w': 'ص', 'e': 'ث', 'r': 'ق', 't': 'ف',
            'y': 'غ', 'u': 'ع', 'i': 'ه', 'o': 'خ', 'p': 'ح',
            '[': 'ج', ']': 'د', 'a': 'ش', 's': 'س', 'd': 'ي',
            'f': 'ب', 'g': 'ل', 'h': 'ا', 'j': 'ت', 'k': 'ن',
            'l': 'م', ';': 'ك', "'": 'ط', 'z': 'ئ', 'x': 'ء',
            'c': 'ؤ', 'v': 'ر', 'b': 'لا', 'n': 'ى', 'm': 'ة',
            ',': 'و', '.': 'ز', '/': 'ظ',
        }
        return ''.join(mapping.get(ch, ch) for ch in text.lower())
    
    def _convert_ar_to_en_fallback(self, text: str) -> str:
        """Fallback Arabic to English conversion."""
        mapping = {
            'ض': 'q', 'ص': 'w', 'ث': 'e', 'ق': 'r', 'ف': 't',
            'غ': 'y', 'ع': 'u', 'ه': 'i', 'خ': 'o', 'ح': 'p',
            'ج': '[', 'د': ']', 'ش': 'a', 'س': 's', 'ي': 'd',
            'ب': 'f', 'ل': 'g', 'ا': 'h', 'ت': 'j', 'ن': 'k',
            'م': 'l', 'ك': ';', 'ط': "'", 'ئ': 'z', 'ء': 'x',
            'ؤ': 'c', 'ر': 'v', 'لا': 'b', 'ى': 'n', 'ة': 'm',
            'و': ',', 'ز': '.', 'ظ': '/',
        }
        return ''.join(mapping.get(ch, ch) for ch in text)
    
    def get_primary_candidate(self, word: str) -> Optional[CorrectionCandidate]:
        """
        Get the most likely correction candidate for a word.
        
        Args:
            word: The typed word
            
        Returns:
            The primary candidate or None if no candidates
        """
        candidates = self.generate_candidates(word)
        return candidates[0] if candidates else None

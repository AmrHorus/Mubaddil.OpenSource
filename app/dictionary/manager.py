"""
Mobadel Dictionary Manager
Handles loading and querying word dictionaries for Arabic and English.
"""

import json
from pathlib import Path
from typing import Set, Dict, Optional, List
from dataclasses import dataclass


@dataclass
class DictionaryStats:
    """Statistics about a loaded dictionary."""
    word_count: int
    max_word_length: int
    min_word_length: int
    avg_word_length: float


class DictionaryManager:
    """
    Manages word dictionaries for validation and scoring.
    
    Supports loading dictionaries from text files (one word per line)
    and provides fast lookup via hash sets.
    """
    
    _instance: Optional['DictionaryManager'] = None
    _initialized: bool = False
    
    def __new__(cls) -> 'DictionaryManager':
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self) -> None:
        if DictionaryManager._initialized:
            return
        
        self._arabic_words: Set[str] = set()
        self._english_words: Set[str] = set()
        self._arabic_stats: Optional[DictionaryStats] = None
        self._english_stats: Optional[DictionaryStats] = None
        
        self._load_dictionaries()
        DictionaryManager._initialized = True
    
    def _load_dictionaries(self) -> None:
        """Load Arabic and English dictionaries from files."""
        data_dir = Path(__file__).parent.parent.parent / 'data' / 'dictionaries'
        
        # Load Arabic dictionary
        ar_dict_path = data_dir / 'arabic_words.txt'
        if ar_dict_path.exists():
            self._load_text_dictionary(ar_dict_path, 'arabic')
        
        # Load English dictionary
        en_dict_path = data_dir / 'english_words.txt'
        if en_dict_path.exists():
            self._load_text_dictionary(en_dict_path, 'english')
    
    def _load_text_dictionary(self, path: Path, lang: str) -> None:
        """Load a dictionary from a text file (one word per line)."""
        try:
            words = set()
            word_lengths = []
            
            with open(path, 'r', encoding='utf-8') as f:
                for line in f:
                    word = line.strip()
                    # Skip comments and empty lines
                    if not word or word.startswith('#'):
                        continue
                    words.add(word)
                    word_lengths.append(len(word))
            
            if lang == 'arabic':
                self._arabic_words = words
                if word_lengths:
                    self._arabic_stats = DictionaryStats(
                        word_count=len(words),
                        max_word_length=max(word_lengths),
                        min_word_length=min(word_lengths),
                        avg_word_length=sum(word_lengths) / len(word_lengths)
                    )
            else:
                self._english_words = words
                if word_lengths:
                    self._english_stats = DictionaryStats(
                        word_count=len(words),
                        max_word_length=max(word_lengths),
                        min_word_length=min(word_lengths),
                        avg_word_length=sum(word_lengths) / len(word_lengths)
                    )
                    
        except IOError as e:
            print(f"Warning: Could not load {lang} dictionary: {e}")
    
    def is_arabic_word(self, word: str) -> bool:
        """Check if a word exists in the Arabic dictionary."""
        return word in self._arabic_words
    
    def is_english_word(self, word: str) -> bool:
        """Check if a word exists in the English dictionary."""
        return word.lower() in self._english_words
    
    def get_arabic_words(self) -> Set[str]:
        """Get a copy of all Arabic words."""
        return self._arabic_words.copy()
    
    def get_english_words(self) -> Set[str]:
        """Get a copy of all English words."""
        return self._english_words.copy()
    
    def add_arabic_word(self, word: str) -> None:
        """Add a word to the Arabic dictionary."""
        self._arabic_words.add(word)
    
    def add_english_word(self, word: str) -> None:
        """Add a word to the English dictionary."""
        self._english_words.add(word.lower())
    
    def get_arabic_stats(self) -> Optional[DictionaryStats]:
        """Get statistics about the Arabic dictionary."""
        return self._arabic_stats
    
    def get_english_stats(self) -> Optional[DictionaryStats]:
        """Get statistics about the English dictionary."""
        return self._english_stats
    
    def contains(self, word: str, language: str) -> bool:
        """
        Check if a word exists in the specified language dictionary.
        
        Args:
            word: The word to check
            language: 'arabic' or 'english'
            
        Returns:
            True if word exists in the dictionary
        """
        if language == 'arabic':
            return self.is_arabic_word(word)
        elif language == 'english':
            return self.is_english_word(word)
        return False
    
    @classmethod
    def reset(cls) -> None:
        """Reset the singleton instance."""
        cls._instance = None
        cls._initialized = False

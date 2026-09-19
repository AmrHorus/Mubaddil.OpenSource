"""
Mobadel Frequency Engine
Handles word frequency scoring based on local unigram data.
"""

import json
import math
from pathlib import Path
from typing import Dict, Optional


class FrequencyEngine:
    """
    Provides frequency-based scoring for words.
    
    Uses log-normalized frequency scores from local corpus data
    to determine how common a word is in its language.
    """
    
    _instance: Optional['FrequencyEngine'] = None
    _initialized: bool = False
    
    def __new__(cls) -> 'FrequencyEngine':
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self) -> None:
        if FrequencyEngine._initialized:
            return
        
        self._arabic_frequencies: Dict[str, int] = {}
        self._english_frequencies: Dict[str, int] = {}
        self._arabic_total: int = 0
        self._english_total: int = 0
        self._arabic_max_freq: int = 1
        self._english_max_freq: int = 1
        
        self._load_frequencies()
        FrequencyEngine._initialized = True
    
    def _load_frequencies(self) -> None:
        """Load frequency data from JSON files."""
        data_dir = Path(__file__).parent.parent.parent / 'data' / 'frequencies'
        
        # Load Arabic frequencies
        ar_freq_path = data_dir / 'arabic_frequency.json'
        if ar_freq_path.exists():
            self._load_frequency_file(ar_freq_path, 'arabic')
        
        # Load English frequencies
        en_freq_path = data_dir / 'english_frequency.json'
        if en_freq_path.exists():
            self._load_frequency_file(en_freq_path, 'english')
    
    def _load_frequency_file(self, path: Path, lang: str) -> None:
        """Load frequency data from a JSON file."""
        try:
            with open(path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            frequencies = data.get('frequencies', {})
            
            if lang == 'arabic':
                self._arabic_frequencies = frequencies
                self._arabic_total = data.get('total_words', sum(frequencies.values()))
                self._arabic_max_freq = max(frequencies.values()) if frequencies else 1
            else:
                self._english_frequencies = {k.lower(): v for k, v in frequencies.items()}
                self._english_total = data.get('total_words', sum(frequencies.values()))
                self._english_max_freq = max(frequencies.values()) if frequencies else 1
                
        except (json.JSONDecodeError, IOError) as e:
            print(f"Warning: Could not load frequency file {path}: {e}")
    
    def get_frequency(self, word: str, language: str) -> int:
        """
        Get the raw frequency count for a word.
        
        Args:
            word: The word to look up
            language: 'arabic' or 'english'
            
        Returns:
            Frequency count, or 0 if not found
        """
        if language == 'arabic':
            return self._arabic_frequencies.get(word, 0)
        elif language == 'english':
            return self._english_frequencies.get(word.lower(), 0)
        return 0
    
    def get_normalized_score(self, word: str, language: str) -> float:
        """
        Get a normalized frequency score (0.0 to 1.0).
        
        Uses log normalization to prevent very common words from
        dominating the score completely.
        
        Args:
            word: The word to score
            language: 'arabic' or 'english'
            
        Returns:
            Normalized score between 0.0 and 1.0
        """
        freq = self.get_frequency(word, language)
        
        if freq == 0:
            return 0.0
        
        if language == 'arabic':
            max_freq = self._arabic_max_freq
        else:
            max_freq = self._english_max_freq
        
        # Log normalization: score = log(1 + freq) / log(1 + max_freq)
        score = math.log1p(freq) / math.log1p(max_freq)
        return min(score, 1.0)
    
    def get_percentile(self, word: str, language: str) -> float:
        """
        Get the frequency percentile for a word.
        
        Args:
            word: The word to analyze
            language: 'arabic' or 'english'
            
        Returns:
            Percentile rank (0.0 to 1.0)
        """
        freq = self.get_frequency(word, language)
        
        if freq == 0:
            return 0.0
        
        if language == 'arabic':
            frequencies = self._arabic_frequencies
            total = self._arabic_total
        else:
            frequencies = self._english_frequencies
            total = self._english_total
        
        if total == 0:
            return 0.0
        
        # Simple percentile: what fraction of all word occurrences is this word?
        return freq / total
    
    def is_common_word(self, word: str, language: str, threshold: float = 0.5) -> bool:
        """
        Check if a word is considered common based on frequency.
        
        Args:
            word: The word to check
            language: 'arabic' or 'english'
            threshold: Normalized score threshold (default 0.5)
            
        Returns:
            True if word is common
        """
        score = self.get_normalized_score(word, language)
        return score >= threshold
    
    def get_top_words(self, language: str, count: int = 10) -> list:
        """
        Get the most frequent words in a language.
        
        Args:
            language: 'arabic' or 'english'
            count: Number of top words to return
            
        Returns:
            List of (word, frequency) tuples
        """
        if language == 'arabic':
            frequencies = self._arabic_frequencies
        else:
            frequencies = self._english_frequencies
        
        sorted_words = sorted(frequencies.items(), key=lambda x: x[1], reverse=True)
        return sorted_words[:count]
    
    @classmethod
    def reset(cls) -> None:
        """Reset the singleton instance."""
        cls._instance = None
        cls._initialized = False

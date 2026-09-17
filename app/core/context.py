"""
Mobadel Context Analyzer
Analyzes text context for better correction decisions.
"""

from typing import List, Tuple, Optional
from collections import deque


class ContextAnalyzer:
    """
    Analyzes surrounding text context for correction scoring.
    
    Provides bi-gram and tri-gram analysis to improve
    correction accuracy based on word co-occurrence patterns.
    """
    
    def __init__(self, max_context_words: int = 5):
        """
        Initialize the context analyzer.
        
        Args:
            max_context_words: Maximum number of context words to track
        """
        self._max_context = max_context_words
        self._recent_words: deque = deque(maxlen=max_context_words)
        self._bigram_counts: dict = {}
        self._trigram_counts: dict = {}
    
    def add_word(self, word: str) -> None:
        """
        Add a word to the recent context history.
        
        Args:
            word: The word that was typed/confirmed
        """
        self._recent_words.append(word)
        
        # Update n-gram counts
        if len(self._recent_words) >= 2:
            bigram = tuple(self._recent_words)[-2:]
            self._bigram_counts[bigram] = self._bigram_counts.get(bigram, 0) + 1
        
        if len(self._recent_words) >= 3:
            trigram = tuple(self._recent_words)[-3:]
            self._trigram_counts[trigram] = self._trigram_counts.get(trigram, 0) + 1
    
    def get_previous_word(self, n: int = 1) -> Optional[str]:
        """
        Get the nth previous word.
        
        Args:
            n: How many words back (1 = immediate previous)
            
        Returns:
            The previous word or None if not enough context
        """
        if len(self._recent_words) >= n:
            return self._recent_words[-n]
        return None
    
    def get_context_words(self, count: int = 3) -> List[str]:
        """
        Get the most recent context words.
        
        Args:
            count: Number of words to return
            
        Returns:
            List of recent words (most recent last)
        """
        return list(self._recent_words)[-count:]
    
    def get_bigram_probability(self, word1: str, word2: str) -> float:
        """
        Estimate probability of word2 following word1.
        
        Args:
            word1: First word
            word2: Second word
            
        Returns:
            Probability estimate (simple frequency-based)
        """
        bigram = (word1, word2)
        bigram_count = self._bigram_counts.get(bigram, 0)
        
        # Count how many times word1 appears as first part of any bigram
        word1_count = sum(
            count for (w1, _), count in self._bigram_counts.items() 
            if w1 == word1
        )
        
        if word1_count == 0:
            return 0.0
        
        return bigram_count / word1_count
    
    def get_trigram_probability(self, word1: str, word2: str, word3: str) -> float:
        """
        Estimate probability of word3 following (word1, word2).
        
        Args:
            word1: First word
            word2: Second word
            word3: Third word
            
        Returns:
            Probability estimate
        """
        trigram = (word1, word2, word3)
        trigram_count = self._trigram_counts.get(trigram, 0)
        
        # Count how many times (word1, word2) appears
        bigram_count = sum(
            count for (w1, w2, _), count in self._trigram_counts.items()
            if w1 == word1 and w2 == word2
        )
        
        if bigram_count == 0:
            return 0.0
        
        return trigram_count / bigram_count
    
    def is_likely_sequence(self, words: List[str]) -> bool:
        """
        Check if a sequence of words is likely based on history.
        
        Args:
            words: Sequence of words to check
            
        Returns:
            True if sequence appears valid
        """
        if len(words) < 2:
            return True
        
        if len(words) == 2:
            prob = self.get_bigram_probability(words[0], words[1])
            return prob > 0.0
        
        if len(words) == 3:
            prob = self.get_trigram_probability(words[0], words[1], words[2])
            return prob > 0.0
        
        # For longer sequences, check all overlapping trigrams
        for i in range(len(words) - 2):
            trigram_prob = self.get_trigram_probability(
                words[i], words[i+1], words[i+2]
            )
            if trigram_prob == 0:
                return False
        
        return True
    
    def clear_history(self) -> None:
        """Clear all context history."""
        self._recent_words.clear()
        self._bigram_counts.clear()
        self._trigram_counts.clear()
    
    def get_statistics(self) -> dict:
        """Get statistics about the context history."""
        return {
            "recent_words_count": len(self._recent_words),
            "unique_bigrams": len(self._bigram_counts),
            "unique_trigrams": len(self._trigram_counts),
            "total_bigram_occurrences": sum(self._bigram_counts.values()),
            "total_trigram_occurrences": sum(self._trigram_counts.values()),
        }


class LanguageDetector:
    """
    Detects the dominant language of a text segment.
    
    Uses character set analysis to determine if text
    is primarily Arabic, English, or mixed.
    """
    
    ARABIC_RANGE = range(0x0600, 0x06FF + 1)
    ARABIC_PRESENTATION_A = range(0xFB50, 0xFDFF + 1)
    ARABIC_PRESENTATION_B = range(0xFE70, 0xFEFF + 1)
    
    @classmethod
    def is_arabic_char(cls, char: str) -> bool:
        """Check if a character is Arabic."""
        code = ord(char)
        return (
            cls.ARABIC_RANGE.start <= code <= cls.ARABIC_RANGE.stop or
            cls.ARABIC_PRESENTATION_A.start <= code <= cls.ARABIC_PRESENTATION_A.stop or
            cls.ARABIC_PRESENTATION_B.start <= code <= cls.ARABIC_PRESENTATION_B.stop
        )
    
    @classmethod
    def is_english_char(cls, char: str) -> bool:
        """Check if a character is an English letter."""
        return 'a' <= char.lower() <= 'z'
    
    @classmethod
    def analyze(cls, text: str) -> dict:
        """
        Analyze text to determine language composition.
        
        Args:
            text: Text to analyze
            
        Returns:
            Dictionary with analysis results
        """
        if not text:
            return {
                "arabic_count": 0,
                "english_count": 0,
                "other_count": 0,
                "arabic_ratio": 0.0,
                "english_ratio": 0.0,
                "dominant": "unknown",
                "is_mixed": False,
            }
        
        arabic_count = 0
        english_count = 0
        other_count = 0
        
        for char in text:
            if cls.is_arabic_char(char):
                arabic_count += 1
            elif cls.is_english_char(char):
                english_count += 1
            else:
                other_count += 1
        
        total = len(text)
        arabic_ratio = arabic_count / total if total > 0 else 0.0
        english_ratio = english_count / total if total > 0 else 0.0
        
        if arabic_ratio > english_ratio and arabic_ratio > 0:
            dominant = "arabic"
        elif english_ratio > arabic_ratio and english_ratio > 0:
            dominant = "english"
        else:
            dominant = "unknown"
        
        is_mixed = arabic_count > 0 and english_count > 0
        
        return {
            "arabic_count": arabic_count,
            "english_count": english_count,
            "other_count": other_count,
            "arabic_ratio": arabic_ratio,
            "english_ratio": english_ratio,
            "dominant": dominant,
            "is_mixed": is_mixed,
        }
    
    @classmethod
    def detect_word_language(cls, word: str) -> str:
        """
        Detect the primary language of a single word.
        
        Args:
            word: Word to analyze
            
        Returns:
            'arabic', 'english', or 'mixed'
        """
        result = cls.analyze(word)
        
        if result["is_mixed"]:
            return "mixed"
        return result["dominant"]

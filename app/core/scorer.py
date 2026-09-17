"""
Mobadel Deterministic Scorer
Multi-factor scoring engine for correction candidates.
"""

from typing import Dict, Optional, Any
from dataclasses import dataclass


@dataclass
class ScoringWeights:
    """Configurable weights for the scoring formula."""
    dictionary: float = 0.35
    frequency: float = 0.25
    keyboard_map: float = 0.20
    context: float = 0.10
    history: float = 0.10
    
    def normalize(self) -> 'ScoringWeights':
        """Normalize weights to sum to 1.0."""
        total = (self.dictionary + self.frequency + 
                 self.keyboard_map + self.context + self.history)
        if total > 0:
            return ScoringWeights(
                dictionary=self.dictionary / total,
                frequency=self.frequency / total,
                keyboard_map=self.keyboard_map / total,
                context=self.context / total,
                history=self.history / total,
            )
        return self


@dataclass
class ScoreResult:
    """Result of scoring a correction candidate."""
    total_score: float
    dictionary_score: float
    frequency_score: float
    keyboard_map_score: float
    context_score: float
    history_score: float
    should_auto_correct: bool
    should_suggest: bool
    confidence_tier: str  # 'high', 'medium', 'low'


class DeterministicScorer:
    """
    Multi-factor deterministic scoring engine.
    
    Implements the scoring formula:
    Score = (W_dict * S_dict) + (W_freq * S_freq) + (W_map * S_map) + 
            (W_ctx * S_ctx) + (W_hist * S_hist)
    
    All scores are normalized to [0.0, 1.0] range.
    """
    
    def __init__(
        self,
        weights: Optional[ScoringWeights] = None,
        threshold_auto: float = 0.90,
        threshold_suggest: float = 0.75,
    ):
        """
        Initialize the scorer.
        
        Args:
            weights: Scoring weights configuration
            threshold_auto: Minimum score for auto-correction
            threshold_suggest: Minimum score for showing suggestion
        """
        self.weights = weights.normalize() if weights else ScoringWeights()
        self.threshold_auto = threshold_auto
        self.threshold_suggest = threshold_suggest
        
        # Dependencies (injected or lazy-loaded)
        self._dictionary = None
        self._frequency = None
        self._user_dict = None
    
    def set_dependencies(
        self,
        dictionary_manager=None,
        frequency_engine=None,
        user_dictionary=None,
    ) -> None:
        """Set required dependencies for scoring."""
        self._dictionary = dictionary_manager
        self._frequency = frequency_engine
        self._user_dict = user_dictionary
    
    def score_candidate(
        self,
        candidate_word: str,
        original_word: str,
        direction: str,
        context_before: str = "",
        context_after: str = "",
    ) -> ScoreResult:
        """
        Calculate multi-factor score for a correction candidate.
        
        Args:
            candidate_word: The proposed correction
            original_word: The original typed word
            direction: 'en_to_ar' or 'ar_to_en'
            context_before: Previous word (for context scoring)
            context_after: Next word (for context scoring)
            
        Returns:
            ScoreResult with all component scores and decision
        """
        target_lang = 'arabic' if direction == 'en_to_ar' else 'english'
        
        # 1. Dictionary Score - binary validation
        s_dict = self._score_dictionary(candidate_word, target_lang)
        
        # 2. Frequency Score - log-normalized frequency
        s_freq = self._score_frequency(candidate_word, target_lang)
        
        # 3. Keyboard Map Confidence - structural validity
        s_map = self._score_keyboard_map(original_word, candidate_word, direction)
        
        # 4. Context Score - bi-gram/tri-gram pairing
        s_ctx = self._score_context(candidate_word, context_before, context_after, target_lang)
        
        # 5. User History Score - past acceptance/rejection
        s_hist = self._score_history(original_word, candidate_word)
        
        # Calculate weighted total
        total = (
            self.weights.dictionary * s_dict +
            self.weights.frequency * s_freq +
            self.weights.keyboard_map * s_map +
            self.weights.context * s_ctx +
            self.weights.history * s_hist
        )
        
        # Determine confidence tier and actions
        if total >= self.threshold_auto:
            confidence_tier = 'high'
            should_auto_correct = True
            should_suggest = False
        elif total >= self.threshold_suggest:
            confidence_tier = 'medium'
            should_auto_correct = False
            should_suggest = True
        else:
            confidence_tier = 'low'
            should_auto_correct = False
            should_suggest = False
        
        return ScoreResult(
            total_score=total,
            dictionary_score=s_dict,
            frequency_score=s_freq,
            keyboard_map_score=s_map,
            context_score=s_ctx,
            history_score=s_hist,
            should_auto_correct=should_auto_correct,
            should_suggest=should_suggest,
            confidence_tier=confidence_tier,
        )
    
    def _score_dictionary(self, word: str, language: str) -> float:
        """
        Dictionary validation score (binary).
        
        Returns 1.0 if word exists in dictionary, 0.0 otherwise.
        """
        if self._dictionary is None:
            return 0.5  # Neutral if no dictionary available
        
        return 1.0 if self._dictionary.contains(word, language) else 0.0
    
    def _score_frequency(self, word: str, language: str) -> float:
        """
        Frequency-based score (log-normalized).
        
        Returns value between 0.0 and 1.0 based on word frequency.
        """
        if self._frequency is None:
            return 0.5  # Neutral if no frequency data
        
        return self._frequency.get_normalized_score(word, language)
    
    def _score_keyboard_map(
        self,
        original: str,
        converted: str,
        direction: str,
    ) -> float:
        """
        Keyboard mapping confidence score.
        
        Heuristic based on:
        - Length preservation
        - Character type consistency
        - No unexpected symbols introduced
        """
        if len(original) != len(converted):
            # Length changed significantly - might be multi-char mapping
            if abs(len(original) - len(converted)) > 1:
                return 0.3
        
        # Check character type consistency
        orig_alpha = sum(1 for c in original if c.isalpha())
        conv_alpha = sum(1 for c in converted if c.isalpha())
        
        if orig_alpha == 0 and conv_alpha > 0:
            return 0.8  # Good - converted to alphabetic
        elif orig_alpha > 0 and conv_alpha == 0:
            return 0.4  # Less ideal - lost alphabetic chars
        
        # Default moderate score for valid mappings
        return 0.7
    
    def _score_context(
        self,
        word: str,
        before: str,
        after: str,
        language: str,
    ) -> float:
        """
        Context-based score using adjacent words.
        
        Simple implementation checks if surrounding words exist
        in the same language dictionary.
        """
        if not before and not after:
            return 0.5  # No context available
        
        if self._dictionary is None:
            return 0.5
        
        score = 0.5
        matches = 0
        total = 0
        
        if before:
            total += 1
            if self._dictionary.contains(before, language):
                matches += 1
                score = 0.7
        
        if after:
            total += 1
            if self._dictionary.contains(after, language):
                matches += 1
                score = 0.7
        
        if matches == total and total > 0:
            score = 0.8  # Both context words match
        
        return score
    
    def _score_history(self, original: str, corrected: str) -> float:
        """
        User history-based score.
        
        Returns score based on past acceptance/rejection ratio.
        """
        if self._user_dict is None:
            return 0.5  # Neutral if no history
        
        return self._user_dict.get_history_score(original, corrected)
    
    def update_weights(self, weights: Dict[str, float]) -> None:
        """Update scoring weights."""
        if 'dictionary' in weights:
            self.weights.dictionary = weights['dictionary']
        if 'frequency' in weights:
            self.weights.frequency = weights['frequency']
        if 'keyboard_map' in weights:
            self.weights.keyboard_map = weights['keyboard_map']
        if 'context' in weights:
            self.weights.context = weights['context']
        if 'history' in weights:
            self.weights.history = weights['history']
        
        self.weights = self.weights.normalize()
    
    def update_thresholds(
        self,
        auto: Optional[float] = None,
        suggest: Optional[float] = None,
    ) -> None:
        """Update decision thresholds."""
        if auto is not None:
            self.threshold_auto = max(0.0, min(1.0, auto))
        if suggest is not None:
            self.threshold_suggest = max(0.0, min(1.0, suggest))

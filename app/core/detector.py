"""
Mobadel Detector
Main detection engine that orchestrates word detection and correction flow.
"""

from typing import Optional, Dict, Any, List
from dataclasses import dataclass


@dataclass
class DetectionResult:
    """Result of processing a typed word."""
    original: str
    corrected: Optional[str]
    confidence: float
    should_correct: bool
    should_suggest: bool
    direction: str  # 'en_to_ar', 'ar_to_en', or 'none'
    reason: str


class WordDetector:
    """
    Main detection engine for keyboard layout mistakes.
    
    Orchestrates candidate generation, scoring, and decision-making
    to determine if a typed word should be corrected.
    """
    
    def __init__(self, keymap_manager=None, dictionary_manager=None, 
                 frequency_engine=None, user_dictionary=None, scorer=None):
        """
        Initialize the detector with required components.
        
        Args:
            keymap_manager: KeymapManager for layout conversion
            dictionary_manager: DictionaryManager for validation
            frequency_engine: FrequencyEngine for frequency scoring
            user_dictionary: UserDictionary for learning
            scorer: DeterministicScorer for scoring
        """
        self._keymap = keymap_manager
        self._dictionary = dictionary_manager
        self._frequency = frequency_engine
        self._user_dict = user_dictionary
        self._scorer = scorer
        
        # Import here to avoid circular imports
        if self._scorer is None:
            from .scorer import DeterministicScorer
            self._scorer = DeterministicScorer()
            self._scorer.set_dependencies(
                dictionary_manager=self._dictionary,
                frequency_engine=self._frequency,
                user_dictionary=self._user_dict,
            )
        
        if self._keymap is None:
            from ..keymap.manager import KeymapManager
            self._keymap = KeymapManager()
        
        if self._dictionary is None:
            from ..dictionary.manager import DictionaryManager
            self._dictionary = DictionaryManager()
        
        if self._frequency is None:
            from ..dictionary.frequency import FrequencyEngine
            self._frequency = FrequencyEngine()
        
        if self._user_dict is None:
            from ..dictionary.user_dictionary import UserDictionary
            self._user_dict = UserDictionary()
        
        # Import candidate generator
        from .candidate_generator import CandidateGenerator
        self._candidate_gen = CandidateGenerator(keymap_manager=self._keymap)
    
    def detect(self, word: str, context_before: str = "", 
               context_after: str = "") -> DetectionResult:
        """
        Detect if a word was typed with wrong keyboard layout.
        
        Args:
            word: The typed word
            context_before: Previous word (optional)
            context_after: Next word (optional)
            
        Returns:
            DetectionResult with correction decision
        """
        if not word or len(word) < 2:
            return DetectionResult(
                original=word,
                corrected=None,
                confidence=0.0,
                should_correct=False,
                should_suggest=False,
                direction='none',
                reason='Word too short',
            )
        
        # Check bypass patterns first
        if self._candidate_gen.should_bypass(word):
            return DetectionResult(
                original=word,
                corrected=None,
                confidence=0.0,
                should_correct=False,
                should_suggest=False,
                direction='none',
                reason='Bypass pattern matched',
            )
        
        # Generate candidates
        candidates = self._candidate_gen.generate_candidates(word)
        
        if not candidates:
            return DetectionResult(
                original=word,
                corrected=None,
                confidence=0.0,
                should_correct=False,
                should_suggest=False,
                direction='none',
                reason='No valid candidates',
            )
        
        # Score each candidate and pick the best
        best_result = None
        best_score = -1.0
        
        for candidate in candidates:
            if candidate.direction == 'en_to_ar':
                candidate_word = candidate.mapped_arabic
            else:
                candidate_word = candidate.mapped_english
            
            score_result = self._scorer.score_candidate(
                candidate_word=candidate_word,
                original_word=word,
                direction=candidate.direction,
                context_before=context_before,
                context_after=context_after,
            )
            
            if score_result.total_score > best_score:
                best_score = score_result.total_score
                best_result = {
                    'candidate': candidate,
                    'score_result': score_result,
                    'candidate_word': candidate_word,
                }
        
        if best_result is None:
            return DetectionResult(
                original=word,
                corrected=None,
                confidence=0.0,
                should_correct=False,
                should_suggest=False,
                direction='none',
                reason='No scorable candidates',
            )
        
        candidate = best_result['candidate']
        score_result = best_result['score_result']
        candidate_word = best_result['candidate_word']
        
        return DetectionResult(
            original=word,
            corrected=candidate_word if score_result.should_auto_correct or score_result.should_suggest else None,
            confidence=score_result.total_score,
            should_correct=score_result.should_auto_correct,
            should_suggest=score_result.should_suggest,
            direction=candidate.direction,
            reason=f"Score: {score_result.total_score:.2f} ({score_result.confidence_tier})",
        )
    
    def record_user_decision(self, original: str, corrected: str, accepted: bool) -> None:
        """
        Record a user's decision about a correction.
        
        Args:
            original: The original typed word
            corrected: The proposed correction
            accepted: True if user accepted, False if rejected
        """
        if self._user_dict:
            if accepted:
                self._user_dict.record_acceptance(original, corrected)
            else:
                self._user_dict.record_rejection(original, corrected)
    
    def get_detection_stats(self) -> Dict[str, Any]:
        """Get statistics about the detection system."""
        return {
            "dictionary_stats": self._dictionary.get_arabic_stats().__dict__ if self._dictionary.get_arabic_stats() else None,
            "user_dict_stats": self._user_dict.get_statistics() if self._user_dict else None,
        }

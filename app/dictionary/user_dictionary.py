"""
Mobadel User Dictionary
Handles user-specific learning and correction history.
"""

import json
from pathlib import Path
from typing import Dict, Optional, Any
from dataclasses import dataclass, field


@dataclass
class CorrectionRecord:
    """Record of a user's correction decision."""
    accepted: int = 0
    rejected: int = 0
    
    @property
    def acceptance_ratio(self) -> float:
        """Calculate the acceptance ratio."""
        total = self.accepted + self.rejected
        if total == 0:
            return 0.5  # Default neutral ratio
        return self.accepted / total
    
    def to_dict(self) -> Dict[str, int]:
        return {"accepted": self.accepted, "rejected": self.rejected}
    
    @classmethod
    def from_dict(cls, data: Dict[str, int]) -> 'CorrectionRecord':
        return cls(
            accepted=data.get("accepted", 0),
            rejected=data.get("rejected", 0)
        )


class UserDictionary:
    """
    Manages user-specific learning data.
    
    Tracks which corrections the user has accepted or rejected,
    allowing the system to learn from user preferences over time.
    All data is stored locally in a JSON file.
    """
    
    _instance: Optional['UserDictionary'] = None
    _initialized: bool = False
    
    def __new__(cls) -> 'UserDictionary':
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self) -> None:
        if UserDictionary._initialized:
            return
        
        # Structure: {original_word: {corrected_word: CorrectionRecord}}
        self._corrections: Dict[str, Dict[str, CorrectionRecord]] = {}
        self._file_path = Path.home() / '.mobadel' / 'user_dictionary.json'
        
        self._load_data()
        UserDictionary._initialized = True
    
    def _load_data(self) -> None:
        """Load user dictionary from file."""
        try:
            if self._file_path.exists():
                with open(self._file_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                
                # Convert dict to CorrectionRecord objects
                for original, corrections in data.items():
                    self._corrections[original] = {}
                    for corrected, record_data in corrections.items():
                        self._corrections[original][corrected] = CorrectionRecord.from_dict(record_data)
                        
        except (json.JSONDecodeError, IOError) as e:
            print(f"Warning: Could not load user dictionary: {e}")
            self._corrections = {}
    
    def _save_data(self) -> bool:
        """Save user dictionary to file."""
        try:
            self._file_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Convert CorrectionRecord objects to dicts
            data = {}
            for original, corrections in self._corrections.items():
                data[original] = {}
                for corrected, record in corrections.items():
                    data[original][corrected] = record.to_dict()
            
            with open(self._file_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            
            return True
        except IOError as e:
            print(f"Error saving user dictionary: {e}")
            return False
    
    def record_acceptance(self, original: str, corrected: str) -> None:
        """
        Record that a user accepted a correction.
        
        Args:
            original: The original (wrong layout) word
            corrected: The corrected word that was accepted
        """
        if original not in self._corrections:
            self._corrections[original] = {}
        
        if corrected not in self._corrections[original]:
            self._corrections[original][corrected] = CorrectionRecord()
        
        self._corrections[original][corrected].accepted += 1
        self._save_data()
    
    def record_rejection(self, original: str, corrected: str) -> None:
        """
        Record that a user rejected a correction.
        
        Args:
            original: The original (wrong layout) word
            corrected: The corrected word that was rejected
        """
        if original not in self._corrections:
            self._corrections[original] = {}
        
        if corrected not in self._corrections[original]:
            self._corrections[original][corrected] = CorrectionRecord()
        
        self._corrections[original][corrected].rejected += 1
        self._save_data()
    
    def get_acceptance_ratio(self, original: str, corrected: str) -> float:
        """
        Get the acceptance ratio for a specific correction pair.
        
        Args:
            original: The original word
            corrected: The corrected word
            
        Returns:
            Acceptance ratio (0.0 to 1.0)
        """
        if original in self._corrections and corrected in self._corrections[original]:
            return self._corrections[original][corrected].acceptance_ratio
        return 0.5  # Default neutral ratio
    
    def get_correction_record(self, original: str, corrected: str) -> Optional[CorrectionRecord]:
        """Get the full correction record for a pair."""
        if original in self._corrections:
            return self._corrections[original].get(corrected)
        return None
    
    def get_all_corrections(self, original: str) -> Dict[str, CorrectionRecord]:
        """Get all corrections for an original word."""
        return self._corrections.get(original, {}).copy()
    
    def get_history_score(self, original: str, corrected: str) -> float:
        """
        Calculate a history-based score for a correction.
        
        This score is used in the multi-factor scoring engine.
        
        Args:
            original: The original word
            corrected: The corrected word
            
        Returns:
            Score between 0.0 and 1.0 based on user history
        """
        record = self.get_correction_record(original, corrected)
        
        if record is None:
            return 0.5  # No history, neutral score
        
        # Weight recent decisions more heavily could be added here
        # For now, simple ratio
        return record.acceptance_ratio
    
    def should_auto_correct(self, original: str, corrected: str, threshold: float = 0.7) -> bool:
        """
        Check if a correction should be auto-applied based on history.
        
        Args:
            original: The original word
            corrected: The corrected word
            threshold: Minimum acceptance ratio required
            
        Returns:
            True if correction should be auto-applied
        """
        ratio = self.get_acceptance_ratio(original, corrected)
        return ratio >= threshold
    
    def clear_entry(self, original: str, corrected: Optional[str] = None) -> None:
        """
        Clear history for a correction or all corrections for a word.
        
        Args:
            original: The original word
            corrected: Specific correction to clear, or None to clear all
        """
        if original in self._corrections:
            if corrected is not None:
                del self._corrections[original][corrected]
                if not self._corrections[original]:
                    del self._corrections[original]
            else:
                del self._corrections[original]
            self._save_data()
    
    def clear_all(self) -> None:
        """Clear all user dictionary data."""
        self._corrections = {}
        self._save_data()
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get statistics about the user dictionary."""
        total_pairs = sum(len(corrections) for corrections in self._corrections.values())
        total_acceptances = sum(
            record.accepted
            for corrections in self._corrections.values()
            for record in corrections.values()
        )
        total_rejections = sum(
            record.rejected
            for corrections in self._corrections.values()
            for record in corrections.values()
        )
        
        return {
            "unique_original_words": len(self._corrections),
            "total_correction_pairs": total_pairs,
            "total_acceptances": total_acceptances,
            "total_rejections": total_rejections,
            "overall_acceptance_rate": total_acceptances / (total_acceptances + total_rejections) if (total_acceptances + total_rejections) > 0 else 0.0
        }
    
    @classmethod
    def reset(cls) -> None:
        """Reset the singleton instance."""
        cls._instance = None
        cls._initialized = False

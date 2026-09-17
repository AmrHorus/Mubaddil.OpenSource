"""
Mobadel Configuration Settings
Centralized configuration management for the application.
"""

from dataclasses import dataclass, field
from typing import Any, Dict
import json
from pathlib import Path


@dataclass
class AppSettings:
    """Application settings with defaults."""
    
    # Core functionality
    enabled: bool = True
    auto_correction: bool = False
    safe_mode: bool = True
    suggestion_popup: bool = True
    local_learning: bool = True
    
    # Scoring thresholds
    confidence_threshold_auto: float = 0.90
    confidence_threshold_suggest: float = 0.75
    min_word_length: int = 2
    max_word_length: int = 100
    
    # Scoring weights (must sum to 1.0 for normalized scoring)
    weight_dictionary: float = 0.35
    weight_frequency: float = 0.25
    weight_keyboard_map: float = 0.20
    weight_context: float = 0.10
    weight_history: float = 0.10
    
    # System settings
    run_on_startup: bool = False
    global_hotkey: str = "Ctrl+Shift+M"
    
    # UI settings
    language: str = "ar"  # 'ar' for Arabic, 'en' for English
    theme: str = "light"
    
    # Paths
    config_path: str = ""
    data_dir: str = ""
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert settings to dictionary."""
        return {
            "enabled": self.enabled,
            "auto_correction": self.auto_correction,
            "safe_mode": self.safe_mode,
            "suggestion_popup": self.suggestion_popup,
            "local_learning": self.local_learning,
            "confidence_threshold_auto": self.confidence_threshold_auto,
            "confidence_threshold_suggest": self.confidence_threshold_suggest,
            "min_word_length": self.min_word_length,
            "max_word_length": self.max_word_length,
            "weight_dictionary": self.weight_dictionary,
            "weight_frequency": self.weight_frequency,
            "weight_keyboard_map": self.weight_keyboard_map,
            "weight_context": self.weight_context,
            "weight_history": self.weight_history,
            "run_on_startup": self.run_on_startup,
            "global_hotkey": self.global_hotkey,
            "language": self.language,
            "theme": self.theme,
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'AppSettings':
        """Create settings from dictionary."""
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


class SettingsManager:
    """
    Manages application settings persistence and access.
    
    Thread-safe singleton that handles loading/saving settings
    and providing access to configuration values.
    """
    
    _instance: 'SettingsManager' = None
    _initialized: bool = False
    
    def __new__(cls) -> 'SettingsManager':
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self) -> None:
        if SettingsManager._initialized:
            return
        
        self._settings = AppSettings()
        self._config_file = Path.home() / '.mobadel' / 'settings.json'
        self._load_settings()
        SettingsManager._initialized = True
    
    def _load_settings(self) -> None:
        """Load settings from config file."""
        try:
            if self._config_file.exists():
                with open(self._config_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                self._settings = AppSettings.from_dict(data)
        except (json.JSONDecodeError, IOError) as e:
            print(f"Warning: Could not load settings: {e}")
            self._settings = AppSettings()
    
    def _save_settings(self) -> bool:
        """Save settings to config file."""
        try:
            self._config_file.parent.mkdir(parents=True, exist_ok=True)
            with open(self._config_file, 'w', encoding='utf-8') as f:
                json.dump(self._settings.to_dict(), f, indent=2, ensure_ascii=False)
            return True
        except IOError as e:
            print(f"Error saving settings: {e}")
            return False
    
    def get(self, key: str, default: Any = None) -> Any:
        """Get a setting value by key."""
        return getattr(self._settings, key, default)
    
    def set(self, key: str, value: Any) -> bool:
        """Set a setting value and save."""
        if hasattr(self._settings, key):
            setattr(self._settings, key, value)
            return self._save_settings()
        return False
    
    def update(self, updates: Dict[str, Any]) -> bool:
        """Update multiple settings at once."""
        for key, value in updates.items():
            if hasattr(self._settings, key):
                setattr(self._settings, key, value)
        return self._save_settings()
    
    def reset_to_defaults(self) -> bool:
        """Reset all settings to defaults."""
        self._settings = AppSettings()
        return self._save_settings()
    
    def get_weights(self) -> Dict[str, float]:
        """Get all scoring weights."""
        return {
            "dictionary": self._settings.weight_dictionary,
            "frequency": self._settings.weight_frequency,
            "keyboard_map": self._settings.weight_keyboard_map,
            "context": self._settings.weight_context,
            "history": self._settings.weight_history,
        }
    
    def set_weights(self, weights: Dict[str, float]) -> bool:
        """Update scoring weights."""
        valid_keys = {
            "dictionary": "weight_dictionary",
            "frequency": "weight_frequency",
            "keyboard_map": "weight_keyboard_map",
            "context": "weight_context",
            "history": "weight_history",
        }
        
        for key, value in weights.items():
            if key in valid_keys:
                setattr(self._settings, valid_keys[key], value)
        
        return self._save_settings()
    
    def get_thresholds(self) -> Dict[str, float]:
        """Get confidence thresholds."""
        return {
            "auto": self._settings.confidence_threshold_auto,
            "suggest": self._settings.confidence_threshold_suggest,
        }
    
    @property
    def settings(self) -> AppSettings:
        """Get the current settings object."""
        return self._settings
    
    @classmethod
    def reset(cls) -> None:
        """Reset the singleton instance."""
        cls._instance = None
        cls._initialized = False

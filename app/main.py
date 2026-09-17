"""
Mobadel (مبدل) - Deterministic Keyboard Layout Auto-Corrector

Main application entry point.
"""

import sys
import os

# Add app directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def main():
    """Main entry point for Mobadel application."""
    print("=" * 60)
    print("   مبدل (Mobadel) - Keyboard Layout Auto-Corrector")
    print("=" * 60)
    print()
    
    # Initialize core components
    from app.keymap.manager import KeymapManager
    from app.dictionary.manager import DictionaryManager
    from app.dictionary.frequency import FrequencyEngine
    from app.dictionary.user_dictionary import UserDictionary
    from app.core.detector import WordDetector
    
    print("Initializing components...")
    
    # Initialize managers (singletons)
    keymap = KeymapManager()
    print(f"  ✓ Keymap loaded: {len(keymap.en_to_ar_map)} mappings")
    
    dictionary = DictionaryManager()
    ar_stats = dictionary.get_arabic_stats()
    en_stats = dictionary.get_english_stats()
    print(f"  ✓ Arabic dictionary: {ar_stats.word_count if ar_stats else 0} words")
    print(f"  ✓ English dictionary: {en_stats.word_count if en_stats else 0} words")
    
    frequency = FrequencyEngine()
    print(f"  ✓ Frequency engine loaded")
    
    user_dict = UserDictionary()
    print(f"  ✓ User dictionary initialized")
    
    # Create detector
    detector = WordDetector(
        keymap_manager=keymap,
        dictionary_manager=dictionary,
        frequency_engine=frequency,
        user_dictionary=user_dict,
    )
    print(f"  ✓ Detector initialized")
    
    print()
    print("-" * 60)
    print("Testing detection examples:")
    print("-" * 60)
    
    # Test cases
    test_words = [
        ("hgsld", "أهلا"),  # Common example
        ("rvNk", "قرآن"),   # Another common example
        ("hello", None),    # Already correct English
        ("مرحبا", None),    # Already correct Arabic
        ("https://example.com", None),  # URL - should bypass
        ("test_user", None),  # Code - should bypass
    ]
    
    for original, expected in test_words:
        result = detector.detect(original)
        status = "✓" if (result.corrected == expected or 
                        (expected is None and result.corrected is None)) else "?"
        print(f"  {status} '{original}' -> {result.corrected or 'no change'} ({result.reason})")
    
    print()
    print("-" * 60)
    print("Core initialization complete!")
    print()
    print("Note: This is a console demonstration.")
    print("For the full GUI application, run with PyQt6 installed:")
    print("  python -m app.ui.main_window")
    print()
    print("For Windows system-wide keyboard hooking, the Rust core")
    print("must be built and installed via maturin.")
    print("=" * 60)
    
    return 0


if __name__ == "__main__":
    sys.exit(main())

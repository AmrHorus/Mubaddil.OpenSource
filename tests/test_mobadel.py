"""
Mobadel Unit Tests
Tests for core functionality.
"""

import unittest
import sys
import os

# Add app directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))


class TestKeymapManager(unittest.TestCase):
    """Tests for KeymapManager."""
    
    def setUp(self):
        from app.keymap.manager import KeymapManager
        KeymapManager.reset()
        self.keymap = KeymapManager()
    
    def test_en_to_ar_conversion(self):
        """Test English to Arabic conversion."""
        # q -> ض, w -> ص, e -> ث
        result = self.keymap.convert_en_to_ar("qwe")
        self.assertEqual(result, "ضصث")
    
    def test_ar_to_en_conversion(self):
        """Test Arabic to English conversion."""
        # ض -> q, ص -> w, ث -> e
        result = self.keymap.convert_ar_to_en("ضصث")
        self.assertEqual(result, "qwe")
    
    def test_bidirectional_consistency(self):
        """Test that conversions are consistent."""
        text = "hello"
        converted = self.keymap.convert_en_to_ar(text)
        back = self.keymap.convert_ar_to_en(converted)
        # Should get back to original (lowercase)
        self.assertEqual(back.lower(), text.lower())
    
    def test_is_arabic_char(self):
        """Test Arabic character detection."""
        self.assertTrue(self.keymap.is_arabic_char('ض'))
        self.assertFalse(self.keymap.is_arabic_char('a'))
    
    def test_is_english_char(self):
        """Test English character detection."""
        self.assertTrue(self.keymap.is_english_char('a'))
        self.assertFalse(self.keymap.is_english_char('ض'))


class TestCandidateGenerator(unittest.TestCase):
    """Tests for CandidateGenerator."""
    
    def setUp(self):
        from app.core.candidate_generator import CandidateGenerator
        self.generator = CandidateGenerator()
    
    def test_bypass_url(self):
        """Test URL bypass."""
        self.assertTrue(self.generator.should_bypass("https://example.com"))
        self.assertTrue(self.generator.should_bypass("www.test.com"))
    
    def test_bypass_email(self):
        """Test email bypass."""
        self.assertTrue(self.generator.should_bypass("test@example.com"))
    
    def test_bypass_numeric(self):
        """Test numeric bypass."""
        self.assertTrue(self.generator.should_bypass("12345"))
    
    def test_bypass_snake_case(self):
        """Test snake_case bypass."""
        self.assertTrue(self.generator.should_bypass("test_variable"))
    
    def test_generate_candidates(self):
        """Test candidate generation."""
        candidates = self.generator.generate_candidates("hgsld")
        # Should generate at least one candidate
        self.assertGreater(len(candidates), 0)


class TestDictionaryManager(unittest.TestCase):
    """Tests for DictionaryManager."""
    
    def setUp(self):
        from app.dictionary.manager import DictionaryManager
        DictionaryManager.reset()
        self.dict_mgr = DictionaryManager()
    
    def test_arabic_word_exists(self):
        """Test Arabic word lookup."""
        self.assertTrue(self.dict_mgr.is_arabic_word("في"))
        self.assertTrue(self.dict_mgr.is_arabic_word("مرحبا"))
    
    def test_english_word_exists(self):
        """Test English word lookup."""
        self.assertTrue(self.dict_mgr.is_english_word("hello"))
        self.assertTrue(self.dict_mgr.is_english_word("world"))
    
    def test_nonexistent_word(self):
        """Test non-existent word."""
        self.assertFalse(self.dict_mgr.is_arabic_word("xyzxyzxyz"))
        self.assertFalse(self.dict_mgr.is_english_word("xyzxyzxyz"))


class TestFrequencyEngine(unittest.TestCase):
    """Tests for FrequencyEngine."""
    
    def setUp(self):
        from app.dictionary.frequency import FrequencyEngine
        FrequencyEngine.reset()
        self.freq = FrequencyEngine()
    
    def test_common_arabic_word(self):
        """Test common Arabic word frequency."""
        score = self.freq.get_normalized_score("في", "arabic")
        self.assertGreater(score, 0.5)  # Common word should have high score
    
    def test_common_english_word(self):
        """Test common English word frequency."""
        score = self.freq.get_normalized_score("the", "english")
        self.assertGreater(score, 0.5)  # Common word should have high score
    
    def test_rare_word(self):
        """Test rare word frequency."""
        score = self.freq.get_normalized_score("nonexistent", "english")
        self.assertEqual(score, 0.0)  # Non-existent word should have 0 score


class TestScorer(unittest.TestCase):
    """Tests for DeterministicScorer."""
    
    def setUp(self):
        from app.core.scorer import DeterministicScorer, ScoringWeights
        from app.dictionary.manager import DictionaryManager
        from app.dictionary.frequency import FrequencyEngine
        from app.dictionary.user_dictionary import UserDictionary
        
        DictionaryManager.reset()
        FrequencyEngine.reset()
        UserDictionary.reset()
        
        self.scorer = DeterministicScorer()
        self.scorer.set_dependencies(
            dictionary_manager=DictionaryManager(),
            frequency_engine=FrequencyEngine(),
            user_dictionary=UserDictionary(),
        )
    
    def test_dictionary_score(self):
        """Test dictionary scoring component."""
        # Valid word should get high dictionary score
        score = self.scorer._score_dictionary("hello", "english")
        self.assertEqual(score, 1.0)
        
        # Invalid word should get low dictionary score
        score = self.scorer._score_dictionary("xyzxyz", "english")
        self.assertEqual(score, 0.0)
    
    def test_frequency_score(self):
        """Test frequency scoring component."""
        score = self.scorer._score_frequency("the", "english")
        self.assertGreater(score, 0.5)
    
    def test_full_scoring(self):
        """Test full multi-factor scoring."""
        result = self.scorer.score_candidate(
            candidate_word="hello",
            original_word="auldi",
            direction="en_to_ar",
        )
        # Score should be between 0 and 1
        self.assertGreaterEqual(result.total_score, 0.0)
        self.assertLessEqual(result.total_score, 1.0)


class TestLanguageDetector(unittest.TestCase):
    """Tests for LanguageDetector."""
    
    def test_arabic_detection(self):
        from app.core.context import LanguageDetector
        
        result = LanguageDetector.analyze("مرحبا بالعالم")
        self.assertEqual(result["dominant"], "arabic")
        self.assertGreater(result["arabic_ratio"], 0.9)
    
    def test_english_detection(self):
        from app.core.context import LanguageDetector
        
        result = LanguageDetector.analyze("hello world")
        self.assertEqual(result["dominant"], "english")
        self.assertGreater(result["english_ratio"], 0.9)
    
    def test_mixed_detection(self):
        from app.core.context import LanguageDetector
        
        result = LanguageDetector.analyze("hello مرحبا")
        self.assertTrue(result["is_mixed"])


class TestLRUCache(unittest.TestCase):
    """Tests for LRUCache."""
    
    def test_basic_operations(self):
        from app.core.cache import LRUCache
        
        cache = LRUCache(capacity=3)
        
        cache.put("a", 1)
        cache.put("b", 2)
        cache.put("c", 3)
        
        self.assertEqual(cache.get("a"), 1)
        self.assertEqual(cache.get("b"), 2)
        self.assertEqual(cache.get("c"), 3)
    
    def test_eviction(self):
        from app.core.cache import LRUCache
        
        cache = LRUCache(capacity=2)
        
        cache.put("a", 1)
        cache.put("b", 2)
        cache.put("c", 3)  # Should evict 'a'
        
        self.assertIsNone(cache.get("a"))
        self.assertEqual(cache.get("b"), 2)
        self.assertEqual(cache.get("c"), 3)


class TestUserDictionary(unittest.TestCase):
    """Tests for UserDictionary."""
    
    def setUp(self):
        from app.dictionary.user_dictionary import UserDictionary
        UserDictionary.reset()
        self.user_dict = UserDictionary()
    
    def tearDown(self):
        self.user_dict.clear_all()
    
    def test_record_acceptance(self):
        """Test recording accepted corrections."""
        self.user_dict.record_acceptance("hgsld", "hello")
        ratio = self.user_dict.get_acceptance_ratio("hgsld", "hello")
        self.assertEqual(ratio, 1.0)
    
    def test_record_rejection(self):
        """Test recording rejected corrections."""
        self.user_dict.record_rejection("hgsld", "hello")
        ratio = self.user_dict.get_acceptance_ratio("hgsld", "hello")
        self.assertEqual(ratio, 0.0)
    
    def test_mixed_history(self):
        """Test mixed acceptance/rejection history."""
        self.user_dict.record_acceptance("hgsld", "hello")
        self.user_dict.record_acceptance("hgsld", "hello")
        self.user_dict.record_rejection("hgsld", "hello")
        
        ratio = self.user_dict.get_acceptance_ratio("hgsld", "hello")
        self.assertAlmostEqual(ratio, 2/3, places=2)


if __name__ == "__main__":
    unittest.main(verbosity=2)

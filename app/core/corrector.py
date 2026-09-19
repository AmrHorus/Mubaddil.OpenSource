"""
Mobadel Corrector
Handles text correction and injection.
"""

from typing import Optional


class TextCorrector:
    """
    Handles the actual text correction process.
    
    Provides methods for simulating backspace sequences
    and injecting corrected text.
    """
    
    def __init__(self):
        """Initialize the corrector."""
        pass
    
    def calculate_backspaces(self, original: str) -> int:
        """
        Calculate number of backspaces needed to delete original text.
        
        Args:
            original: The original text to delete
            
        Returns:
            Number of backspace keystrokes needed
        """
        # For most cases, one backspace per character
        # Could be enhanced to handle multi-character graphemes
        return len(original)
    
    def prepare_correction_sequence(self, original: str, corrected: str) -> dict:
        """
        Prepare a sequence of actions for correction.
        
        Args:
            original: The original text
            corrected: The corrected text
            
        Returns:
            Dictionary with correction sequence details
        """
        return {
            "backspaces": self.calculate_backspaces(original),
            "text_to_insert": corrected,
            "original_length": len(original),
            "corrected_length": len(corrected),
        }
    
    def validate_correction(self, original: str, corrected: str) -> bool:
        """
        Validate that a correction is safe to apply.
        
        Args:
            original: The original text
            corrected: The corrected text
            
        Returns:
            True if correction is valid
        """
        if not original or not corrected:
            return False
        
        # Don't allow corrections that are drastically different length
        if abs(len(original) - len(corrected)) > 10:
            return False
        
        # Don't allow corrections with control characters
        if any(ord(c) < 32 and c not in ' \t\n' for c in corrected):
            return False
        
        return True


class KeyboardInjector:
    """
    Simulates keyboard input for text injection.
    
    This is a Python implementation. For production use on Windows,
    this would delegate to the Rust core via PyO3 bindings.
    """
    
    def __init__(self, use_rust_backend: bool = False):
        """
        Initialize the injector.
        
        Args:
            use_rust_backend: Whether to use Rust backend (Windows only)
        """
        self._use_rust = use_rust_backend
        self._rust_module = None
        
        if use_rust_backend:
            try:
                import mubaddil_core
                self._rust_module = mubaddil_core
            except ImportError:
                print("Warning: Rust backend not available, using Python fallback")
                self._use_rust = False
    
    def send_backspaces(self, count: int, delay_ms: int = 12) -> bool:
        """
        Send backspace keystrokes.
        
        Args:
            count: Number of backspaces to send
            delay_ms: Delay between keystrokes in milliseconds
            
        Returns:
            True if successful
        """
        if self._use_rust and self._rust_module:
            try:
                # Would call Rust function here
                # self._rust_module.send_backspaces(count, delay_ms)
                pass
                return True
            except Exception as e:
                print(f"Rust backend error: {e}")
        
        # Python fallback - would use pyautogui or similar
        # For now, just simulate
        import time
        for _ in range(count):
            # In real implementation, would use ctypes to call SendInput
            time.sleep(delay_ms / 1000.0)
        
        return True
    
    def send_text(self, text: str, delay_ms: int = 3) -> bool:
        """
        Send text as keystrokes.
        
        Args:
            text: Text to send
            delay_ms: Delay between characters in milliseconds
            
        Returns:
            True if successful
        """
        if self._use_rust and self._rust_module:
            try:
                # Would call Rust function here
                # self._rust_module.send_text(text, delay_ms)
                pass
                return True
            except Exception as e:
                print(f"Rust backend error: {e}")
        
        # Python fallback
        import time
        for char in text:
            # In real implementation, would use ctypes to call SendInput
            time.sleep(delay_ms / 1000.0)
        
        return True
    
    def inject_correction(self, original: str, corrected: str) -> bool:
        """
        Perform a complete correction injection.
        
        Args:
            original: Original text to replace
            corrected: Corrected text to insert
            
        Returns:
            True if successful
        """
        backspaces = len(original)
        
        # Send backspaces
        if not self.send_backspaces(backspaces):
            return False
        
        # Small delay between backspaces and new text
        import time
        time.sleep(0.01)
        
        # Send corrected text
        return self.send_text(corrected)

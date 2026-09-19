"""
Mobadel Corrector
Handles text correction and injection.
"""

import ctypes
import ctypes.wintypes
import time
from typing import Optional


# Windows API constants for SendInput
VK_BACK = 0x08
INPUT_KEYBOARD = 1
KEYEVENTF_KEYDOWN = 0x0000
KEYEVENTF_KEYUP = 0x0002


class KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", ctypes.wintypes.WORD),
        ("wScan", ctypes.wintypes.WORD),
        ("dwFlags", ctypes.wintypes.DWORD),
        ("time", ctypes.wintypes.DWORD),
        ("dwExtraInfo", ctypes.wintypes.DWORD),
    ]


class MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx", ctypes.wintypes.LONG),
        ("dy", ctypes.wintypes.LONG),
        ("mouseData", ctypes.wintypes.DWORD),
        ("dwFlags", ctypes.wintypes.DWORD),
        ("time", ctypes.wintypes.DWORD),
        ("dwExtraInfo", ctypes.wintypes.DWORD),
    ]


class HARDWAREINPUT(ctypes.Structure):
    _fields_ = [
        ("uMsg", ctypes.wintypes.DWORD),
        ("wParamL", ctypes.wintypes.WORD),
        ("wParamH", ctypes.wintypes.WORD),
    ]


class INPUT_UNION(ctypes.Union):
    _fields_ = [
        ("ki", KEYBDINPUT),
        ("mi", MOUSEINPUT),
        ("hi", HARDWAREINPUT),
    ]


class INPUT(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.wintypes.DWORD),
        ("union", INPUT_UNION),
    ]


# Load Windows libraries
user32 = ctypes.windll.user32
SendInput = user32.SendInput
SendInput.argtypes = [ctypes.wintypes.UINT, ctypes.POINTER(INPUT), ctypes.c_int]
SendInput.restype = ctypes.wintypes.UINT


def _create_key_input(vk: int, key_up: bool = False) -> INPUT:
    """Create an INPUT structure for a keyboard event."""
    inp = INPUT()
    inp.type = INPUT_KEYBOARD
    inp.union.ki.wVk = vk
    inp.union.ki.dwFlags = KEYEVENTF_KEYUP if key_up else 0
    return inp


def _send_key(vk: int, press: bool = True) -> None:
    """Send a single key press or release."""
    inp = _create_key_input(vk, not press)
    SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))


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
    
    Pure Python implementation using Windows SendInput API via ctypes.
    No Rust dependencies required.
    """
    
    def __init__(self):
        """Initialize the injector."""
        pass
    
    def send_backspaces(self, count: int, delay_ms: int = 12) -> bool:
        """
        Send backspace keystrokes.
        
        Args:
            count: Number of backspaces to send
            delay_ms: Delay between keystrokes in milliseconds
            
        Returns:
            True if successful
        """
        try:
            for _ in range(count):
                _send_key(VK_BACK, press=True)
                _send_key(VK_BACK, press=False)
                if delay_ms > 0:
                    time.sleep(delay_ms / 1000.0)
            return True
        except Exception as e:
            print(f"Error sending backspaces: {e}")
            return False
    
    def send_text(self, text: str, delay_ms: int = 3) -> bool:
        """
        Send text as keystrokes.
        
        Args:
            text: Text to send
            delay_ms: Delay between characters in milliseconds
            
        Returns:
            True if successful
        """
        try:
            for char in text:
                # Get virtual key code for character
                vk_code = ord(char.upper())
                _send_key(vk_code, press=True)
                _send_key(vk_code, press=False)
                if delay_ms > 0:
                    time.sleep(delay_ms / 1000.0)
            return True
        except Exception as e:
            print(f"Error sending text: {e}")
            return False
    
    def inject_correction(self, original: str, corrected: str, delay_backspace: int = 12, delay_text: int = 3) -> bool:
        """
        Perform a complete correction injection.
        
        Args:
            original: Original text to replace
            corrected: Corrected text to insert
            delay_backspace: Delay between backspace keystrokes in milliseconds
            delay_text: Delay between text keystrokes in milliseconds
            
        Returns:
            True if successful
        """
        backspaces = len(original)
        
        # Send backspaces
        if not self.send_backspaces(backspaces, delay_backspace):
            return False
        
        # Small delay between backspaces and new text
        time.sleep(0.01)
        
        # Send corrected text
        return self.send_text(corrected, delay_text)

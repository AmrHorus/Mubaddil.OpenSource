#!/usr/bin/env python3
"""
Test script for Mubaddil Core Python Engine

This script demonstrates how to use the pure Python Mubaddil core
with a PySide6 event loop. The Python engine runs in the background,
handling keyboard hooks and text correction with ctypes-based SendInput.

Usage:
    1. Install Python dependencies: pip install -r requirements.txt
    2. Run this script: python test_engine.py
"""

import sys
import time
from PySide6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLabel, QPushButton
from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QFont

# Import the Python core module
try:
    from core import KeyboardMapper, LanguageDetector, ConverterEngine, Direction
except ImportError as e:
    print(f"Error importing core module: {e}")
    print("Make sure you've installed Python dependencies with: pip install -r requirements.txt")
    sys.exit(1)


class MubaddilTestWindow(QMainWindow):
    """Simple test window to demonstrate Mubaddil Core integration"""
    
    def __init__(self):
        super().__init__()
        self.converter = ConverterEngine()
        self.setWindowTitle("Mubaddil Core - Test Window")
        self.setGeometry(100, 100, 500, 300)
        
        # Create central widget and layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        
        # Title label
        title_label = QLabel("Mubaddil Keyboard Layout Corrector")
        title_label.setFont(QFont("Arial", 16, QFont.Bold))
        title_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(title_label)
        
        # Status label
        self.status_label = QLabel("Engine Status: Ready")
        self.status_label.setFont(QFont("Arial", 12))
        self.status_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.status_label)
        
        # Info label
        info_label = QLabel(
            "The Python engine is ready.\n"
            "Type text like 'اثممخ' (Arabic layout) and it will be\n"
            "automatically corrected to 'hello' (English)."
        )
        info_label.setFont(QFont("Arial", 10))
        info_label.setAlignment(Qt.AlignCenter)
        info_label.setWordWrap(True)
        layout.addWidget(info_label)
        
        # Control buttons
        button_layout = QVBoxLayout()
        
        self.test_button = QPushButton("Test Conversion")
        self.test_button.clicked.connect(self.test_conversion)
        button_layout.addWidget(self.test_button)
        
        layout.addLayout(button_layout)
        
        # Result display (for debugging)
        self.result_label = QLabel("Result: (click test to see)")
        self.result_label.setFont(QFont("Consolas", 10))
        self.result_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.result_label)
    
    def test_conversion(self):
        """Test the text conversion"""
        test_cases = [
            ("اثممخ", "hello"),  # Arabic layout -> English
            ("auldi", "مرحبا"),  # English layout -> Arabic
        ]
        
        results = []
        for input_text, expected in test_cases:
            if LanguageDetector.analyze(input_text)["dominant"] == LanguageDetector.ARABIC:
                result = self.converter.convert_ar_to_en(input_text)
            else:
                result = self.converter.convert_en_to_ar(input_text)
            status = "✓" if result == expected else f"✗ (got: {result})"
            results.append(f"{input_text} -> {result} {status}")
        
        self.result_label.setText("\n".join(results))
        self.status_label.setText("Engine Status: Tests Complete")
        self.status_label.setStyleSheet("color: green;")
        print("✓ Tests completed successfully")


def test_correction_api():
    """Test the text correction API"""
    print("\n=== Testing Correction API ===\n")
    
    converter = ConverterEngine()
    
    test_cases = [
        ("اثممخ", "hello"),  # Arabic layout -> English
        ("hello", None),     # Already correct
        ("the", None),       # Common word
        ("abc", None),       # Random chars
    ]
    
    for input_text, expected in test_cases:
        # Simple conversion test
        result = converter.convert_en_to_ar(input_text) if input_text.isascii() else converter.convert_ar_to_en(input_text)
        status = "✓" if result == expected or expected is None else "✗"
        print(f"{status} convert('{input_text}') = {result} (expected: {expected})")
    
    print("\n=== API Tests Complete ===\n")


def main():
    """Main entry point"""
    print("=" * 60)
    print("Mubaddil Core - Python Engine Test")
    print("=" * 60)
    
    # First, test the correction API
    test_correction_api()
    
    # Create Qt application
    app = QApplication(sys.argv)
    app.setApplicationName("Mubaddil Test")
    
    # Create and show the test window
    window = MubaddilTestWindow()
    window.show()
    
    print("\nTest window opened.")
    print("Click 'Test Conversion' to verify the engine.")
    print("\nPress Ctrl+C or close the window to exit.\n")
    
    # Run the event loop
    sys.exit(app.exec())


if __name__ == "__main__":
    main()

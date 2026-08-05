#!/usr/bin/env python3
"""
Test script for Mubaddil Core Rust Engine

This script demonstrates how to use the compiled Rust extension module
with a PySide6 event loop. The Rust engine runs in the background,
handling keyboard hooks and text correction with sub-3ms latency.

Usage:
    1. Build the Rust extension: maturin develop --release
    2. Run this script: python test_engine.py
"""

import sys
import time
from PySide6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLabel, QPushButton
from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QFont

# Import the Rust core module
try:
    import mubaddil_core
except ImportError as e:
    print(f"Error importing mubaddil_core: {e}")
    print("Make sure you've built the Rust extension with: maturin develop --release")
    sys.exit(1)


class MubaddilTestWindow(QMainWindow):
    """Simple test window to demonstrate Mubaddil Core integration"""
    
    def __init__(self, core: mubaddil_core.MubaddilCore):
        super().__init__()
        self.core = core
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
        self.status_label = QLabel("Engine Status: Stopped")
        self.status_label.setFont(QFont("Arial", 12))
        self.status_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.status_label)
        
        # Info label
        info_label = QLabel(
            "The Rust engine runs in the background.\n"
            "Type text like 'اثممخ' (Arabic layout) and it will be\n"
            "automatically corrected to 'hello' (English)."
        )
        info_label.setFont(QFont("Arial", 10))
        info_label.setAlignment(Qt.AlignCenter)
        info_label.setWordWrap(True)
        layout.addWidget(info_label)
        
        # Control buttons
        button_layout = QVBoxLayout()
        
        self.start_button = QPushButton("Start Engine")
        self.start_button.clicked.connect(self.start_engine)
        button_layout.addWidget(self.start_button)
        
        self.stop_button = QPushButton("Stop Engine")
        self.stop_button.clicked.connect(self.stop_engine)
        self.stop_button.setEnabled(False)
        button_layout.addWidget(self.stop_button)
        
        layout.addLayout(button_layout)
        
        # Buffer display (for debugging)
        self.buffer_label = QLabel("Buffer: (empty)")
        self.buffer_label.setFont(QFont("Consolas", 10))
        self.buffer_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.buffer_label)
        
        # Timer to update status
        self.status_timer = QTimer()
        self.status_timer.timeout.connect(self.update_status)
        self.status_timer.start(500)  # Update every 500ms
    
    def start_engine(self):
        """Start the Rust keyboard hook engine"""
        try:
            self.core.start()
            self.status_label.setText("Engine Status: Running")
            self.status_label.setStyleSheet("color: green;")
            self.start_button.setEnabled(False)
            self.stop_button.setEnabled(True)
            print("✓ Engine started successfully")
        except Exception as e:
            print(f"✗ Failed to start engine: {e}")
            self.status_label.setText(f"Error: {e}")
            self.status_label.setStyleSheet("color: red;")
    
    def stop_engine(self):
        """Stop the Rust keyboard hook engine"""
        try:
            self.core.stop()
            self.status_label.setText("Engine Status: Stopped")
            self.status_label.setStyleSheet("color: black;")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
            print("✓ Engine stopped successfully")
        except Exception as e:
            print(f"✗ Failed to stop engine: {e}")
    
    def update_status(self):
        """Update the status display"""
        if self.core.is_running():
            buffer = self.core.get_buffer()
            self.buffer_label.setText(f"Buffer: '{buffer}'")
        else:
            self.buffer_label.setText("Buffer: (engine not running)")


def test_correction_api():
    """Test the text correction API without starting the hook"""
    print("\n=== Testing Correction API ===\n")
    
    core = mubaddil_core.MubaddilCore()
    
    test_cases = [
        ("اثممخ", "hello"),  # Arabic layout -> English
        ("hello", None),     # Already correct
        ("the", None),       # Common word
        ("abc", None),       # Random chars
    ]
    
    for input_text, expected in test_cases:
        result = core.correct_text(input_text)
        status = "✓" if result == expected else "✗"
        print(f"{status} correct_text('{input_text}') = {result} (expected: {expected})")
    
    print("\n=== API Tests Complete ===\n")
    return core


def main():
    """Main entry point"""
    print("=" * 60)
    print("Mubaddil Core - Rust Engine Test")
    print("=" * 60)
    
    # First, test the correction API
    core = test_correction_api()
    
    # Create Qt application
    app = QApplication(sys.argv)
    app.setApplicationName("Mubaddil Test")
    
    # Create and show the test window
    window = MubaddilTestWindow(core)
    window.show()
    
    print("\nTest window opened.")
    print("Click 'Start Engine' to activate the keyboard hook.")
    print("Note: Keyboard hooks require administrator privileges on Windows.")
    print("\nPress Ctrl+C or close the window to exit.\n")
    
    # Run the event loop
    sys.exit(app.exec())


if __name__ == "__main__":
    main()

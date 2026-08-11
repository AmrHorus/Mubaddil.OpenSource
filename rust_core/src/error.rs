//! Mubaddil Core - Error Types
//! 
//! This module defines error types for the Mubaddil core engine.

use thiserror::Error;

/// Error types for the Mubaddil Core
#[derive(Error, Debug)]
pub enum MubaddilError {
    #[error("Failed to install keyboard hook: {0}")]
    HookInstallationFailed(String),
    
    #[error("Failed to remove keyboard hook: {0}")]
    HookRemovalFailed(String),
    
    #[error("Engine is already running")]
    EngineAlreadyRunning,
    
    #[error("Engine is not running")]
    EngineNotRunning,
    
    #[error("Thread synchronization error: {0}")]
    SyncError(String),
    
    #[error("Invalid input: {0}")]
    InvalidInput(String),
    
    #[error("Windows API error: {0}")]
    WindowsApiError(String),
    
    #[error("Input injection failed: {0}")]
    InputInjectionFailed(String),
    
    #[error("Clipboard operation failed: {0}")]
    ClipboardError(String),
}

impl From<MubaddilError> for pyo3::PyErr {
    fn from(err: MubaddilError) -> pyo3::PyErr {
        match err {
            MubaddilError::EngineAlreadyRunning => pyo3::exceptions::PyRuntimeError::new_err(err.to_string()),
            MubaddilError::EngineNotRunning => pyo3::exceptions::PyRuntimeError::new_err(err.to_string()),
            MubaddilError::InvalidInput(_) => pyo3::exceptions::PyValueError::new_err(err.to_string()),
            _ => pyo3::exceptions::PyRuntimeError::new_err(err.to_string()),
        }
    }
}

/// Result type alias for Mubaddil operations
pub type MubaddilResult<T> = Result<T, MubaddilError>;

//! Mubaddil Kernel - Error Types
//!
//! This module defines error types for the Mubaddil kernel.

use thiserror::Error;

/// Error types for the Mubaddil Kernel
#[derive(Error, Debug)]
pub enum MubaddilError {
    #[error("Invalid input: {0}")]
    InvalidInput(String),

    #[error("Keyboard layout not found: {0}")]
    LayoutNotFound(String),

    #[error("Mapping failed: {0}")]
    MappingFailed(String),

    #[error("Dictionary error: {0}")]
    DictionaryError(String),

    #[error("Configuration error: {0}")]
    ConfigError(String),

    #[error("JSON parsing error: {0}")]
    JsonError(String),

    #[error("IO error: {0}")]
    IoError(String),
}

impl From<serde_json::Error> for MubaddilError {
    fn from(err: serde_json::Error) -> Self {
        MubaddilError::JsonError(err.to_string())
    }
}

impl From<std::io::Error> for MubaddilError {
    fn from(err: std::io::Error) -> Self {
        MubaddilError::IoError(err.to_string())
    }
}

/// Result type alias for Mubaddil operations
pub type MubaddilResult<T> = Result<T, MubaddilError>;

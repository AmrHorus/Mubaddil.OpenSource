"""
Mobadel Cache
LRU cache for word conversions and lookups.
"""

from typing import Optional, Dict, Any
from collections import OrderedDict


class LRUCache:
    """
    Least Recently Used (LRU) cache implementation.
    
    Provides fast O(1) lookup and insertion with automatic
    eviction of least recently used items when capacity is reached.
    """
    
    def __init__(self, capacity: int = 1000):
        """
        Initialize the LRU cache.
        
        Args:
            capacity: Maximum number of items to cache
        """
        self._capacity = capacity
        self._cache: OrderedDict = OrderedDict()
    
    def get(self, key: str) -> Optional[Any]:
        """
        Get a value from the cache.
        
        Args:
            key: Cache key
            
        Returns:
            Cached value or None if not found
        """
        if key not in self._cache:
            return None
        
        # Move to end (most recently used)
        self._cache.move_to_end(key)
        return self._cache[key]
    
    def put(self, key: str, value: Any) -> None:
        """
        Put a value in the cache.
        
        Args:
            key: Cache key
            value: Value to cache
        """
        if key in self._cache:
            # Update and move to end
            self._cache.move_to_end(key)
            self._cache[key] = value
        else:
            # Add new item
            self._cache[key] = value
            
            # Evict oldest if over capacity
            if len(self._cache) > self._capacity:
                self._cache.popitem(last=False)
    
    def contains(self, key: str) -> bool:
        """Check if key exists in cache."""
        return key in self._cache
    
    def remove(self, key: str) -> bool:
        """
        Remove a key from the cache.
        
        Args:
            key: Key to remove
            
        Returns:
            True if key was found and removed
        """
        if key in self._cache:
            del self._cache[key]
            return True
        return False
    
    def clear(self) -> None:
        """Clear all cached items."""
        self._cache.clear()
    
    def size(self) -> int:
        """Get current cache size."""
        return len(self._cache)
    
    def capacity(self) -> int:
        """Get cache capacity."""
        return self._capacity
    
    def stats(self) -> Dict[str, Any]:
        """Get cache statistics."""
        return {
            "size": len(self._cache),
            "capacity": self._capacity,
            "utilization": len(self._cache) / self._capacity if self._capacity > 0 else 0.0,
        }


class WordCache:
    """
    Specialized cache for word conversion results.
    
    Maintains separate caches for different conversion directions
    and provides convenient methods for caching word transformations.
    """
    
    def __init__(self, capacity: int = 5000):
        """
        Initialize the word cache.
        
        Args:
            capacity: Maximum items per direction cache
        """
        self._en_to_ar_cache = LRUCache(capacity)
        self._ar_to_en_cache = LRUCache(capacity)
    
    def get_conversion(self, word: str, direction: str) -> Optional[str]:
        """
        Get a cached conversion.
        
        Args:
            word: Original word
            direction: 'en_to_ar' or 'ar_to_en'
            
        Returns:
            Cached conversion or None
        """
        if direction == 'en_to_ar':
            return self._en_to_ar_cache.get(word)
        elif direction == 'ar_to_en':
            return self._ar_to_en_cache.get(word)
        return None
    
    def cache_conversion(self, word: str, direction: str, result: str) -> None:
        """
        Cache a conversion result.
        
        Args:
            word: Original word
            direction: 'en_to_ar' or 'ar_to_en'
            result: Converted word
        """
        if direction == 'en_to_ar':
            self._en_to_ar_cache.put(word, result)
        elif direction == 'ar_to_en':
            self._ar_to_en_cache.put(word, result)
    
    def is_cached(self, word: str, direction: str) -> bool:
        """Check if a conversion is cached."""
        if direction == 'en_to_ar':
            return self._en_to_ar_cache.contains(word)
        elif direction == 'ar_to_en':
            return self._ar_to_en_cache.contains(word)
        return False
    
    def clear(self) -> None:
        """Clear all caches."""
        self._en_to_ar_cache.clear()
        self._ar_to_en_cache.clear()
    
    def get_stats(self) -> Dict[str, Any]:
        """Get cache statistics."""
        return {
            "en_to_ar": self._en_to_ar_cache.stats(),
            "ar_to_en": self._ar_to_en_cache.stats(),
        }

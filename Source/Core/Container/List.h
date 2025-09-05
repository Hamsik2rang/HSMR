//
//  List.h
//  Core
//
//  High-performance dynamic array container for HSMR Engine
//  Optimized for game development with predictable performance
//
#ifndef __HS_CORE_LIST_H__
#define __HS_CORE_LIST_H__

#include "Precompile.h"
#include "Core/Memory/MemoryPool.h"
#include <algorithm>
#include <initializer_list>

HS_NS_BEGIN

template<typename T, class Allocator = DefaultAllocator>
class List
{
public:
    using value_type = T;
    using size_type = uint32;
    using difference_type = int32;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

private:
    static constexpr size_type DEFAULT_CAPACITY = 8;
    static constexpr size_type GROWTH_FACTOR_NUMERATOR = 3;
    static constexpr size_type GROWTH_FACTOR_DENOMINATOR = 2;  // 1.5x growth instead of 2x

    T* _data;
    size_type _size;
    size_type _capacity;
    Allocator _allocator;

public:
    // Constructors
    List() noexcept
        : _data(nullptr)
        , _size(0)
        , _capacity(0)
    {
    }

    explicit List(size_type capacity, const Allocator& alloc = Allocator())
        : _data(nullptr)
        , _size(0)
        , _capacity(0)
        , _allocator(alloc)
    {
        if (capacity > 0)
        {
            Reserve(capacity);
        }
    }

    List(size_type count, const T& value, const Allocator& alloc = Allocator())
        : List(count, alloc)
    {
        ResizeAndFill(count, value);
    }

    List(std::initializer_list<T> init, const Allocator& alloc = Allocator())
        : List(static_cast<size_type>(init.size()), alloc)
    {
        for (const auto& item : init)
        {
            EmplaceBack(item);
        }
    }

    // Copy constructor - deep copy
    List(const List& other)
        : _data(nullptr)
        , _size(0)
        , _capacity(0)
        , _allocator(other._allocator)
    {
        if (other._size > 0)
        {
            Reserve(other._capacity);
            CopyRange(other._data, other._data + other._size, _data);
            _size = other._size;
        }
    }

    // Move constructor - transfer ownership
    List(List&& other) noexcept
        : _data(other._data)
        , _size(other._size)
        , _capacity(other._capacity)
        , _allocator(std::move(other._allocator))
    {
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }

    // Destructor
    ~List()
    {
        Clear();
        if (_data)
        {
            _allocator.Deallocate(_data, _capacity * sizeof(T));
        }
    }

    // Assignment operators
    List& operator=(const List& other)
    {
        if (this != &other)
        {
            Clear();
            _allocator = other._allocator;
            
            if (other._size > 0)
            {
                Reserve(other._capacity);
                CopyRange(other._data, other._data + other._size, _data);
                _size = other._size;
            }
        }
        return *this;
    }

    List& operator=(List&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            if (_data)
            {
                _allocator.Deallocate(_data, _capacity * sizeof(T));
            }

            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            _allocator = std::move(other._allocator);

            other._data = nullptr;
            other._size = 0;
            other._capacity = 0;
        }
        return *this;
    }

    List& operator=(std::initializer_list<T> init)
    {
        Clear();
        Reserve(static_cast<size_type>(init.size()));
        for (const auto& item : init)
        {
            EmplaceBack(item);
        }
        return *this;
    }

    // Element access
    HS_FORCEINLINE reference operator[](size_type index) noexcept
    {
        HS_ASSERT(index < _size, "Index out of bounds");
        return _data[index];
    }

    HS_FORCEINLINE const_reference operator[](size_type index) const noexcept
    {
        HS_ASSERT(index < _size, "Index out of bounds");
        return _data[index];
    }

    HS_FORCEINLINE reference At(size_type index)
    {
        if (index >= _size) [[unlikely]]
        {
            HS_LOG(crash, "List index {} out of bounds (size: {})", index, _size);
        }
        return _data[index];
    }

    HS_FORCEINLINE const_reference At(size_type index) const
    {
        if (index >= _size) [[unlikely]]
        {
            HS_LOG(crash, "List index {} out of bounds (size: {})", index, _size);
        }
        return _data[index];
    }

    HS_FORCEINLINE reference Front() noexcept
    {
        HS_ASSERT(_size > 0, "List is empty");
        return _data[0];
    }

    HS_FORCEINLINE const_reference Front() const noexcept
    {
        HS_ASSERT(_size > 0, "List is empty");
        return _data[0];
    }

    HS_FORCEINLINE reference Back() noexcept
    {
        HS_ASSERT(_size > 0, "List is empty");
        return _data[_size - 1];
    }

    HS_FORCEINLINE const_reference Back() const noexcept
    {
        HS_ASSERT(_size > 0, "List is empty");
        return _data[_size - 1];
    }

    HS_FORCEINLINE T* Data() noexcept { return _data; }
    HS_FORCEINLINE const T* Data() const noexcept { return _data; }

    // Iterators
    HS_FORCEINLINE iterator Begin() noexcept { return _data; }
    HS_FORCEINLINE const_iterator Begin() const noexcept { return _data; }
    HS_FORCEINLINE const_iterator CBegin() const noexcept { return _data; }

    HS_FORCEINLINE iterator End() noexcept { return _data + _size; }
    HS_FORCEINLINE const_iterator End() const noexcept { return _data + _size; }
    HS_FORCEINLINE const_iterator CEnd() const noexcept { return _data + _size; }

    // STL compatibility
    HS_FORCEINLINE iterator begin() noexcept { return Begin(); }
    HS_FORCEINLINE const_iterator begin() const noexcept { return Begin(); }
    HS_FORCEINLINE iterator end() noexcept { return End(); }
    HS_FORCEINLINE const_iterator end() const noexcept { return End(); }

    // Capacity
    HS_FORCEINLINE bool Empty() const noexcept { return _size == 0; }
    HS_FORCEINLINE size_type Size() const noexcept { return _size; }
    HS_FORCEINLINE size_type Capacity() const noexcept { return _capacity; }
    HS_FORCEINLINE size_type MaxSize() const noexcept { return UINT32_MAX / sizeof(T); }

    // Reserve capacity - key optimization method
    void Reserve(size_type newCapacity)
    {
        if (newCapacity <= _capacity)
            return;

        // Align to cache line boundaries for better performance
        newCapacity = AlignToCacheLine(newCapacity);

        T* newData = static_cast<T*>(_allocator.Allocate(newCapacity * sizeof(T), alignof(T)));
        
        if (_data)
        {
            if (_size > 0)
            {
                MoveRange(_data, _data + _size, newData);
            }
            _allocator.Deallocate(_data, _capacity * sizeof(T));
        }

        _data = newData;
        _capacity = newCapacity;
    }

    void ShrinkToFit()
    {
        if (_size < _capacity)
        {
            if (_size == 0)
            {
                if (_data)
                {
                    _allocator.Deallocate(_data, _capacity * sizeof(T));
                    _data = nullptr;
                    _capacity = 0;
                }
            }
            else
            {
                T* newData = static_cast<T*>(_allocator.Allocate(_size * sizeof(T), alignof(T)));
                MoveRange(_data, _data + _size, newData);
                _allocator.Deallocate(_data, _capacity * sizeof(T));
                _data = newData;
                _capacity = _size;
            }
        }
    }

    // Modifiers
    void Clear() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_type i = 0; i < _size; ++i)
            {
                _data[i].~T();
            }
        }
        _size = 0;
    }

    void Resize(size_type newSize)
    {
        if (newSize > _size)
        {
            if (newSize > _capacity)
            {
                Reserve(newSize);
            }
            // Default construct new elements
            if constexpr (std::is_default_constructible_v<T>)
            {
                for (size_type i = _size; i < newSize; ++i)
                {
                    new(_data + i) T();
                }
            }
        }
        else if (newSize < _size)
        {
            // Destroy elements beyond new size
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_type i = newSize; i < _size; ++i)
                {
                    _data[i].~T();
                }
            }
        }
        _size = newSize;
    }

    void ResizeAndFill(size_type newSize, const T& value)
    {
        if (newSize > _capacity)
        {
            Reserve(newSize);
        }
        
        if (newSize > _size)
        {
            for (size_type i = _size; i < newSize; ++i)
            {
                new(_data + i) T(value);
            }
        }
        else if (newSize < _size)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_type i = newSize; i < _size; ++i)
                {
                    _data[i].~T();
                }
            }
        }
        _size = newSize;
    }

    // High-performance insertion methods
    void PushBack(const T& value)
    {
        EmplaceBack(value);
    }

    void PushBack(T&& value)
    {
        EmplaceBack(std::move(value));
    }

    template<typename... Args>
    reference EmplaceBack(Args&&... args)
    {
        if (_size == _capacity) [[unlikely]]
        {
            size_type newCapacity = _capacity == 0 ? DEFAULT_CAPACITY : 
                (_capacity * GROWTH_FACTOR_NUMERATOR) / GROWTH_FACTOR_DENOMINATOR;
            Reserve(newCapacity);
        }

        new(_data + _size) T(std::forward<Args>(args)...);
        return _data[_size++];
    }

    void PopBack() noexcept
    {
        HS_ASSERT(_size > 0, "Cannot pop from empty list");
        --_size;
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            _data[_size].~T();
        }
    }

    // Batch operations for better performance
    void PushBackRange(const T* first, const T* last)
    {
        size_type count = static_cast<size_type>(last - first);
        if (count == 0)
            return;

        if (_size + count > _capacity)
        {
            Reserve(_size + count);
        }

        CopyRange(first, last, _data + _size);
        _size += count;
    }

    template<typename Iterator>
    void PushBackRange(Iterator first, Iterator last)
    {
        for (auto it = first; it != last; ++it)
        {
            EmplaceBack(*it);
        }
    }

    // Fast erase (doesn't preserve order)
    void EraseSwap(size_type index) noexcept
    {
        HS_ASSERT(index < _size, "Index out of bounds");
        
        if (index != _size - 1)
        {
            _data[index] = std::move(_data[_size - 1]);
        }
        PopBack();
    }

    // Ordered erase (preserves order, slower)
    iterator Erase(size_type index)
    {
        HS_ASSERT(index < _size, "Index out of bounds");
        
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            _data[index].~T();
        }
        
        // Move elements down
        if (index < _size - 1)
        {
            MoveRange(_data + index + 1, _data + _size, _data + index);
        }
        
        --_size;
        return _data + index;
    }

    iterator Erase(iterator pos)
    {
        return Erase(static_cast<size_type>(pos - _data));
    }

    iterator Erase(iterator first, iterator last)
    {
        if (first == last)
            return last;

        size_type firstIndex = static_cast<size_type>(first - _data);
        size_type count = static_cast<size_type>(last - first);
        
        // Destroy elements in range
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_type i = 0; i < count; ++i)
            {
                _data[firstIndex + i].~T();
            }
        }
        
        // Move remaining elements
        if (last != End())
        {
            MoveRange(last, End(), first);
        }
        
        _size -= count;
        return _data + firstIndex;
    }

    // Find operations
    iterator Find(const T& value) noexcept
    {
        for (size_type i = 0; i < _size; ++i)
        {
            if (_data[i] == value)
                return _data + i;
        }
        return End();
    }

    const_iterator Find(const T& value) const noexcept
    {
        for (size_type i = 0; i < _size; ++i)
        {
            if (_data[i] == value)
                return _data + i;
        }
        return End();
    }

    bool Contains(const T& value) const noexcept
    {
        return Find(value) != End();
    }

    // Comparison operators
    bool operator==(const List& other) const noexcept
    {
        if (_size != other._size)
            return false;
        
        for (size_type i = 0; i < _size; ++i)
        {
            if (!(_data[i] == other._data[i]))
                return false;
        }
        return true;
    }

    bool operator!=(const List& other) const noexcept
    {
        return !(*this == other);
    }

private:
    // Memory alignment for cache efficiency
    static size_type AlignToCacheLine(size_type size) noexcept
    {
        constexpr size_type CACHE_LINE_SIZE = 64;
        constexpr size_type ALIGNMENT_ELEMENTS = CACHE_LINE_SIZE / sizeof(T);
        
        if constexpr (ALIGNMENT_ELEMENTS > 1)
        {
            return ((size + ALIGNMENT_ELEMENTS - 1) / ALIGNMENT_ELEMENTS) * ALIGNMENT_ELEMENTS;
        }
        else
        {
            return size;
        }
    }

    // Optimized copy/move operations
    void CopyRange(const T* first, const T* last, T* dest)
    {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            size_type count = static_cast<size_type>(last - first);
            std::memcpy(dest, first, count * sizeof(T));
        }
        else
        {
            for (const T* src = first; src != last; ++src, ++dest)
            {
                new(dest) T(*src);
            }
        }
    }

    void MoveRange(T* first, T* last, T* dest) noexcept
    {
        if constexpr (std::is_trivially_move_constructible_v<T>)
        {
            size_type count = static_cast<size_type>(last - first);
            std::memmove(dest, first, count * sizeof(T));
        }
        else
        {
            if (dest < first)
            {
                // Move forward
                for (T* src = first; src != last; ++src, ++dest)
                {
                    new(dest) T(std::move(*src));
                    src->~T();
                }
            }
            else
            {
                // Move backward
                T* srcEnd = last - 1;
                T* destEnd = dest + (last - first) - 1;
                for (T* src = srcEnd; src >= first; --src, --destEnd)
                {
                    new(destEnd) T(std::move(*src));
                    src->~T();
                }
            }
        }
    }
};

HS_NS_END

#endif // __HS_CORE_LIST_H__
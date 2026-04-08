//
//  Flag.h
//  Core
//
//  Created by Yongsik Im on 3/22/2025
//

#ifndef __HS_FLAG_H__
#define __HS_FLAG_H__

#include "Precompile.h"

#include <limits>
#include <type_traits>
#include <vector>

HS_NS_BEGIN

/**
 * @brief Enum bit flag helper.
 * @details EnumFlag values must be independent bit flags. This class does not validate that at runtime.
 */
template <typename EnumFlag, typename Storage = uint32>
class HS_CORE_API Flag
{
    static_assert(std::is_enum_v<EnumFlag>, "Flag requires an enum type.");
    static_assert(std::is_integral_v<Storage>, "Flag storage must be an integral type.");
    static_assert(std::is_unsigned_v<Storage>, "Flag storage must be unsigned.");

public:
    using EnumType = EnumFlag;
    using StorageType = Storage;

    Flag() = default;
    explicit Flag(Storage bitFlag)
        : _bitFlag(bitFlag)
    {}

    static std::vector<uint8> MakeList(Storage bitFlag, uint8 maxIndex = uint8(-1))
    {
        std::vector<uint8> ret;
        const uint8 storageBitCount = static_cast<uint8>(sizeof(Storage) * 8);
        const uint8 bitLimit = maxIndex < storageBitCount ? maxIndex : storageBitCount;

        uint8 bit = 0;
        while (bitFlag != 0 && bit < bitLimit)
        {
            if ((bitFlag & static_cast<Storage>(1)) == static_cast<Storage>(1))
            {
                ret.push_back(bit);
            }

            bitFlag >>= 1;
            ++bit;
        }

        return ret;
    }

    template<typename... Args>
    void Add(EnumFlag flag, Args... args)
    {
        static_assert((std::is_same_v<EnumFlag, Args> && ...), "Flag::Add arguments must use the same enum type.");
        add(flag);
        (add(args), ...);
    }

    template<typename... Args>
    void Set(EnumFlag flag, Args... args)
    {
        static_assert((std::is_same_v<EnumFlag, Args> && ...), "Flag::Set arguments must use the same enum type.");
        set(flag);
        (add(args), ...);
    }

    template<typename... Args>
    void Remove(EnumFlag flag, Args... args)
    {
        static_assert((std::is_same_v<EnumFlag, Args> && ...), "Flag::Remove arguments must use the same enum type.");
        remove(flag);
        (remove(args), ...);
    }

    bool Contains(EnumFlag flag) const
    {
        return contains(flag);
    }

    template<typename... Args>
    bool ContainsAny(EnumFlag flag, Args... args) const
    {
        static_assert((std::is_same_v<EnumFlag, Args> && ...), "Flag::ContainsAny arguments must use the same enum type.");
        if constexpr (sizeof...(Args) == 0)
        {
            return contains(flag);
        }
        else
        {
            return contains(flag) || (... || contains(args));
        }
    }

    template<typename... Args>
    bool ContainsAll(EnumFlag flag, Args... args) const
    {
        static_assert((std::is_same_v<EnumFlag, Args> && ...), "Flag::ContainsAll arguments must use the same enum type.");
        if constexpr (sizeof...(Args) == 0)
        {
            return contains(flag);
        }
        else
        {
            return contains(flag) && (... && contains(args));
        }
    }

    virtual bool IsEmpty() const
    {
        return _bitFlag == 0;
    }

    virtual void Clear(bool active = false)
    {
        _bitFlag = active ? AllBits() : 0;
    }

    Storage GetBit() const
    {
        return _bitFlag;
    }

    std::vector<uint8> GetList(uint8 maxIndex = uint8(-1)) const
    {
        return MakeList(_bitFlag, maxIndex);
    }

    Flag& operator=(const Flag& flag)
    {
        _bitFlag = flag._bitFlag;
        return *this;
    }

    Flag& operator=(Flag&& flag) noexcept
    {
        _bitFlag = flag._bitFlag;
        return *this;
    }

    bool operator==(const Flag& flag) const
    {
        return _bitFlag == flag._bitFlag;
    }

    bool operator!=(const Flag& flag) const
    {
        return _bitFlag != flag._bitFlag;
    }

protected:
    static constexpr Storage AllBits()
    {
        return std::numeric_limits<Storage>::max();
    }

    static constexpr Storage ToBit(EnumFlag flag)
    {
        return static_cast<Storage>(flag);
    }

    virtual void add(EnumFlag flag)
    {
        _bitFlag |= ToBit(flag);
    }

    virtual void set(EnumFlag flag)
    {
        _bitFlag = ToBit(flag);
    }

    virtual void remove(EnumFlag flag)
    {
        _bitFlag &= static_cast<Storage>(~ToBit(flag));
    }

    virtual bool contains(EnumFlag flag) const
    {
        Storage bit = ToBit(flag);
        return (_bitFlag & bit) == bit;
    }

    Storage _bitFlag = 0;
};

template <typename EnumFlag, typename Storage = uint32>
class HS_CORE_API DirtyFlag : public Flag<EnumFlag, Storage>
{
    using Base = Flag<EnumFlag, Storage>;

public:
    DirtyFlag() = default;
    explicit DirtyFlag(Storage bitFlag)
        : Base(bitFlag)
        , _isChanged(bitFlag != 0)
    {}

    bool IsChanged() const
    {
        return _isChanged;
    }

    void Clear(bool active = false) override
    {
        Base::_bitFlag = active ? Base::AllBits() : 0;
        _isChanged = active;
    }

protected:
    void add(EnumFlag flag) override
    {
        Base::add(flag);
        _isChanged = true;
    }

    void set(EnumFlag flag) override
    {
        Base::set(flag);
        _isChanged = true;
    }

    void remove(EnumFlag flag) override
    {
        Base::remove(flag);
        _isChanged = true;
    }

    bool _isChanged = false;
};

HS_NS_END

#endif /* Flag_h */

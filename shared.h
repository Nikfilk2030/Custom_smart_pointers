#pragma once

#include <cstddef>  // std::nullptr_t
#include <iostream>
#include <type_traits>

#include "sw_fwd.h"  // Forward declaration
#include "weak.h"

class IEnableSharedFromThis {};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public IEnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this);
    }

    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(weak_this);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this;
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(weak_this);
    }

public:
    WeakPtr<T> weak_this;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

public:
    T* ptr_;
    ControlBlockBase* block_;

protected:
    void IncrementBlockStrongCounter() {
        if (block_ == nullptr) {
            return;
        }
        block_->IncrementStrongCounter();
    }

    void DecrementBlockStrongCounter() {
        if (block_ == nullptr) {
            return;
        }
        if (block_->GetStrongCounter() == 1 && block_->GetWeakCounter() == 0) {
            block_->DeletePointer();
            delete block_;
            return;
        }
        block_->DecrementStrongCounter();
        if (block_->GetStrongCounter() <= 0) {
            block_->DeletePointer();
        }
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : ptr_(nullptr), block_(nullptr){};

    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr){};

    // constructor from ptr
    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(new ControlBlockPointer<T>(ptr)) {
        if constexpr (std::is_convertible_v<T*, IEnableSharedFromThis*>) {
            ptr->weak_this = *this;
        }
    }

    template <typename U>
    SharedPtr(U* ptr) : ptr_(ptr), block_(new ControlBlockPointer<U>(ptr)) {
        if constexpr (std::is_convertible_v<U*, IEnableSharedFromThis*>) {
            ptr->weak_this = *this;
        }
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        IncrementBlockStrongCounter();
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        IncrementBlockStrongCounter();
    }

    // ctor for control block holder
    template <typename U>
    SharedPtr(ControlBlockHolder<U>* block) : ptr_(block->GetPointer()), block_(block) {
        if constexpr (std::is_convertible_v<T*, IEnableSharedFromThis*>) {
            ptr_->weak_this = *this;
        }
    }

    SharedPtr(SharedPtr&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename U>
    SharedPtr(const SharedPtr<U>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        IncrementBlockStrongCounter();
    }

    //     Promote `WeakPtr`
    //     #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.block_->GetStrongCounter() == 0) {
            throw BadWeakPtr();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        block_->IncrementStrongCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        DecrementBlockStrongCounter();
        block_ = other.block_;
        IncrementBlockStrongCounter();
        ptr_ = other.ptr_;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        DecrementBlockStrongCounter();
        block_ = nullptr;
        ptr_ = nullptr;
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        DecrementBlockStrongCounter();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecrementBlockStrongCounter();
        ptr_ = nullptr;
        block_ = nullptr;
    }

    void Reset(T* ptr) {
        DecrementBlockStrongCounter();
        ptr_ = ptr;
        block_ = new ControlBlockPointer<T>(ptr);
    }

    template <typename U>
    void Reset(U* ptr) {
        DecrementBlockStrongCounter();
        ptr_ = ptr;
        block_ = new ControlBlockPointer<U>(ptr);
    }

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->GetStrongCounter();
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new ControlBlockHolder<T>(std::forward<Args>(args)...));
}

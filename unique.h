#pragma once

#include <cstddef>  // std::nullptr_t

#include "compressed_pair.h"

template <typename T>
class UniquePtrDeleter {
public:
    UniquePtrDeleter() = default;

    template <typename ChildType>                       // Will we call derived child using <>?
    UniquePtrDeleter(UniquePtrDeleter<ChildType>&&){};  // to call deleter from any type

public:
    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
class UniquePtrDeleter<T[]> {
public:
    UniquePtrDeleter() = default;

    template <typename ChildType>                       // Will we call derived child using <>?
    UniquePtrDeleter(UniquePtrDeleter<ChildType>&&){};  // to call deleter from any type

public:
    void operator()(T* ptr) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = UniquePtrDeleter<T>>
class UniquePtr {
protected:
    CompressedPair<T*, Deleter> data_;

public:
    template <typename OtherType, typename OtherDeleter>
    friend class UniquePtr;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : data_(ptr, Deleter()){};

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr(T* ptr, Deleter deleter) noexcept : data_(ptr, std::move(deleter)){};

    template <typename OtherType, typename OtherDeleter>
    UniquePtr(UniquePtr<OtherType, OtherDeleter>&& other) noexcept
        : data_(other.Release(), std::forward<OtherDeleter>(other.GetDeleter())){};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;

    template <typename OtherType, typename OtherDeleter>
    UniquePtr& operator=(UniquePtr<OtherType, OtherDeleter>&& other) noexcept {
        if (Get() == other.Get()) {
            return *this;
        }

        GetDeleter()(Get());  // deletes current element
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        GetDeleter()(Get());  // calls deleter from current element
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        GetDeleter()(Get());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_data = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return tmp_data;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = Get();  // Gets current element
        data_.GetFirst() = ptr;
        GetDeleter()(tmp_ptr);  // Calls deleter from the current element
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() noexcept {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const noexcept {
        return data_.GetSecond();
    }
    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *(Get());
    }
    T* operator->() const noexcept {
        return Get();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
protected:
    CompressedPair<T*, Deleter> data_;

public:
    template <typename OtherType, typename OtherDeleter>
    friend class UniquePtr;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : data_(ptr, Deleter()){};

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr(T* ptr, Deleter deleter) noexcept : data_(ptr, std::move(deleter)){};

    template <typename OtherType, typename OtherDeleter>
    UniquePtr(UniquePtr<OtherType, OtherDeleter>&& other) noexcept
        : data_(other.Release(), std::forward<OtherDeleter>(other.GetDeleter())){};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;

    template <typename OtherType, typename OtherDeleter>
    UniquePtr& operator=(UniquePtr<OtherType, OtherDeleter>&& other) noexcept {
        if (Get() == other.Get()) {
            return *this;
        }

        GetDeleter()(Get());  // deletes current element
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        data_.GetSecond() = std::move(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        GetDeleter()(Get());  // calls deleter from current element
        data_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        GetDeleter()(Get());  // переписать через get и get deleter
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_data = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return tmp_data;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = Get();  // Gets current element
        data_.GetFirst() = ptr;
        GetDeleter()(tmp_ptr);  // Calls deleter from the current element
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        std::swap(data_.GetSecond(), other.data_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() noexcept {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const noexcept {
        return data_.GetSecond();
    }
    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *(Get());
    }
    T* operator->() const noexcept {
        return Get();
    }
    T& operator[](size_t i) {
        return Get()[i];
    }
    const T& operator[](size_t i) const {
        return Get()[i];
    }
};

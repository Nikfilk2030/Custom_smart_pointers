#pragma once

// Paste here your implementation of compressed_pair from seminar 2 to use in UniquePtr

#include <type_traits>
#include <iostream>

template <typename T, size_t Index, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedPairElement {
public:
    CompressedPairElement() = default;
    template <typename U>  // надо ли?
    CompressedPairElement(U&& elem) : elem_(std::forward<U>(elem)){};

public:
    const T& GetElem() const {
        return elem_;
    }
    T& GetElem() {
        return elem_;
    }

protected:
    T elem_{};
};

template <typename T, size_t Index>  // implementation in case element is empty
class CompressedPairElement<T, Index, true> : public T {
public:
    CompressedPairElement() = default;
    template <typename U>  // надо ли?
    CompressedPairElement(U&& elem) : T(std::forward<U>(elem)){};

public:
    const T& GetElem() const {
        return *this;
    }
    T& GetElem() {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : public CompressedPairElement<F, 0>, public CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair() = default;
    template <typename U1, typename U2>
    CompressedPair(U1&& t1, U2&& t2) : First(std::forward<U1>(t1)), Second(std::forward<U2>(t2)){};

    F& GetFirst() {
        return First::GetElem();
    }
    const F& GetFirst() const {
        return First::GetElem();
    }

    S& GetSecond() {
        return Second::GetElem();
    };
    const S& GetSecond() const {
        return Second::GetElem();
    };
};
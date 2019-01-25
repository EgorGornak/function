#ifndef FUNCTIONIMPL_MY_FUNCTION_H
#define FUNCTIONIMPL_MY_FUNCTION_H

#include <memory>
#include <cstring>

template<typename>
class my_function;


template<typename ReturnType, typename ... ArgTypes>
class my_function<ReturnType(ArgTypes ...)> {
public:
    my_function() noexcept : is_small(false), holder(nullptr) {}

    my_function(nullptr_t) noexcept : is_small(false), holder(nullptr) {}

    my_function(my_function const &other) {
        is_small = other.is_small;
        if (is_small) {
            auto p = reinterpret_cast<function_holder_base *>(const_cast<char *>(other.buffer));
            p->create_small_copy(buffer);
        } else {
            holder = other.holder->copy();
        }
    }

    my_function(my_function &&other) : is_small(false), holder(nullptr) {
        std::swap(buffer, other.buffer);
        std::swap(is_small, other.is_small);
        other.~my_function();
    }

    my_function &operator=(my_function const &other) {
        my_function tmp(other);
        swap(tmp);
        return *this;
    }

    my_function &operator=(my_function &&other) noexcept {
        swap(other);
        return *this;
    }

    explicit operator bool() const noexcept {
        return (is_small || holder != nullptr);
    }

    template<typename FunctionT>
    my_function(FunctionT function) {
        if (sizeof(FunctionT) <= BUFFER_SIZE) {
            is_small = true;
            new(buffer) template_function_holder<FunctionT>(std::move(function));
        } else {
            is_small = false;
            holder = std::make_unique<template_function_holder<FunctionT>>(std::move(function));
        }
    }

    ReturnType operator()(ArgTypes ... args) const {
        if (is_small) {
            auto p = reinterpret_cast<function_holder_base *>(const_cast<char *>(buffer));
            return p->call(std::forward<ArgTypes>(args)...);
        } else {
            return holder->call(std::forward<ArgTypes>(args)...);
        }
    }

    void swap(my_function &other) noexcept {
        std::swap(is_small, other.is_small);
        std::swap(buffer, other.buffer);
    }

    ~my_function() {
        if (is_small) {
            auto p = reinterpret_cast<function_holder_base *>(const_cast<char *>(buffer));
            p->~function_holder_base();
        } else {
            holder.reset();
        }
    }

private:
    class function_holder_base {
    public:
        function_holder_base() {}

        virtual ~function_holder_base() {}

        virtual ReturnType call(ArgTypes ... args) = 0;

        virtual std::unique_ptr<function_holder_base> copy() = 0;

        virtual void create_small_copy(void *buff) = 0;
    };


    template<typename FunctionT>
    class template_function_holder : public function_holder_base {
    public:
        template_function_holder(FunctionT const &function) : currentFunction(function) {}

        virtual ReturnType call(ArgTypes ... args) {
            return currentFunction(std::forward<ArgTypes>(args) ...);
        }

        virtual std::unique_ptr<function_holder_base> copy() {
            return std::make_unique<template_function_holder<FunctionT>>(currentFunction);
        }

        virtual void create_small_copy(void *buffer) {
            new(buffer)template_function_holder<FunctionT>(currentFunction);
        }

    private:
        FunctionT currentFunction;
    };


    static unsigned const int BUFFER_SIZE = 40;
    bool is_small;
    union {
        std::unique_ptr<function_holder_base> holder;
        char buffer[BUFFER_SIZE];
    };
};

#endif //FUNCTIONIMPL_MY_FUNCTION_H
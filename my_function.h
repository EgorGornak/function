#ifndef FUNCTIONIMPL_MY_FUNCTION_H
#define FUNCTIONIMPL_MY_FUNCTION_H

#include <memory>
#include <cstring>

template<typename>
class my_function;


template<typename ReturnType, typename ... ArgTypes>
class my_function<ReturnType(ArgTypes ...)> {
public:
    my_function() noexcept : holder(), is_small(false) {}

    my_function(nullptr_t) noexcept : holder(), is_small(false) {}

    my_function(my_function const &other) noexcept {
        is_small = other.is_small;
        if (other.is_small) {
            memcpy(buffer, other.buffer, BUFFER_SIZE);
        } else {
            holder = other.holder->copy();
        }
    }

    my_function(my_function &&other) {
        is_small = other.is_small;
        if (other.is_small) {
            memcpy(buffer, other.buffer, BUFFER_SIZE);
        } else {
            holder = std::move(other.holder);
        }
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
        return is_small && holder != nullptr;
    }

    template<typename FunctionT>
    my_function(FunctionT functionT) : holder(new template_function_holder<FunctionT>(functionT)){
        if (sizeof(FunctionT) <= BUFFER_SIZE) {
            is_small = true;
            new(buffer) template_function_holder<FunctionT>(functionT);
        } else {
            is_small = false;
            ///holder = std::make_unique<FunctionT>(FunctionT)
            ///holder = std::make_unique<template_function_holder<FunctionT>>(functionT);
        }
    }

    ReturnType operator()(ArgTypes ... args) const {
        if (is_small) {
            auto p = (function_holder_base*)(buffer);
            return p->call(args...);
        }
        return holder->call(args ...);
    }

    void swap(my_function &other) noexcept {
        std::swap(is_small, other.is_small);
        std::swap(holder, other.holder);
        char tmpBuffer[BUFFER_SIZE];
        memcpy(tmpBuffer, buffer, BUFFER_SIZE);
        memcpy(buffer, other.buffer, BUFFER_SIZE);
        memcpy(other.buffer, tmpBuffer, BUFFER_SIZE);
    }

private:
    class function_holder_base {
    public:
        function_holder_base() {}

        virtual ~function_holder_base() {}

        virtual ReturnType call(ArgTypes ... args)  = 0;

        virtual std::unique_ptr<function_holder_base> copy() = 0;


    private:
        function_holder_base(function_holder_base const &other);

        void operator=(function_holder_base const &other);
    };


    template<typename FunctionT>
    class template_function_holder : public function_holder_base {
    public:
        template_function_holder(FunctionT functionT) : function_holder_base(), currentFunction(functionT) {}

        virtual ReturnType call(ArgTypes ... args)  {
            return currentFunction(args ...);
        }

        virtual std::unique_ptr<function_holder_base> copy() {
            return std::unique_ptr<function_holder_base>(new template_function_holder<FunctionT>(currentFunction));
        }

    private:
        FunctionT currentFunction;
    };



    bool is_small;
    static unsigned const int BUFFER_SIZE = 40;
    std::unique_ptr<function_holder_base> holder;
    char buffer[BUFFER_SIZE];
};

#endif //FUNCTIONIMPL_MY_FUNCTION_H

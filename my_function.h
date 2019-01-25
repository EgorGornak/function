#ifndef FUNCTIONIMPL_MY_FUNCTION_H
#define FUNCTIONIMPL_MY_FUNCTION_H

#include <memory>
#include <cstring>

template<typename>
class my_function;


template<typename ReturnType, typename ... ArgTypes>
class my_function<ReturnType(ArgTypes ...)> {
public:
    my_function() noexcept : state(EMPTY) {}

    my_function(nullptr_t) noexcept : state(EMPTY) {}

    my_function(my_function const &other) noexcept {
        state = other.state;
        if (state == SMALL) {
            function_holder_base *p = reinterpret_cast<function_holder_base *>(const_cast<char *>(other.buffer));
            p->create_small_copy(buffer);
        } else if (state == BIG) {
            holder = other.holder->copy();
        }
    }

    my_function(my_function &&other) {
        std::swap(buffer, other.buffer);
        std::swap(state, other.state);
        other.~my_function();
    }

    my_function &operator=(my_function const &other) noexcept {
        my_function tmp(other);
        swap(tmp);
        return *this;
    }

    my_function &operator=(my_function &&other) noexcept {
        swap(other);
        return *this;
    }

    explicit operator bool() const noexcept {
        return state != EMPTY;
    }

    template<typename FunctionT>
    my_function(FunctionT function) {
        if (sizeof(FunctionT) <= BUFFER_SIZE) {
            state = SMALL;
            new(buffer) template_function_holder<FunctionT>(std::move(function));
        } else {
            state = BIG;
            holder = std::make_unique<template_function_holder<FunctionT>>(function);
        }
    }

    ReturnType operator()(ArgTypes ... args) const {
        if (state == EMPTY) {
            std::__throw_bad_function_call();
        } else if (state == SMALL) {
            auto p = reinterpret_cast<function_holder_base *>(const_cast<char *>(buffer));
            return p->call(std::forward<ArgTypes>(args)...);
        } else {
            return holder->call(std::forward<ArgTypes>(args)...);
        }
    }

    void swap(my_function &other) noexcept {
        std::swap(state, other.state);
        std::swap(buffer, other.buffer);
    }

    ~my_function() {
        if (state == SMALL) {
            ((function_holder_base *) buffer)->~function_holder_base();
        } else if (state == BIG) {
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

        ~template_function_holder() override = default;

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
    enum STATE {
        EMPTY, SMALL, BIG
    };
    STATE state;
    union {
        std::unique_ptr<function_holder_base> holder;
        char buffer[BUFFER_SIZE];
    };
};

#endif //FUNCTIONIMPL_MY_FUNCTION_H
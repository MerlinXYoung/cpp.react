
//          Copyright Sebastian Jeckel 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef REACT_EVENT_H_INCLUDED
#define REACT_EVENT_H_INCLUDED

#pragma once

#include "react/detail/Defs.h"

#include <memory>
#include <type_traits>
#include <utility>

#include "react/Observer.h"
#include "react/TypeTraits.h"
#include "react/common/Util.h"
#include "react/detail/EventBase.h"

/*****************************************/ REACT_BEGIN /*****************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class EventStream;

template <typename T>
class EventSource;

enum class Token;

template <typename T>
class Signal;

using REACT_IMPL::WeightHint;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Merge
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename TArg1,
    typename ... TArgs,
    typename E = TArg1
>
auto Merge(const Events<TArg1>& arg1, const Events<TArgs>& ... args) -> Events<E>
{
    using REACT_IMPL::EventOpNode;

    static_assert(sizeof...(TArgs) > 0, "Merge: 2+ arguments are required.");

    return Events<E>(
        std::make_shared<EventMergeNode<E>>(
            GetNodePtr(arg1), GetNodePtr(args) ...));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Filter
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename E,
    typename FIn,
    typename F = typename std::decay<FIn>::type
>
auto Filter(const Events<E>& src, FIn&& filter) -> Events<E>
{
    using REACT_IMPL::EventOpNode;

    return Events<E>(
        std::make_shared<EventOpNode<E>>(
            std::forward<FIn>(filter), GetNodePtr(src)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Filter - Synced
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename E,
    typename FIn,
    typename ... TDepValues
>
auto Filter(const Events<E>& source, const SignalPack<TDepValues...>& depPack, FIn&& func) -> Events<E>
{
    using REACT_IMPL::SyncedEventFilterNode;

    using F = typename std::decay<FIn>::type;

    struct NodeBuilder_
    {
        NodeBuilder_(const Events<E>& source, FIn&& func) :
            MySource( source ),
            MyFunc( std::forward<FIn>(func) )
        {}

        auto operator()(const Signal<D,TDepValues>& ... deps)
            -> Events<E>
        {
            return Events<E>(
                std::make_shared<SyncedEventFilterNode<D,E,F,TDepValues ...>>(
                     GetNodePtr(MySource), std::forward<FIn>(MyFunc), GetNodePtr(deps) ...));
        }

        const Events<E>&      MySource;
        FIn                     MyFunc;
    };

    return REACT_IMPL::apply(
        NodeBuilder_( source, std::forward<FIn>(func) ),
        depPack.Data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Transform
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename TIn,
    typename FIn,
    typename F = typename std::decay<FIn>::type,
    typename TOut = typename std::result_of<F(TIn)>::type
>
auto Transform(const Events<TIn>& src, FIn&& func) -> Events<TOut>
{
    using REACT_IMPL::EventOpNode;

    return Events<TOut>(
        std::make_shared<EventOpNode<TOut>>(
            std::forward<FIn>(func), GetNodePtr(src)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Transform - Synced
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename TIn,
    typename FIn,
    typename ... TDepValues,
    typename TOut = typename std::result_of<FIn(TIn,TDepValues...)>::type
>
auto Transform(const Events<TIn>& source, FIn&& func, const Signal<TDepValues>& ... deps) -> Events<TOut>
{
    using REACT_IMPL::SyncedEventTransformNode;

    using F = typename std::decay<FIn>::type;

    struct NodeBuilder_
    {
        NodeBuilder_(const Events<TIn>& source, FIn&& func) :
            MySource( source ),
            MyFunc( std::forward<FIn>(func) )
        {}

        auto operator()(const Signal<D,TDepValues>& ... deps)
            -> Events<TOut>
        {
            return Events<TOut>(
                std::make_shared<SyncedEventTransformNode<D,TIn,TOut,F,TDepValues ...>>(
                     GetNodePtr(MySource), std::forward<FIn>(MyFunc), GetNodePtr(deps) ...));
        }

        const Events<TIn>&    MySource;
        FIn                     MyFunc;
    };

    return REACT_IMPL::apply(
        NodeBuilder_( source, std::forward<FIn>(func) ),
        depPack.Data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Process
///////////////////////////////////////////////////////////////////////////////////////////////////
using REACT_IMPL::EventRange;
using REACT_IMPL::EventEmitter;

template
<
    typename TOut,
    typename TIn,
    typename FIn,
    typename F = typename std::decay<FIn>::type
>
auto Process(const Events<TIn>& src, FIn&& func) -> Events<TOut>
{
    using REACT_IMPL::EventProcessingNode;

    return Events<TOut>(
        std::make_shared<EventProcessingNode<D,TIn,TOut,F>>(
            GetNodePtr(src), std::forward<FIn>(func)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Process - Synced
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename TOut,
    typename TIn,
    typename FIn,
    typename ... TDepValues
>
auto Process(const Events<TIn>& source, const SignalPack<D,TDepValues...>& depPack, FIn&& func) -> Events<TOut>
{
    using REACT_IMPL::SyncedEventProcessingNode;

    using F = typename std::decay<FIn>::type;

    struct NodeBuilder_
    {
        NodeBuilder_(const Events<TIn>& source, FIn&& func) :
            MySource( source ),
            MyFunc( std::forward<FIn>(func) )
        {}

        auto operator()(const Signal<D,TDepValues>& ... deps)
            -> Events<TOut>
        {
            return Events<TOut>(
                std::make_shared<SyncedEventProcessingNode<D,TIn,TOut,F,TDepValues ...>>(
                     GetNodePtr(MySource), std::forward<FIn>(MyFunc), GetNodePtr(deps) ...));
        }

        const Events<TIn>&    MySource;
        FIn                     MyFunc;
    };

    return REACT_IMPL::apply(
        NodeBuilder_( source, std::forward<FIn>(func) ),
        depPack.Data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Flatten
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TInnerValue>
auto Flatten(const Signal<Events<TInnerValue>>& outer) -> Events<TInnerValue>
{
    return Events<TInnerValue>(
        std::make_shared<REACT_IMPL::EventFlattenNode<Events<TInnerValue>, TInnerValue>>(
            GetNodePtr(outer), GetNodePtr(outer.Value())));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Join
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename ... TArgs>
auto Join(const Events<TArgs>& ... args) -> Events<std::tuple<TArgs ...>>
{
    using REACT_IMPL::EventJoinNode;

    static_assert(sizeof...(TArgs) > 1, "Join: 2+ arguments are required.");

    return Events< std::tuple<TArgs ...>>(
        std::make_shared<EventJoinNode<TArgs...>>(
            GetNodePtr(args) ...));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Token
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class Token { value };

struct Tokenizer
{
    template <typename T>
    Token operator()(const T&) const { return Token::value; }
};

template <typename T>
auto Tokenize(T&& source) -> decltype(auto)
{
    return Transform(source, Tokenizer{ });
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Events
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T = Token>
class Events : public REACT_IMPL::EventStreamBase<T>
{
private:
    using NodeType     = REACT_IMPL::EventStreamNode<T>;
    using NodePtrType  = std::shared_ptr<NodeType>;

public:
    using ValueType = T;

    // Default ctor
    Events() = default;

    // Copy ctor
    Events(const Events&) = default;

    // Move ctor
    Events(Events&& other) = default;

    // Node ctor
    explicit Events(NodePtrT&& nodePtr) = default;

    // Copy assignment
    Events& operator=(const Events&) = default;

    // Move assignment
    Events& operator=(Events&& other) = default;

    bool Equals(const Events& other) const
    {
        return Events::EventStreamBase::Equals(other);
    }

    bool IsValid() const
    {
        return Events::EventStreamBase::IsValid();
    }

    void SetWeightHint(WeightHint weight)
    {
        Events::EventStreamBase::SetWeightHint(weight);
    }

    auto Tokenize() const -> decltype(auto)
    {
        return REACT::Tokenize(*this);
    }

    template <typename ... TArgs>
    auto Merge(TArgs&& ... args) const -> decltype(auto)
    {
        return REACT::Merge(*this, std::forward<TArgs>(args) ...);
    }

    template <typename F>
    auto Filter(F&& f) const -> decltype(auto)
    {
        return REACT::Filter(*this, std::forward<F>(f));
    }

    template <typename F>
    auto Transform(F&& f) const -> decltype(auto)
    {
        return REACT::Transform(*this, std::forward<F>(f));
    }

    template <typename E = Token>
    static auto Create() -> EventSource<E>
    {
        using REACT_IMPL::EventSourceNode;

        return EventSource<E>(std::make_shared<EventSourceNode<E>>());
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// EventSource
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T = Token>
class EventSource : public Events<T>
{
private:
    using NodeType     = REACT_IMPL::EventSourceNode<T>;
    using NodePtrType  = std::shared_ptr<NodeType>;

public:
    // Default ctor
    EventSource() = default;

    // Copy ctor
    EventSource(const EventSource&) = default;

    // Move ctor
    EventSource(EventSource&& other) = default;

    // Node ctor
    explicit EventSource(NodePtrT&& nodePtr) :
        EventSource::Events( std::move(nodePtr) )
    {}

    // Copy assignemnt
    EventSource& operator=(const EventSource&) = default;

    // Move assignment
    EventSource& operator=(EventSource&& other) = default;

    // Explicit emit
    void Emit(const T& e) const     { EventSource::EventStreamBase::emit(e); }
    void Emit(T&& e) const          { EventSource::EventStreamBase::emit(std::move(e)); }

    void Emit() const
    {
        static_assert(std::is_same<E,Token>::value, "Can't emit on non token stream.");
        EventSource::EventStreamBase::emit(Token::value);
    }

    // Function object style
    void operator()(const E& e) const   { EventSource::EventStreamBase::emit(e); }
    void operator()(E&& e) const        { EventSource::EventStreamBase::emit(std::move(e)); }

    void operator()() const
    {
        static_assert(std::is_same<T,Token>::value, "Can't emit on non token stream.");
        EventSource::EventStreamBase::emit(Token::value);
    }

    // Stream style
    const EventSource& operator<<(const T& e) const
    {
        EventSource::EventStreamBase::emit(e);
        return *this;
    }

    const EventSource& operator<<(T&& e) const
    {
        EventSource::EventStreamBase::emit(std::move(e));
        return *this;
    }
};

/******************************************/ REACT_END /******************************************/

/***************************************/ REACT_IMPL_BEGIN /**************************************/

template <typename L, typename R>
bool Equals(const Events<L>& lhs, const Events<R>& rhs)
{
    return lhs.Equals(rhs);
}

/****************************************/ REACT_IMPL_END /***************************************/

#endif // REACT_EVENT_H_INCLUDED
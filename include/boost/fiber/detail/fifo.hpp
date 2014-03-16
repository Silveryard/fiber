
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_FIFO_H
#define BOOST_FIBERS_DETAIL_FIFO_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/worker_fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

class fifo : private noncopyable
{
public:
    typedef worker_fiber::ptr_t   ptr_t;

    fifo() BOOST_NOEXCEPT :
        head_(),
        tail_()
    {}

    bool empty() const BOOST_NOEXCEPT
    { return 0 == head_.get(); }

    std::size_t size() const BOOST_NOEXCEPT
    {
        std::size_t counter = 0; 
        for ( ptr_t x = head_; x; x = x->next() )
            ++counter;
        return counter;
    }

    void push( ptr_t const& item) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( item);

        if ( empty() )
        {
            head_ = tail_ = item;
            return;
        }
        tail_->next( item);
        tail_ = tail_->next();
    }

    ptr_t head() const BOOST_NOEXCEPT
    { return head_; }

    void top( ptr_t const& item) BOOST_NOEXCEPT
    { head_ = item; }

    ptr_t tail() const BOOST_NOEXCEPT
    { return tail_; }

    void tail( ptr_t const& item) BOOST_NOEXCEPT
    { tail_ = item; }

    ptr_t pop() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! empty() );

        ptr_t item = head_;
        head_ = head_->next();
        if ( ! head_)
            tail_ = head_;
        else
            item->next_reset();
        return item;
    }

    ptr_t find( ptr_t const& item) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( item);

        for ( ptr_t x = head_; x; x = x->next() )
            if ( item == x) return x;
        return ptr_t();
    }

    void erase( ptr_t const& item) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( item);
        BOOST_ASSERT( ! empty() );

        if ( item == head_)
        {
            pop();
            return;
        }
        for ( ptr_t x = head_; x; x = x->next() )
        {
            ptr_t nxt = x->next();
            if ( ! nxt) return;
            if ( item == nxt)
            {
                if ( tail_ == nxt) tail_ = x;
                x->next( nxt->next() );
                nxt->next_reset();
                return;
            }
        }
    }

    template< typename Queue, typename Fn >
    void move_to( Queue & queue, Fn fn)
    {
        for ( ptr_t f = head_, prev = head_; f; )
        {
            ptr_t nxt = f->next();
            if ( fn( f) )
            {
                if ( f == head_)
                {
                    head_ = nxt;
                    if ( ! head_)
                        tail_ = head_;
                    else
                    {
                        head_ = nxt;
                        prev = nxt;
                    }
                }
                else
                {
                    if ( ! nxt)
                        tail_ = prev;

                    prev->next( nxt); 
                }
                f->next_reset();
                queue.push_back( f);
            }
            else
                prev = f;
            f = nxt;
        }
    }

    void swap( fifo & other)
    {
        head_.swap( other.head_);
        tail_.swap( other.tail_);
    }

private:
    ptr_t   head_;
    ptr_t   tail_;
};

}}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_FIFO_H
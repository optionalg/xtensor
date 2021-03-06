/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_STRIDED_VIEW_HPP
#define XTENSOR_STRIDED_VIEW_HPP

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "xtl/xsequence.hpp"
#include "xtl/xvariant.hpp"

#include "xarray.hpp"
#include "xexpression.hpp"
#include "xiterable.hpp"
#include "xstrides.hpp"
#include "xutils.hpp"
#include "xview.hpp"

namespace xt
{
    template <class CT, class S, class CD>
    class xstrided_view;

    template <class CT, class S, class CD>
    struct xcontainer_inner_types<xstrided_view<CT, S, CD>>
    {
        using xexpression_type = std::decay_t<CT>;
        using temporary_type = xarray<typename xexpression_type::value_type>;
    };

    namespace detail
    {
        template <class T>
        struct is_indexed_stepper
        {
            static const bool value = false;
        };

        template <class T>
        struct is_indexed_stepper<xindexed_stepper<T>>
        {
            static const bool value = true;
        };
    }


    template <class CT, class S, class CD>
    struct xiterable_inner_types<xstrided_view<CT, S, CD>>
    {
        using inner_shape_type = S;
        using inner_strides_type = inner_shape_type;
        using inner_backstrides_type_type = inner_shape_type;

        using const_stepper = std::conditional_t<
                detail::is_indexed_stepper<typename std::decay_t<CT>::stepper>::value,
                xindexed_stepper<xstrided_view<CT, S, CD>>,
                xstepper<const xstrided_view<CT, S, CD>>
            >;

        using stepper = xstepper<xstrided_view<CT, S, CD>>;
    };

    /*****************
     * xstrided_view *
     *****************/

    /**
     * @class xstrided_view
     * @brief View of an xexpression using strides
     *
     * The xstrided_view class implements a view utilizing an offset and strides
     * into a multidimensional xcontainer. The xstridedview is currently used
     * to implement `transpose`.
     * @tparam CT the closure type of the \ref xexpression type underlying this view
     * @tparam CD the closure type of the underlying data container
     *
     * @sa strided_view, transpose
     */
    template <class CT, class S, class CD>
    class xstrided_view : public xview_semantic<xstrided_view<CT, S, CD>>,
                          public xiterable<xstrided_view<CT, S, CD>>
    {
    public:

        using self_type = xstrided_view<CT, S, CD>;
        using xexpression_type = std::decay_t<CT>;
        using semantic_base = xview_semantic<self_type>;

        using value_type = typename xexpression_type::value_type;
        using reference = typename xexpression_type::reference;
        using const_reference = typename xexpression_type::const_reference;
        using pointer = typename xexpression_type::pointer;
        using const_pointer = typename xexpression_type::const_pointer;
        using size_type = typename xexpression_type::size_type;
        using difference_type = typename xexpression_type::difference_type;

        using underlying_container_type = CD;

        using iterable_base = xiterable<self_type>;
        using inner_shape_type = typename iterable_base::inner_shape_type;
        using shape_type = inner_shape_type;
        using strides_type = shape_type;
        using backstrides_type = shape_type;
        using closure_type = const self_type;

        using stepper = typename iterable_base::stepper;
        using const_stepper = typename iterable_base::const_stepper;

        static constexpr layout_type static_layout = layout_type::dynamic;
        static constexpr bool contiguous_layout = false;

        using temporary_type = typename xcontainer_inner_types<self_type>::temporary_type;
        using base_index_type = xindex_type_t<shape_type>;

        xstrided_view(CT e, S&& shape, S&& strides, std::size_t offset, layout_type layout) noexcept;
        xstrided_view(CT e, CD data, S&& shape, S&& strides, std::size_t offset, layout_type layout) noexcept;

        template <class E>
        self_type& operator=(const xexpression<E>& e);

        template <class E>
        disable_xexpression<E, self_type>& operator=(const E& e);

        size_type size() const noexcept;
        size_type dimension() const noexcept;
        const shape_type& shape() const noexcept;
        const strides_type& strides() const noexcept;
        const backstrides_type& backstrides() const noexcept;
        layout_type layout() const noexcept;

        reference operator()();
        template <class... Args>
        reference operator()(Args... args);
        template <class... Args>
        reference at(Args... args);
        template <class OS>
        disable_integral_t<OS, reference> operator[](const OS& index);
        template <class I>
        reference operator[](std::initializer_list<I> index);
        reference operator[](size_type i);

        template <class It>
        reference element(It first, It last);

        const_reference operator()() const;
        template <class... Args>
        const_reference operator()(Args... args) const;
        template <class... Args>
        const_reference at(Args... args) const;
        template <class OS>
        disable_integral_t<OS, const_reference> operator[](const OS& index) const;
        template <class I>
        const_reference operator[](std::initializer_list<I> index) const;
        const_reference operator[](size_type i) const;

        template <class It>
        const_reference element(It first, It last) const;

        template <class O>
        bool broadcast_shape(O& shape, bool reuse_cache = false) const;

        template <class O>
        bool is_trivial_broadcast(const O& strides) const noexcept;

        template <class ST>
        stepper stepper_begin(const ST& shape);
        template <class ST>
        stepper stepper_end(const ST& shape, layout_type l);

        template <class ST, class STEP = const_stepper>
        std::enable_if_t<!detail::is_indexed_stepper<STEP>::value, STEP>
        stepper_begin(const ST& shape) const;
        template <class ST, class STEP = const_stepper>
        std::enable_if_t<!detail::is_indexed_stepper<STEP>::value, STEP>
        stepper_end(const ST& shape, layout_type l) const;

        template <class ST, class STEP = const_stepper>
        std::enable_if_t<detail::is_indexed_stepper<STEP>::value, STEP>
        stepper_begin(const ST& shape) const;
        template <class ST, class STEP = const_stepper>
        std::enable_if_t<detail::is_indexed_stepper<STEP>::value, STEP>
        stepper_end(const ST& shape, layout_type l) const;

        using container_iterator = typename std::decay_t<CD>::iterator;
        using const_container_iterator = typename std::decay_t<CD>::const_iterator;

        underlying_container_type& data() noexcept;
        const underlying_container_type& data() const noexcept;

        value_type* raw_data() noexcept;
        const value_type* raw_data() const noexcept;

        size_type raw_data_offset() const noexcept;

    protected:

        container_iterator data_xbegin() noexcept;
        const_container_iterator data_xbegin() const noexcept;
        container_iterator data_xend(layout_type l) noexcept;
        const_container_iterator data_xend(layout_type l) const noexcept;

    private:

        template <class C>
        friend class xstepper;

        template <class It>
        It data_xbegin_impl(It begin) const noexcept;

        template <class It>
        It data_xend_impl(It end, layout_type l) const noexcept;

        void assign_temporary_impl(temporary_type&& tmp);

        CT m_e;
        CD m_data;
        shape_type m_shape;
        strides_type m_strides;
        backstrides_type m_backstrides;
        std::size_t m_offset;
        layout_type m_layout;

        friend class xview_semantic<xstrided_view<CT, S, CD>>;
    };

    /********************************
     * xstrided_view implementation *
     ********************************/

    /**
     * @name Constructor
     */
    //@{
    /**
     * Constructs an xstrided_view
     *
     * @param e the underlying xexpression for this view
     * @param shape the shape of the view
     * @param strides the strides of the view
     * @param offset the offset of the first element in the underlying container
     * @param layout the layout of the view
     */
    template <class CT, class S, class CD>
    inline xstrided_view<CT, S, CD>::xstrided_view(CT e, S&& shape, S&& strides, std::size_t offset, layout_type layout) noexcept
        : m_e(e), m_data(m_e.data()), m_shape(std::forward<S>(shape)), m_strides(std::forward<S>(strides)), m_offset(offset), m_layout(layout)
    {
        m_backstrides = xtl::make_sequence<backstrides_type>(m_shape.size(), 0);
        adapt_strides(m_shape, m_strides, m_backstrides);
    }

    template <class CT, class S, class CD>
    inline xstrided_view<CT, S, CD>::xstrided_view(CT e, CD data, S&& shape, S&& strides, std::size_t offset, layout_type layout) noexcept
        : m_e(e), m_data(data), m_shape(std::forward<S>(shape)), m_strides(std::forward<S>(strides)), m_offset(offset), m_layout(layout)
    {
        m_backstrides = xtl::make_sequence<backstrides_type>(m_shape.size(), 0);
        adapt_strides(m_shape, m_strides, m_backstrides);
    }
    //@}

    /**
     * @name Extended copy semantic
     */
    //@{
    /**
     * The extended assignment operator.
     */
    template <class CT, class S, class CD>
    template <class E>
    inline auto xstrided_view<CT, S, CD>::operator=(const xexpression<E>& e) -> self_type&
    {
        return semantic_base::operator=(e);
    }
    //@}

    template <class CT, class S, class CD>
    template <class E>
    inline auto xstrided_view<CT, S, CD>::operator=(const E& e) -> disable_xexpression<E, self_type>&
    {
        std::fill(this->begin(), this->end(), e);
        return *this;
    }

    template <class CT, class S, class CD>
    inline void xstrided_view<CT, S, CD>::assign_temporary_impl(temporary_type&& tmp)
    {
        std::copy(tmp.cbegin(), tmp.cend(), this->begin());
    }

    /**
     * @name Size and shape
     */
    //@{
    /**
     * Returns the size of the xstrided_view.
     */
    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::size() const noexcept -> size_type
    {
        return compute_size(shape());
    }

    /**
     * Returns the number of dimensions of the xstrided_view.
     */
    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::dimension() const noexcept -> size_type
    {
        return m_shape.size();
    }

    /**
     * Returns the shape of the xstrided_view.
     */
    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::shape() const noexcept -> const shape_type&
    {
        return m_shape;
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::strides() const noexcept -> const strides_type&
    {
        return m_strides;
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::backstrides() const noexcept -> const backstrides_type&
    {
        return m_backstrides;
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::layout() const noexcept -> layout_type
    {
        return m_layout;
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data() noexcept -> underlying_container_type&
    {
        return m_e.data();
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data() const noexcept -> const underlying_container_type&
    {
        return m_e.data();
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::raw_data() noexcept -> value_type*
    {
        return m_e.raw_data();
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::raw_data() const noexcept -> const value_type*
    {
        return m_e.raw_data();
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::raw_data_offset() const noexcept -> size_type
    {
        return m_offset;
    }
    //@}

    /**
     * @name Data
     */
    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::operator()() -> reference
    {
        return m_data[m_offset];
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::operator()() const -> const_reference
    {
        return m_data[m_offset];
    }

    template <class CT, class S, class CD>
    template <class... Args>
    inline auto xstrided_view<CT, S, CD>::operator()(Args... args) -> reference
    {
        XTENSOR_TRY(check_index(shape(), args...));
        size_type index = m_offset + data_offset<size_type>(strides(), static_cast<size_type>(args)...);
        return m_data[index];
    }

    /**
     * Returns the element at the specified position in the xstrided_view.
     *
     * @param args a list of indices specifying the position in the view. Indices
     * must be unsigned integers, the number of indices should be equal or greater than
     * the number of dimensions of the view.
     */
    template <class CT, class S, class CD>
    template <class... Args>
    inline auto xstrided_view<CT, S, CD>::operator()(Args... args) const -> const_reference
    {
        XTENSOR_TRY(check_index(shape(), args...));
        size_type index = m_offset + data_offset<size_type>(strides(), static_cast<size_type>(args)...);
        return m_data[index];
    }

    /**
     * Returns a reference to the element at the specified position in the expression,
     * after dimension and bounds checking.
     * @param args a list of indices specifying the position in the function. Indices
     * must be unsigned integers, the number of indices should be equal to the number of dimensions
     * of the expression.
     * @exception std::out_of_range if the number of argument is greater than the number of dimensions
     * or if indices are out of bounds.
     */
    template <class CT, class S, class CD>
    template <class... Args>
    inline auto xstrided_view<CT, S, CD>::at(Args... args) -> reference
    {
        check_access(shape(), static_cast<size_type>(args)...);
        return this->operator()(args...);
    }

    /**
     * Returns a constant reference to the element at the specified position in the expression,
     * after dimension and bounds checking.
     * @param args a list of indices specifying the position in the function. Indices
     * must be unsigned integers, the number of indices should be equal to the number of dimensions
     * of the expression.
     * @exception std::out_of_range if the number of argument is greater than the number of dimensions
     * or if indices are out of bounds.
     */
    template <class CT, class S, class CD>
    template <class... Args>
    inline auto xstrided_view<CT, S, CD>::at(Args... args) const -> const_reference
    {
        check_access(shape(), static_cast<size_type>(args)...);
        return this->operator()(args...);
    }

    template <class CT, class S, class CD>
    template <class OS>
    inline auto xstrided_view<CT, S, CD>::operator[](const OS& index)
        -> disable_integral_t<OS, reference>
    {
        return element(index.cbegin(), index.cend());
    }

    template <class CT, class S, class CD>
    template <class I>
    inline auto xstrided_view<CT, S, CD>::operator[](std::initializer_list<I> index)
        -> reference
    {
        return element(index.begin(), index.end());
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::operator[](size_type i) -> reference
    {
        return operator()(i);
    }

    template <class CT, class S, class CD>
    template <class OS>
    inline auto xstrided_view<CT, S, CD>::operator[](const OS& index) const
        -> disable_integral_t<OS, const_reference>
    {
        return element(index.cbegin(), index.cend());
    }

    template <class CT, class S, class CD>
    template <class I>
    inline auto xstrided_view<CT, S, CD>::operator[](std::initializer_list<I> index) const
        -> const_reference
    {
        return element(index.begin(), index.end());
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::operator[](size_type i) const -> const_reference
    {
        return operator()(i);
    }

    /**
     * Returns a reference to the element at the specified position in the xstrided_view.
     * @param first iterator starting the sequence of indices
     * @param last iterator ending the sequence of indices
     * The number of indices in the sequence should be equal to or greater than the the number
     * of dimensions of the container..
     */
    template <class CT, class S, class CD>
    template <class It>
    inline auto xstrided_view<CT, S, CD>::element(It first, It last) -> reference
    {
        return m_data[m_offset + element_offset<size_type>(strides(), first, last)];
    }

    template <class CT, class S, class CD>
    template <class It>
    inline auto xstrided_view<CT, S, CD>::element(It first, It last) const -> const_reference
    {
        return m_data[m_offset + element_offset<size_type>(strides(), first, last)];
    }
    //@}

    /**
     * @name Broadcasting
     */
    //@{
    /**
     * Broadcast the shape of the xstrided_view to the specified parameter.
     * @param shape the result shape
     * @return a boolean indicating whether the broadcasting is trivial
     */
    template <class CT, class S, class CD>
    template <class O>
    inline bool xstrided_view<CT, S, CD>::broadcast_shape(O& shape, bool) const
    {
        return xt::broadcast_shape(m_shape, shape);
    }

    /**
     * Compares the specified strides with those of the container to see whether
     * the broadcasting is trivial.
     * @return a boolean indicating whether the broadcasting is trivial
     */
    template <class CT, class S, class CD>
    template <class O>
    inline bool xstrided_view<CT, S, CD>::is_trivial_broadcast(const O& str) const noexcept
    {
        return str.size() == strides().size() &&
            std::equal(str.cbegin(), str.cend(), strides().begin());
    }
    //@}

    /***************
     * stepper api *
     ***************/

    template <class CT, class S, class CD>
    template <class ST>
    inline auto xstrided_view<CT, S, CD>::stepper_begin(const ST& shape) -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(this, data_xbegin(), offset);
    }

    template <class CT, class S, class CD>
    template <class ST>
    inline auto xstrided_view<CT, S, CD>::stepper_end(const ST& shape, layout_type l) -> stepper
    {
        size_type offset = shape.size() - dimension();
        return stepper(this, data_xend(l), offset);
    }

    template <class CT, class S, class CD>
    template <class ST, class STEP>
    inline auto xstrided_view<CT, S, CD>::stepper_begin(const ST& shape) const -> std::enable_if_t<!detail::is_indexed_stepper<STEP>::value, STEP>
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, data_xbegin(), offset);
    }

    template <class CT, class S, class CD>
    template <class ST, class STEP>
    inline auto xstrided_view<CT, S, CD>::stepper_end(const ST& shape, layout_type l) const -> std::enable_if_t<!detail::is_indexed_stepper<STEP>::value, STEP>
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, data_xend(l), offset);
    }

    template <class CT, class S, class CD>
    template <class ST, class STEP>
    inline auto xstrided_view<CT, S, CD>::stepper_begin(const ST& shape) const -> std::enable_if_t<detail::is_indexed_stepper<STEP>::value, STEP>
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, offset);
    }

    template <class CT, class S, class CD>
    template <class ST, class STEP>
    inline auto xstrided_view<CT, S, CD>::stepper_end(const ST& shape, layout_type /*l*/) const -> std::enable_if_t<detail::is_indexed_stepper<STEP>::value, STEP>
    {
        size_type offset = shape.size() - dimension();
        return const_stepper(this, offset, true);
    }

    template <class CT, class S, class CD>
    template <class It>
    inline It xstrided_view<CT, S, CD>::data_xbegin_impl(It begin) const noexcept
    {
        return begin + m_offset;
    }

    template <class CT, class S, class CD>
    template <class It>
    inline It xstrided_view<CT, S, CD>::data_xend_impl(It end, layout_type l) const noexcept
    {
        return strided_data_end(*this, end, l);
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data_xbegin() noexcept -> container_iterator
    {
        return data_xbegin_impl(m_data.begin());
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data_xbegin() const noexcept -> const_container_iterator
    {
        return data_xbegin_impl(m_data.begin());
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data_xend(layout_type l) noexcept -> container_iterator
    {
        return data_xend_impl(m_data.end(), l);
    }

    template <class CT, class S, class CD>
    inline auto xstrided_view<CT, S, CD>::data_xend(layout_type l) const noexcept -> const_container_iterator
    {
        return data_xend_impl(m_data.end(), l);
    }

    /**
     * Construct a strided view from an xexpression, shape, strides and offset.
     *
     * @param e xexpression
     * @param shape the shape of the view
     * @param strides the new strides of the view
     * @param offset the offset of the first element in the underlying container
     * @param layout the new layout of the expression
     *
     * @tparam E type of xexpression
     * @tparam I shape and strides type
     *
     * @return the view
     */
    template <class E, class I>
    inline auto strided_view(E&& e, I&& shape, I&& strides, std::size_t offset = 0, layout_type layout = layout_type::dynamic) noexcept
    {
        using view_type = xstrided_view<xclosure_t<E>, I, decltype(e.data())>;
        return view_type(std::forward<E>(e), std::forward<I>(shape), std::forward<I>(strides), offset, layout);
    }

    /****************************
     * transpose implementation *
     ****************************/

    namespace detail
    {
        template <class E, class S>
        inline auto transpose_impl(E&& e, S&& permutation, check_policy::none)
        {
            if (sequence_size(permutation) != e.dimension())
            {
                throw transpose_error("Permutation does not have the same size as shape");
            }

            // permute stride and shape
            using strides_type = typename std::decay_t<E>::strides_type;
            strides_type temp_strides;
            resize_container(temp_strides, e.strides().size());

            using shape_type = typename std::decay_t<E>::shape_type;
            shape_type temp_shape;
            resize_container(temp_shape, e.shape().size());

            using size_type = typename std::decay_t<E>::size_type;
            for (std::size_t i = 0; i < e.shape().size(); ++i)
            {
                if (std::size_t(permutation[i]) >= e.dimension())
                {
                    throw transpose_error("Permutation contains wrong axis");
                }
                size_type perm = static_cast<size_type>(permutation[i]);
                temp_shape[i] = e.shape()[perm];
                temp_strides[i] = e.strides()[perm];
            }

            layout_type new_layout = layout_type::dynamic;
            if (std::is_sorted(std::begin(permutation), std::end(permutation)))
            {
                // keep old layout
                new_layout = e.layout();
            }
            else if (std::is_sorted(std::begin(permutation), std::end(permutation), std::greater<>()))
            {
                // this swaps the layout
                if (e.layout() == layout_type::row_major)
                {
                    new_layout = layout_type::column_major;
                }
                else if (e.layout() == layout_type::column_major)
                {
                    new_layout = layout_type::row_major;
                }
            }

            using view_type = xstrided_view<xclosure_t<E>, shape_type, decltype(e.data())>;
            return view_type(std::forward<E>(e), std::move(temp_shape), std::move(temp_strides), 0, new_layout);
        }

        template <class E, class S>
        inline auto transpose_impl(E&& e, S&& permutation, check_policy::full)
        {
            // check if axis appears twice in permutation
            for (std::size_t i = 0; i < sequence_size(permutation); ++i)
            {
                for (std::size_t j = i + 1; j < sequence_size(permutation); ++j)
                {
                    if (permutation[i] == permutation[j])
                    {
                        throw transpose_error("Permutation contains axis more than once");
                    }
                }
            }
            return transpose_impl(std::forward<E>(e), std::forward<S>(permutation), check_policy::none());
        }
    }

    /**
     * Returns a transpose view by reversing the dimensions of xexpression e
     * @param e the input expression
     */
    template <class E>
    inline auto transpose(E&& e) noexcept
    {
        using shape_type = typename std::decay_t<E>::shape_type;

        shape_type shape;
        resize_container(shape, e.shape().size());
        std::copy(e.shape().rbegin(), e.shape().rend(), shape.begin());

        shape_type strides;
        resize_container(strides, e.strides().size());
        std::copy(e.strides().rbegin(), e.strides().rend(), strides.begin());

        layout_type new_layout = layout_type::dynamic;
        if (e.layout() == layout_type::row_major)
        {
            new_layout = layout_type::column_major;
        }
        else if (e.layout() == layout_type::column_major)
        {
            new_layout = layout_type::row_major;
        }

        using view_type = xstrided_view<xclosure_t<E>, shape_type, decltype(e.data())>;
        return view_type(std::forward<E>(e), std::move(shape), std::move(strides), 0, new_layout);
    }

    /**
     * Returns a transpose view by permuting the xexpression e with @p permutation.
     * @param e the input expression
     * @param permutation the sequence containing permutation
     * @param check_policy the check level (check_policy::full() or check_policy::none())
     * @tparam Tag selects the level of error checking on permutation vector defaults to check_policy::none.
     */
    template <class E, class S, class Tag = check_policy::none>
    inline auto transpose(E&& e, S&& permutation, Tag check_policy = Tag())
    {
        return detail::transpose_impl(std::forward<E>(e), std::forward<S>(permutation), check_policy);
    }

    /// @cond DOXYGEN_INCLUDE_SFINAE
#ifdef X_OLD_CLANG
    template <class E, class I, class Tag = check_policy::none>
    inline auto transpose(E&& e, std::initializer_list<I> permutation, Tag check_policy = Tag())
    {
        dynamic_shape<I> perm(permutation);
        return detail::transpose_impl(std::forward<E>(e), std::move(perm), check_policy);
    }
#else
    template <class E, class I, std::size_t N, class Tag = check_policy::none>
    inline auto transpose(E&& e, const I (&permutation)[N], Tag check_policy = Tag())
    {
        return detail::transpose_impl(std::forward<E>(e), permutation, check_policy);
    }
#endif
    /// @endcond

    namespace detail
    {
        template <class CT>
        class expression_adaptor
        {
        public:

            using xexpression_type = std::decay_t<CT>;
            using shape_type = typename xexpression_type::shape_type;
            using index_type = xindex_type_t<shape_type>;
            using size_type = typename xexpression_type::size_type;
            using value_type = typename xexpression_type::value_type;
            using reference = typename xexpression_type::reference;

            using iterator = typename xexpression_type::iterator;
            using const_iterator = typename xexpression_type::const_iterator;

            expression_adaptor(CT&& e)
                : m_e(e)
            {
                resize_container(m_index, m_e.dimension());
                resize_container(m_strides, m_e.dimension());
                m_size = compute_size(m_e.shape());
                compute_strides(m_e.shape(), DEFAULT_LAYOUT, m_strides);
            }

            const reference operator[](std::size_t idx) const
            {
                std::size_t quot;
                for (size_type i = 0; i < m_strides.size(); ++i)
                {
                    quot = idx / m_strides[i];
                    idx = idx % m_strides[i];
                    m_index[i] = quot;
                }
                return m_e.element(m_index.cbegin(), m_index.cend());
            }

            size_type size() const
            {
                return m_size;
            }

        private:

            CT m_e;
            shape_type m_strides;
            mutable index_type m_index;
            size_type m_size;
        };

        template <class E>
        struct slice_getter_impl {
            E& m_expr;
            mutable std::size_t idx;

            slice_getter_impl(E& expr)
                : m_expr(expr), idx(0)
            {
            }

            template <class T>
            std::array<int, 3> operator()(const T& t) const {
                auto sl = get_slice_implementation(m_expr, t, idx);
                return std::array<int, 3>({int(sl(0)), int(sl.size()), int(sl.step_size())});
            }

            std::array<int, 3> operator()(const int& /*t*/) const {
                return std::array<int, 3>({0, 0, 0});
            }
        };
    }


    template <class T>
    using slice_variant = xtl::variant<
        T,

        xt::xrange_adaptor<xt::placeholders::xtuph, T, T>,
        xt::xrange_adaptor<T, xt::placeholders::xtuph, T>,
        xt::xrange_adaptor<T, T, xt::placeholders::xtuph>,

        xt::xrange_adaptor<T, xt::placeholders::xtuph, xt::placeholders::xtuph>,
        xt::xrange_adaptor<xt::placeholders::xtuph, T, xt::placeholders::xtuph>,
        xt::xrange_adaptor<xt::placeholders::xtuph, xt::placeholders::xtuph, T>,

        xt::xrange_adaptor<T, T, T>,
        xt::xrange_adaptor<xt::placeholders::xtuph, xt::placeholders::xtuph, xt::placeholders::xtuph>,

        xt::xall_tag,
        xt::xnewaxis_tag
    >;

    using slice_vector = std::vector<slice_variant<int>>;

    namespace detail
    {
        template <class E, std::enable_if_t<has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline auto&& get_data(E&& e)
        {
            return e.data();
        }

        template <class E, std::enable_if_t<has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline std::size_t get_offset(E&& e)
        {
            return e.raw_data_offset();
        }

        template <class E, std::enable_if_t<has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline auto&& get_strides(E&& e)
        {
            return e.strides();
        }

        template <class E, std::enable_if_t<!has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline auto get_data(E&& e) -> expression_adaptor<xclosure_t<E>>
        {
            return std::move(expression_adaptor<xclosure_t<E>>(e));
        }

        template <class E, std::enable_if_t<!has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline std::size_t get_offset(E&& /*e*/)
        {
            return std::size_t(0);
        }

        template <class E, std::enable_if_t<!has_raw_data_interface<std::decay_t<E>>::value>* = nullptr>
        inline auto get_strides(E&& e)
        {
            dynamic_shape<std::size_t> strides;
            strides.resize(e.shape().size());
            compute_strides(e.shape(), DEFAULT_LAYOUT, strides);
            return strides;
        }
    }

    /**
     * Function to create a dynamic view from
     * a xexpression and a slice_vector.
     *
     * @param e xexpression
     * @param slices the slice vector
     *
     * @return initialized strided_view according to slices
     *
     * \code{.cpp}
     * xt::xarray<double> a = {{1, 2, 3}, {4, 5, 6}};
     * xt::slice_vector sv({xt::range(0, 1)});
     * sv.push_back(xt::range(0, 3, 2));
     * auto v = xt::dynamic_view(a, sv);
     * // ==> {{1, 3}}
     * \endcode
     * 
     * You can also achieve the same with the following short-hand syntax:
     * 
     * \code{.cpp}
     * xt::xarray<double> a = {{1, 2, 3}, {4, 5, 6}};
     * auto v = xt::dynamic_view(a, {xt::range(0, 1), xt::range(0, 3, 2)});
     * // ==> {{1, 3}}
     * \endcode
     */
    template <class E>
    inline auto dynamic_view(E&& e, const slice_vector& slices)
    {
        // Compute dimension
        std::size_t dimension = e.dimension();

        for (const auto& el : slices)
        {
            if (xtl::get_if<xt::xnewaxis_tag>(&el) != nullptr)
            {
                dimension++;
            }
            else if(xtl::get_if<int>(&el) != nullptr)
            {
                dimension--;
            }
        }

        // Compute strided view
        std::size_t offset = detail::get_offset(e);
        using shape_type = dynamic_shape<std::size_t>;

        shape_type new_shape(dimension);
        shape_type new_strides(dimension);

        auto old_shape = e.shape();
        auto&& old_strides = detail::get_strides(e);

        std::size_t i = 0, idx = 0, newaxis_skip = 0;

        auto slice_getter = detail::slice_getter_impl<E>(e);

        for (; i < slices.size(); ++i)
        {
            auto ptr = xtl::get_if<int>(&slices[i]);
            if (ptr != nullptr)
            {
                std::size_t slice0 = static_cast<std::size_t>(*ptr);
                offset += slice0 * old_strides[i - newaxis_skip];
            }
            else if (xtl::get_if<xt::xnewaxis_tag>(&slices[i]) != nullptr)
            {
                new_shape[idx] = 1;
                new_strides[idx] = 0;
                ++newaxis_skip;
                ++idx;
            }
            else if (xtl::get_if<xt::xall_tag>(&slices[i]) != nullptr)
            {
                new_shape[idx] = old_shape[i - newaxis_skip];
                new_strides[idx] = old_strides[i - newaxis_skip];
                ++idx;
            }
            else
            {
                slice_getter.idx = i - newaxis_skip;
                std::array<int, 3> info = xtl::visit(slice_getter, slices[i]);
                offset += std::size_t(info[0]) * old_strides[i - newaxis_skip];
                new_shape[idx] = std::size_t(info[1]);
                new_strides[idx] = std::size_t(info[2]) * old_strides[i - newaxis_skip];
                idx++;
            }
        }

        for (; (i - newaxis_skip) < old_shape.size(); ++i)
        {
            new_shape[idx] = old_shape[i - newaxis_skip];
            new_strides[idx] = old_strides[i - newaxis_skip];
            ++idx;
        }

        decltype(auto) data = detail::get_data(e);

        using view_type = xstrided_view<xclosure_t<E>, shape_type, decltype(data)>;
        // TODO change layout type?
        return view_type(std::forward<E>(e), std::forward<decltype(data)>(data), std::move(new_shape), std::move(new_strides), offset, layout_type::dynamic);
    }
}

#endif

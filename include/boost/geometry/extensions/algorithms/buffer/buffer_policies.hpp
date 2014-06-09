// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP


#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/extensions/strategies/buffer_side.hpp>

#include <boost/geometry/algorithms/detail/overlay/backtrack_check_si.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


enum intersection_location_type
{
    location_ok, inside_buffer, inside_original
};

class backtrack_for_buffer
{
public :
    typedef detail::overlay::backtrack_state state_type;

    template <typename Operation, typename Rings, typename Turns, typename Geometry, typename RobustPolicy>
    static inline void apply(std::size_t size_at_start,
                Rings& rings, typename boost::range_value<Rings>::type& ring,
                Turns& turns, Operation& operation,
                std::string const& /*reason*/,
                Geometry const& ,
                Geometry const& ,
                RobustPolicy const& ,
                state_type& state
                )
    {
#if defined(BOOST_GEOMETRY_COUNT_BACKTRACK_WARNINGS)
extern int g_backtrack_warning_count;
g_backtrack_warning_count++;
#endif
//std::cout << "!";
//std::cout << "WARNING " << reason << std::endl;

        // TODO this is a copy of dissolve, check this for buffer
        state.m_good = false;

        // Make bad output clean
        rings.resize(size_at_start);
        ring.clear();

        // Reject this as a starting point
        operation.visited.set_rejected();

        // And clear all visit info
        clear_visit_info(turns);
    }
};

struct turn_assign_for_buffer
{
    static bool const include_no_turn = false;
    static bool const include_degenerate = false;
    static bool const include_opposite = false;

    template
    <
        typename Point1,
        typename Point2,
        typename Turn,
        typename IntersectionInfo,
        typename DirInfo
    >
    static inline void apply(Turn& turn,
            Point1 const& , Point2 const& ,
            IntersectionInfo const& intersection_info,
            DirInfo const& dir_info)
    {
        if (dir_info.opposite && intersection_info.count == 2)
        {
            turn.is_opposite = true;
        }
    }
};

// Should follow traversal-turn-concept (enrichment, visit structure)
// and adds index in piece vector to find it back
template <typename Point, typename SegmentRatio>
struct buffer_turn_operation
    : public detail::overlay::traversal_turn_operation<Point, SegmentRatio>
{
    int piece_index;
    bool include_in_occupation_map;

    inline buffer_turn_operation()
        : piece_index(-1)
        , include_in_occupation_map(false)
    {}
};

// Version for buffer including type of location, is_opposite, and helper variables
template <typename Point, typename RobustPoint, typename SegmentRatio>
struct buffer_turn_info
    : public detail::overlay::turn_info
        <
            Point,
            SegmentRatio,
            buffer_turn_operation<Point, SegmentRatio>
        >
{
    RobustPoint robust_point;
    bool is_opposite;

    intersection_location_type location;

    int priority;
    int count_within;
    int count_on_occupied;
    int count_on_multi;
#if defined(BOOST_GEOMETRY_COUNT_DOUBLE_UU)
    int count_on_uu;
#endif

    std::set<int> piece_indices_to_skip;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    std::string debug_string;
#endif

    inline buffer_turn_info()
        : is_opposite(false)
        , location(location_ok)
        , priority(0)
        , count_within(0)
        , count_on_occupied(0)
        , count_on_multi(0)
#if defined(BOOST_GEOMETRY_COUNT_DOUBLE_UU)
        , count_on_uu(0)
#endif
    {}
};

struct buffer_operation_less
{
    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        segment_identifier const& sl = left.seg_id;
        segment_identifier const& sr = right.seg_id;

        // Sort them descending
        return sl == sr
            ? left.fraction < right.fraction
            : sl < sr;
    }
};

}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


class si
{
private :
    segment_identifier m_id;

public :
    inline si(segment_identifier const& id)
        : m_id(id)
    {}

    template <typename Char, typename Traits>
    inline friend std::basic_ostream<Char, Traits>& operator<<(
            std::basic_ostream<Char, Traits>& os,
            si const& s)
    {
        os << s.m_id.multi_index << "." << s.m_id.segment_index;
        return os;
    }
};



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP

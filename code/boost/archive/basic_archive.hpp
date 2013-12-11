#ifndef BOOST_ARCHIVE_BASIC_ARCHIVE_HPP
#define BOOST_ARCHIVE_BASIC_ARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// basic_archive.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/integer_traits.hpp>
#include <boost/serialization/strong_typedef.hpp>

#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/detail/abi_prefix.hpp> // must be the last header

#define BOOST_ARCHIVE_STRONG_TYPEDEF(T, D)        \
namespace boost {                                 \
namespace archive {                               \
BOOST_STRONG_TYPEDEF(T, D)                        \
} /* archive */                                   \
template<>                                        \
class integer_traits<boost::archive::D>  :        \
    public integer_traits<boost::T>               \
{};                                               \
} /* boost */                                     \
/**/

BOOST_ARCHIVE_STRONG_TYPEDEF ( uint_least16_t, version_type )
BOOST_ARCHIVE_STRONG_TYPEDEF ( int_least16_t, class_id_type )
BOOST_ARCHIVE_STRONG_TYPEDEF ( int_least16_t, class_id_optional_type )
BOOST_ARCHIVE_STRONG_TYPEDEF ( int_least16_t, class_id_reference_type )
BOOST_ARCHIVE_STRONG_TYPEDEF ( uint_least32_t, object_id_type )
BOOST_ARCHIVE_STRONG_TYPEDEF ( uint_least32_t, object_reference_type )

namespace boost
{
namespace archive
{

struct tracking_type
{
	//    typedef bool value_type;
	bool t;
	explicit tracking_type ( const bool t_ = false )
		: t ( t_ )
	{};
	tracking_type ( const tracking_type &t_ )
		: t ( t_.t )
	{}
	operator bool () const
	{
		return t;
	};
	operator bool &()
	{
		return t;
	};
	tracking_type &operator= ( const bool t_ )
	{
		t = t_;
		return *this;
	}
	bool operator== ( const tracking_type &rhs ) const
	{
		return t == rhs.t;
	}
	bool operator== ( const bool &rhs ) const
	{
		return t == rhs;
	}
	tracking_type &operator= ( const tracking_type &rhs )
	{
		t = rhs.t;
		return *this;
	}
};

struct class_name_type :
		private boost::noncopyable
{
	char *t;
	operator const char *&() const
	{
		return const_cast<const char*&> ( t );
	}
	operator char *()
	{
		return t;
	}
	explicit class_name_type ( const char *key_ )
		: t ( const_cast<char *> ( key_ ) ) {}
	explicit class_name_type ( char *key_ )
		: t ( key_ ) {}
	class_name_type &operator= ( const class_name_type &rhs )
	{
		t = rhs.t;
		return *this;
	}
};

enum archive_flags
{
	no_header = 1,  // suppress archive header info
	no_codecvt = 2,  // suppress alteration of codecvt facet
	no_xml_tag_checking = 4,   // suppress checking of xml tags
	no_tracking = 8,           // suppress ALL tracking
	flags_last = 8
};

#define NULL_POINTER_TAG class_id_type(-1)

BOOST_ARCHIVE_DECL ( const char * )
BOOST_ARCHIVE_SIGNATURE();

BOOST_ARCHIVE_DECL ( version_type )
BOOST_ARCHIVE_VERSION();

}// namespace archive
}// namespace boost

#include <boost/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#include <boost/serialization/level.hpp>

// set implementation level to primitive for all types
// used internally by the serialization library

BOOST_CLASS_IMPLEMENTATION ( boost::archive::version_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::class_id_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::class_id_reference_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::class_id_optional_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::class_name_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::object_id_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::object_reference_type, primitive_type )
BOOST_CLASS_IMPLEMENTATION ( boost::archive::tracking_type, primitive_type )

#endif //BOOST_ARCHIVE_BASIC_ARCHIVE_HPP

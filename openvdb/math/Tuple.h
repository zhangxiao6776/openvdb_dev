///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2013 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////
//
/// @file Tuple.h
/// @author Ben Kwa

#ifndef OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED
#define OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED

#include <sstream>
#include <boost/type_traits/is_integral.hpp>
#include "Math.h"


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace math {

/// @class Tuple "Tuple.h"
/// A base class for homogenous tuple types
template<int SIZE, typename T>
class Tuple {
public:
    typedef T value_type;
    typedef T ValueType;

    static const int size = SIZE;

    /// Default ctor.  Does nothing.  Required because declaring a copy (or
    /// other) constructor means the default constructor gets left out.
    Tuple() {}

    /// Copy constructor.  Used when the class signature matches exactly.
    inline Tuple(Tuple const &src) {
        for (int i = 0; i < SIZE; ++i) {
            mm[i] = src.mm[i];
        }
    }

    /// Conversion constructor.  Tuples with different value types and
    /// different sizes can be interconverted using this member.  Converting
    /// from a larger tuple results in truncation; converting from a smaller
    /// tuple results in the extra data members being zeroed out.  This
    /// function assumes that the integer 0 is convertible to the tuple's
    /// value type.
    template <int src_size, typename src_valtype>
    explicit Tuple(Tuple<src_size, src_valtype> const &src) {
        static const int copyEnd = SIZE < src_size ? SIZE : src_size;

        for (int i = 0; i < copyEnd; ++i) {
            mm[i] = src[i];
        }
        for (int i = copyEnd; i < SIZE; ++i) {
            mm[i] = 0;
        }
    }

    T operator[](int i) const {
        // we'd prefer to use size_t, but can't because gcc3.2 doesn't like
        // it - it conflicts with child class conversion operators to
        // pointer types.
//             assert(i >= 0 && i < SIZE);
        return mm[i];
    }

    T& operator[](int i) {
        // see above for size_t vs int
//             assert(i >= 0 && i < SIZE);
        return mm[i];
    }

    /// @name Compatibility
    /// These are mostly for backwards compability with functions that take
    /// old-style Vs (which are just arrays).
    //@{
    /// Copies this tuple into an array of a compatible type
    template <typename S>
    void toV(S *v) const {
        for (int i = 0; i < SIZE; ++i) {
            v[i] = mm[i];
        }
    }

    /// Exposes the internal array.  Be careful when using this function.
    value_type *asV() {
        return mm;
    }
    /// Exposes the internal array.  Be careful when using this function.
    value_type const *asV() const {
        return mm;
    }
    //@}  Compatibility

    /// @return string representation of Classname
    std::string
    str() const {
        std::ostringstream buffer;

        buffer << "[";

        // For each column
        for (unsigned j(0); j < SIZE; j++) {
            if (j) buffer << ", ";
            buffer << mm[j];
        }

        buffer << "]";

        return buffer.str();
    }

    void write(std::ostream& os) const {
        os.write(reinterpret_cast<const char*>(&mm), sizeof(T)*SIZE);
    }
    void read(std::istream& is) {
        is.read(reinterpret_cast<char*>(&mm), sizeof(T)*SIZE);
    }

protected:
    T mm[SIZE];
};


////////////////////////////////////////


/// @return true if t0 < t1, comparing components in order of significance.
template<int SIZE, typename T0, typename T1>
bool
operator<(const Tuple<SIZE, T0>& t0, const Tuple<SIZE, T1>& t1)
{
    for (size_t i = 0; i < SIZE-1; ++i) {
        if (!isExactlyEqual(t0[i], t1[i])) return t0[i] < t1[i];
    }
    return t0[SIZE-1] < t1[SIZE-1];
}


/// @return true if t0 > t1, comparing components in order of significance.
template<int SIZE, typename T0, typename T1>
bool
operator>(const Tuple<SIZE, T0>& t0, const Tuple<SIZE, T1>& t1)
{
    for (size_t i = 0; i < SIZE-1; ++i) {
        if (!isExactlyEqual(t0[i], t1[i])) return t0[i] > t1[i];
    }
    return t0[SIZE-1] > t1[SIZE-1];
}


////////////////////////////////////////


/// Helper class to compute the absolute value of a Tuple
template<int SIZE, typename T, bool IsInteger>
struct TupleAbs {
    static inline Tuple<SIZE, T> absVal(const Tuple<SIZE, T>& t)
    {
        Tuple<SIZE, T> result;
        for (size_t i = 0; i < SIZE; ++i) result[i] = ::fabs(t[i]);
        return result;
    }
};

// Partial specialization for integer types, using abs() instead of fabs()
template<int SIZE, typename T>
struct TupleAbs<SIZE, T, /*IsInteger=*/true> {
    static inline Tuple<SIZE, T> absVal(const Tuple<SIZE, T>& t)
    {
        Tuple<SIZE, T> result;
        for (size_t i = 0; i < SIZE; ++i) result[i] = ::abs(t[i]);
        return result;
    }
};


/// @return the absolute value of the given Tuple.
template<int SIZE, typename T>
Tuple<SIZE, T>
Abs(const Tuple<SIZE, T>& t)
{
    return TupleAbs<SIZE, T, boost::is_integral<T>::value>::absVal(t);
}


////////////////////////////////////////


/// Write a Tuple to an output stream
template <int SIZE, typename T>
std::ostream& operator<<(std::ostream& ostr, const Tuple<SIZE, T>& classname)
{
    ostr << classname.str();
    return ostr;
}

} // namespace math
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED

// Copyright (c) 2012-2013 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )

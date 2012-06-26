/** @file  bigramiterator.h
 *  @brief Class for iterating over a list of bigrams
 */
/* Copyright (C) 2012 Gaurav Arora
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_BIGRAMITERATOR_H
#define XAPIAN_INCLUDED_BIGRAMITERATOR_H

#include <iterator>
#include <string>

#include <xapian/attributes.h>
#include <xapian/derefwrapper.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

/// Class for iterating over a list of bigrams.
class XAPIAN_VISIBILITY_DEFAULT BigramIterator {
  public:
    /// Class representing the Bigramterator internal.
    class Internal;
    /// @private @internal Reference counted internals.
    Internal * internal;

    /// @private @internal Construct given internals.
    explicit BigramIterator(Internal *internal_);

    /// Copy constructor.
    BigramIterator(const BigramIterator & o);

    /// Assignment.
    BigramIterator & operator=(const BigramIterator & o);

    /** Default constructor.
     *
     *  Creates an uninitialised iterator, which can't be used before being
     *  assigned to, but is sometimes syntactically convenient.
     */
    XAPIAN_NOTHROW(BigramIterator())
	: internal(0) { }

    /// Destructor.
    ~BigramIterator() {
	if (internal) decref();
    }

    /// Return the bigram at the current position.
    std::string operator*() const XAPIAN_PURE_FUNCTION;

    /// Return the wdf for the bigram at the current position.
    Xapian::termcount get_wdf() const XAPIAN_PURE_FUNCTION;

    /// Return the bigram frequency for the term at the current position.
    Xapian::doccount get_termfreq() const XAPIAN_PURE_FUNCTION;

    /// Advance the iterator to the next position.
    BigramIterator & operator++();

    /// Advance the iterator to the next position (postfix version).
    DerefWrapper_<std::string> operator++(int) {
	const std::string & term(**this);
	operator++();
	return DerefWrapper_<std::string>(term);
    }

    /** Advance the iterator to bigram @a bigram.
     *
     *  If the iteration is over an unsorted list of bigrams, then this method
     *  will throw Xapian::InvalidOperationError.
     *
     *  @param bterm	The bigram to advance to.  If this bigram isn't in
     *			the stream being iterated, then the iterator is moved
     *			to the next bigram after it which is.
     */
    void skip_to(const std::string &bterm);

    /// Return a string describing this object.
    std::string get_description() const XAPIAN_PURE_FUNCTION;

    /** @private @internal BigramIterator is what the C++ STL calls an
     *  input_iterator.
     *
     *  The following typedefs allow std::iterator_traits<> to work so that
     *  this iterator can be used with the STL.
     *
     *  These are deliberately hidden from the Doxygen-generated docs, as the
     *  machinery here isn't interesting to API users.  They just need to know
     *  that Xapian iterator classes are compatible with the STL.
     */
    // @{
    /// @private
    typedef std::input_iterator_tag iterator_category;
    /// @private
    typedef std::string value_type;
    /// @private
    typedef Xapian::termcount_diff difference_type;
    /// @private
    typedef std::string * pointer;
    /// @private
    typedef std::string & reference;
    // @}

  private:
    void decref();

    void post_advance(Internal * res);
};

bool
XAPIAN_NOTHROW(operator==(const BigramIterator &a, const BigramIterator &b));

/// Equality test for TermIterator objects.
inline bool
operator==(const BigramIterator &a, const BigramIterator &b)
{
    // Use a pointer comparison - this ensures both that (a == a) and correct
    // handling of end iterators (which we ensure have NULL internals).
    return a.internal == b.internal;
}

bool
XAPIAN_NOTHROW(operator!=(const BigramIterator &a, const BigramIterator &b));

/// Inequality test for BigramIterator objects.
inline bool
operator!=(const BigramIterator &a, const BigramIterator &b)
{
    return !(a == b);
}

}

#endif // XAPIAN_INCLUDED_BIGRAMITERATOR_H

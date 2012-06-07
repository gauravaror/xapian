/** @file brass_bigramlist.h
 * @brief A BigramList in a brass database.
 */
/* Copyright (C) 2012 Gaurav Arora
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_BRASS_BIGRAMLIST_H
#define XAPIAN_INCLUDED_BRASS_BIGRAMLIST_H

#include <string>

#include "xapian/intrusive_ptr.h"
#include <xapian/positioniterator.h>
#include <xapian/types.h>

namespace Xapian {
    namespace Internal {
	class ExpandStats;
    }
}

#include "brass_database.h"
#include "api/bigramlist.h"
#include "brass_table.h"

/// A BigramList in a brass database.
class BrassBigramList : public BigramList {
    /// Don't allow assignment.
    void operator=(const BrassBigramList &);

    /// Don't allow copying.
    BrassBigramList(const BrassBigramList &);

    /// The database we're reading data from.
    Xapian::Internal::intrusive_ptr<const BrassDatabase> db;

    /// The document id that this BigramList is for.
    Xapian::docid did;

    /// The length of document @a did.
    brass_doclen_t doclen;

    /// The number of entries in this bigramlist.
    Xapian::termcount bigramlist_size;

    /// The tag value from the bigram table which holds the encoded bigram.
    std::string data;

    /** Current position with the encoded tag value held in @a data.
     *
     *  If we've iterated to the end of the list, this gets set to NULL.
     */
    const char *pos;

    /// Pointer to the end of the encoded tag value.
    const char *end;

    /// The bigramname at the current position.
    std::string current_bigram;

    /// The wdf for the bigram at the current position.
    Xapian::termcount current_wdf;

    /** The bigram frequency for the term at the current position.
     *
     *  This will have the value 0 if the term frequency has not yet been
     *  looked up in the database (so it needs to be mutable).
     */
    mutable Xapian::doccount current_termfreq;

  public:
    /// Create a new BrassBigramList object for document @a did_ in DB @a db_
    BrassBigramList(Xapian::Internal::intrusive_ptr<const BrassDatabase> db_,
		  Xapian::docid did_);

    /** Return the length of this document.
     *
     *  This is a non-virtual method, used by BrassDatabase.
     */
    brass_doclen_t get_doclength() const;

    /** Return approximate size of this bigramlist.
     *
     *  For a BrassBigramList, this value will always be exact.
     */
    Xapian::termcount get_approx_size() const;

    /// Collate weighting information for the current bigram.
    void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

    /// Return the bigramname at the current position.
    std::string get_bigramname() const;

    /// Return the wdf for the bigram at the current position.
    Xapian::termcount get_wdf() const;

    /** Return the term frequency for the bigram at the current position.
     *
     *  In order to be able to support updating databases efficiently, we can't
     *  store this value in the bigramlist table, so it has to be read from the
     *  postlist table, which is relatively expensive (compared to reading the
     *  wdf for example).
     */
    Xapian::doccount get_termfreq() const;

    /** Advance the current position to the next bigram in the bigramlist.
     *
     *  The list starts before the first bigram in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  @return Always returns 0 for a BrassBigramList.
     */
    BigramList * next();

    BigramList * skip_to(const std::string & term);

    /// Return true if the current position is past the last bigram in this list.
    bool at_end() const;

};

#endif // XAPIAN_INCLUDED_BRASS_BIGRAMLIST_H

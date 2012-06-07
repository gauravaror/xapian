/** @file bigramdocumentterm.h
 * @brief internal class representing a bigrams in a modified document
 */
/* Copyright 2012 Gaurav Arora
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

#ifndef XAPIAN_INCLUDED_BIGRAMDOCUMENTTERM_H
#define XAPIAN_INCLUDED_BIGRAMDOCUMENTTERM_H

#include "debuglog.h"
#include <iostream>
#include <sstream>

#include <string>
#include <vector>

#include <xapian/types.h>

using namespace std;

/// A term in a document.
class BigramDocumentTerm {
    public:
    /** Make a new bigram.
     *
     *  @param bname_ The name of the new bigram.
     *  @param wdf_   Initial wdf.
     */
    BigramDocumentTerm(const string & bname_, Xapian::termcount wdf_)
	: bname(bname_), wdf(wdf_)
    {
	LOGCALL_CTOR(DB, "BigramDocumentTerm", bname_ | wdf_);
    istringstream iss(bname_);
	iss >> term1;
    }

    /** The name of this bigram.
     */
    string bname;

	/** First term in the bigram
	 * this will be required for setting collocation list
	 */
	string term1;

    /** Within document frequency of the bigram.
     *  This is the number of occurrences of the bigram in the document.
     */
    Xapian::termcount wdf;


    /// Increase the wdf
    void inc_wdf(Xapian::termcount inc) { wdf += inc; }

    /// Decrease the wdf
    void dec_wdf(Xapian::termcount dec) {
	if (wdf <= dec) {
	    wdf = 0;
	} else {
	    wdf -= dec;
	}
    }

    /// Get the wdf
    Xapian::termcount get_wdf() const { return wdf; }

    /// Return a string describing this object.
    string get_description() const;
};

#endif // XAPIAN_INCLUDED_BIGRAMDOCUMENTTERM_H

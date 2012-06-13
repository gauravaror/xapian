/** @file mapbigramlist.h
 * @brief BigramList which iterates a std::map.
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

#ifndef OM_HGUARD_MAPBIGRAMLIST_H
#define OM_HGUARD_MAPBIGRAMLIST_H

#include "bigramlist.h"

#include "backends/document.h"

#include "omassert.h"

using namespace std;

class MapBigramList : public BigramList {
    private:
	Xapian::Document::Internal::document_terms::const_iterator it;
	Xapian::Document::Internal::document_terms::const_iterator it_end;
	bool started;

    public:
	MapBigramList(const Xapian::Document::Internal::document_terms::const_iterator &it_,
		    const Xapian::Document::Internal::document_terms::const_iterator &it_end_)
		: it(it_), it_end(it_end_), started(false)
	{ }

	// Gets size of bigramlist
	Xapian::termcount get_approx_size() const {
	    Assert(false);
	    return 0;
	}

	// Gets current bigramname
	string get_bigramname() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->first;
	}

	// Get wdf of current term
	Xapian::termcount get_wdf() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->second.wdf;
	}

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const {
	    throw Xapian::InvalidOperationError("Can't get term frequency from a document termlist which is not associated with a database.");
	}

	BigramList * next() {
		do
		{
	    if (!started) {
		started = true;
	    } else {
		Assert(!at_end());
		it++;
	    }
		}while(it->first.find(" ") == std::string::npos);
	    return NULL;
	}

	BigramList * skip_to(const std::string & term) {
	    while (it != it_end && it->first < term) {
		++it;
	    }
	    started = true;
	    return NULL;
	}

	// True if we're off the end of the list
	bool at_end() const {
	    Assert(started);
	    return it == it_end;
	}
};

#endif /* OM_HGUARD_MAPBIGRAMLIST_H */

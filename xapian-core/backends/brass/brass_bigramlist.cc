/* brass_bigramlist.cc: BigramList in a brass database
 *
 * copyrights (C) 2012 Gaurav Arora
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

#include <config.h>
#include "brass_bigramlist.h"

#include "xapian/error.h"

#include "expand/expandweight.h"
#include "brass_positionlist.h"
#include "debuglog.h"
#include "omassert.h"
#include "pack.h"
#include "str.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

BrassBigramList::BrassBigramList(intrusive_ptr<const BrassDatabase> db_,
			     Xapian::docid did_)
	: db(db_), did(did_), current_wdf(0), current_termfreq(0)
{
    LOGCALL_CTOR(DB, "BrassBigramList", db_ | did_);

    if (!db->termlist_table.get_exact_entry(BrassTermListTable::make_bigramkey(did),
					    data))
	throw Xapian::DocNotFoundError("No bigramlist for document " + str(did));

    pos = data.data();
    end = pos + data.size();

    if (pos == end) {
	doclen = 0;
	bigramlist_size = 0;
	return;
    }

    // Read doclen
    if (!unpack_uint(&pos, end, &doclen)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for doclen in bigramlist";
	} else {
	    msg = "Overflowed value for doclen in bigramlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    // Read bigramlist_size
    if (!unpack_uint(&pos, end, &bigramlist_size)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for list size in bigramlist";
	} else {
	    msg = "Overflowed value for list size in bigramlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }
}

brass_doclen_t
BrassBigramList::get_doclength() const
{
    LOGCALL(DB, brass_doclen_t, "BrassBigramList::get_doclength", NO_ARGS);
    RETURN(doclen);
}

Xapian::termcount
BrassBigramList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "BrassBigramList::get_approx_size", NO_ARGS);
    RETURN(bigramlist_size);
}

void
BrassBigramList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    LOGCALL_VOID(DB, "BrassBigramList::accumulate_stats", stats);
    Assert(!at_end());
    stats.accumulate(current_wdf, doclen, get_termfreq(), db->get_doccount());
}

string
BrassBigramList::get_bigramname() const
{
    LOGCALL(DB, string, "BrassBigramList::get_bigramname", NO_ARGS);
    RETURN(current_bigram);
}

Xapian::termcount
BrassBigramList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "BrassBigramList::get_wdf", NO_ARGS);
    RETURN(current_wdf);
}

Xapian::doccount
BrassBigramList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "BrassBigramList::get_termfreq", NO_ARGS);
    if (current_termfreq == 0)
	current_termfreq = db->get_termfreq(current_bigram);
    RETURN(current_termfreq);
}

BigramList *
BrassBigramList::next()
{
    LOGCALL(DB, BigramList *, "BrassBigramList::next", NO_ARGS);
    Assert(!at_end());
    if (pos == end) {
	pos = NULL;
	RETURN(NULL);
    }

    // Reset to 0 to indicate that the termfreq needs to be read.
    current_termfreq = 0;

    bool wdf_in_reuse = false;
    if (!current_bigram.empty()) {
	// Find out how much of the previous term to reuse.
	size_t len = static_cast<unsigned char>(*pos++);
	if (len > current_bigram.size()) {
	    // The wdf is also stored in the "reuse" byte.
	    wdf_in_reuse = true;
	    size_t divisor = current_bigram.size() + 1;
	    current_wdf = len / divisor - 1;
	    len %= divisor;
	}
	current_bigram.resize(len);
    }

    // Append the new tail to form the next term.
    size_t append_len = static_cast<unsigned char>(*pos++);
    current_bigram.append(pos, append_len);
    pos += append_len;

    // Read the wdf if it wasn't packed into the reuse byte.
    if (!wdf_in_reuse && !unpack_uint(&pos, end, &current_wdf)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for wdf in termlist";
	} else {
	    msg = "Overflowed value for wdf in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    RETURN(NULL);
}

BigramList *
BrassBigramList::skip_to(const string & bterm)
{
    LOGCALL(API, BigramList *, "BrassBigramList::skip_to", bterm);
    while (pos != NULL && current_bigram < bterm) {
	(void)BrassBigramList::next();
    }
    RETURN(NULL);
}

bool
BrassBigramList::at_end() const
{
    LOGCALL(DB, bool, "BrassBigramList::at_end", NO_ARGS);
    RETURN(pos == NULL);
}

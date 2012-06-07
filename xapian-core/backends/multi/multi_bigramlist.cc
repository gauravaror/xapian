/* multi_termlist.cc: C++ class definition for multiple database access
 *

 * Copyright 2012 Gaurav Arora
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

#include "multi_bigramlist.h"

#include "backends/database.h"
#include "debuglog.h"
#include "expand/expandweight.h"

MultiBigramList::MultiBigramList(BigramList * tl_,
			     const Xapian::Database &db_,
			     size_t db_index_)
	: tl(tl_), db(db_), db_index(db_index_)
{
    termfreq_factor = double(db.get_doccount());
    termfreq_factor /= db.internal[db_index]->get_doccount();
    LOGLINE(DB, "Approximation factor for termfreq: " << termfreq_factor);
}

MultiBigramList::~MultiBigramList()
{
    delete tl;
}

Xapian::termcount
MultiBigramList::get_approx_size() const
{
    return tl->get_approx_size();
}

void
MultiBigramList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    // For a multidb, every termlist access during expand will be through
    // a MultiBigramList, so it's safe to just set db_index like this.
    stats.db_index = db_index;
    tl->accumulate_stats(stats);
}

string
MultiBigramList::get_bigramname() const
{
    return tl->get_bigramname();
}

Xapian::termcount MultiBigramList::get_wdf() const
{
    return tl->get_wdf();
}

Xapian::doccount MultiBigramList::get_termfreq() const
{
    // Approximate term frequency
    return Xapian::doccount(tl->get_termfreq() * termfreq_factor);
}

BigramList * MultiBigramList::next()
{
    return tl->next();
}

BigramList * MultiBigramList::skip_to(const string & term)
{
    return tl->skip_to(term);
}

bool MultiBigramList::at_end() const
{
    return tl->at_end();
}


/** @file multi_termlist.h
 * @brief C++ class declaration for multiple database access
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

#ifndef OM_HGUARD_MULTI_BIGRAMLIST_H
#define OM_HGUARD_MULTI_BIGRAMLIST_H

#include "api/bigramlist.h"
#include "backends/database.h"

class MultiBigramList : public BigramList {
    friend class Xapian::Database;
    private:
	BigramList *tl;
	const Xapian::Database & db;
	size_t db_index;
	double termfreq_factor;

	MultiBigramList(BigramList * tl_,
		      const Xapian::Database &db_,
		      size_t db_index_);
    public:
	Xapian::termcount get_approx_size() const;

	/// Collate weighting information for the current bigram.
	void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

	string get_bigramname() const;
	Xapian::termcount get_wdf() const; // Number of occurrences of term in current doc
	Xapian::doccount get_termfreq() const;  // Number of docs indexed by bigram
	BigramList * next();
	BigramList * skip_to(const std::string & term);
	bool   at_end() const;


	~MultiBigramList();
};

#endif /* OM_HGUARD_MULTI_TERMLIST_H */

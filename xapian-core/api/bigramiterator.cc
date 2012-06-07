/** @file bigramiterator.cc
 *  @brief Class for iterating over a list of bigrams.
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

#include <config.h>

#include "xapian/bigramiterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "bigramlist.h"

using namespace std;

namespace Xapian {

void
BigramIterator::decref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

void
BigramIterator::post_advance(Internal * res)
{
    if (res) {
	// This can happen with iterating allbigrams from multiple databases.
	++res->_refs;
	decref();
	internal = res;
    }
    if (internal->at_end()) {
	decref();
	internal = NULL;
    }
}

BigramIterator::BigramIterator(Internal *internal_) : internal(internal_)
{
    LOGCALL_CTOR(API, "BigramIterator", internal_);
    if (!internal) return;
    try {
	++internal->_refs;
	post_advance(internal->next());
    } catch (...) {
	// The destructor only runs if the constructor completes, so we have to
	// take care of cleaning up for ourselves here.
	decref();
	throw;
    }
}

BigramIterator::BigramIterator(const BigramIterator & o)
    : internal(o.internal)
{
    LOGCALL_CTOR(API, "BigramIterator", o);
    if (internal)
	++internal->_refs;
}

BigramIterator &
BigramIterator::operator=(const BigramIterator & o)
{
    LOGCALL(API, BigramIterator &, "BigramIterator::operator=", o);
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	decref();
    internal = o.internal;
    RETURN(*this);
}

string
BigramIterator::operator*() const
{
    LOGCALL(API, string, "BigramIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_bigramname());
}

BigramIterator &
BigramIterator::operator++()
{
    LOGCALL(API, BigramIterator &, "BigramIterator::operator++", NO_ARGS);
    Assert(internal);
    post_advance(internal->next());
    RETURN(*this);
}

Xapian::termcount
BigramIterator::get_wdf() const
{
    LOGCALL(API, Xapian::termcount, "BigramIterator::get_wdf", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_wdf());
}

Xapian::doccount
BigramIterator::get_termfreq() const
{
    LOGCALL(API, Xapian::doccount, "BigramIterator::get_termfreq", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_termfreq());
}

void
BigramIterator::skip_to(const string & bterm)
{
    LOGCALL_VOID(API, "BigramIterator::skip_to", bterm);
    Assert(internal);
    post_advance(internal->skip_to(bterm));
}

std::string
BigramIterator::get_description() const
{
#if 0 // FIXME: Add BigramIterator::Internal::get_description() method.
    string desc = "BigramIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
#else
    return "BigramIterator()";
#endif
}

}

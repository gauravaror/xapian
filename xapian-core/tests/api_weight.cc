/** @file api_weight.cc
 * @brief tests of Xapian::Weight subclasses
 */
/* Copyright (C) 2012 Olly Betts
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

#include "api_weight.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(tradweight3, !backend) {
    Xapian::TradWeight wt(42);
    try {
	Xapian::TradWeight t;
	Xapian::TradWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised TradWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised TradWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	// Regression test for error in exception message fixed in 1.2.11 and
	// 1.3.1.
	TEST(e.get_msg().find("BM25") == string::npos);
    }
    return true;
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(bm25weight3, !backend) {
    Xapian::BM25Weight wt(2.0, 0.5, 1.3, 0.6, 0.01);
    try {
	Xapian::BM25Weight b;
	Xapian::BM25Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised BM25Weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised BM25Weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &) {
	// Good!
    }
    return true;
}

// Test parameter combinations which should be unaffected by doclength.
DEFINE_TESTCASE(bm25weight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::BM25Weight(1, 0, 1, 0, 0.5));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: wdf has an effect on weight, but doclen doesn't.
    TEST_REL(mset[0].get_weight(),>,mset[1].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), mset[2].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());
    TEST_EQUAL_DOUBLE(mset[3].get_weight(), mset[4].get_weight());

    enquire.set_weighting_scheme(Xapian::BM25Weight(0, 0, 1, 1, 0.5));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: neither wdf nor doclen affects weight.
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), mset[4].get_weight());

    return true;
}

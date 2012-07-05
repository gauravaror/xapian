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

//Test Exception for junk after serialised weight.
DEFINE_TESTCASE(unigramlmweight3, !backend) {
    Xapian::LMWeight wt(79898.0,Xapian::Weight::JELINEK_MERCER_SMOOTHING,0.5,1.0);
    try {
	Xapian::LMWeight t;
	Xapian::LMWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised LMWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised LMWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	//GOOD!
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
//Two stage should perform same as Jelinek mercer if smoothing parameter for mercer is kept 1 in both.
DEFINE_TESTCASE(unigramlmweight4,backend) {
	Xapian::Database db = get_database("apitest_simpledata");
	Xapian::Enquire enquire1(db);
	Xapian::Enquire enquire2(db);
	enquire1.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset1;
	enquire2.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset2;
	//5 documents avaialble with term paragraph so mset size should be 5
	enquire1.set_weighting_scheme(Xapian::LMWeight(Xapian::Weight::TWO_STAGE_SMOOTHING,1,0));
	enquire2.set_weighting_scheme(Xapian::LMWeight(Xapian::Weight::JELINEK_MERCER_SMOOTHING,1,0));
	mset1 = enquire1.get_mset(0,10);
	mset2 = enquire2.get_mset(0,10);

	TEST_EQUAL(mset1.size(), 5);	
	TEST_EQUAL_DOUBLE(mset1[1].get_weight(),mset2[1].get_weight());
	return true;
}

/*Test for checking if we dont use smoothing all
* of them should give same result i.e wdf_double/len_double */
DEFINE_TESTCASE(unigramlmweight5,backend) {
	Xapian::Database db = get_database("apitest_simpledata");
	Xapian::Enquire enquire1(db);
	Xapian::Enquire enquire2(db);
	Xapian::Enquire enquire3(db);
	Xapian::Enquire enquire4(db);
	enquire1.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset1;
	enquire2.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset2;
	enquire3.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset3;
	enquire4.set_query(Xapian::Query("paragraph"));
	Xapian::MSet mset4;
	//5 documents avaialble with term paragraph so mset size should be 5
	enquire1.set_weighting_scheme(Xapian::LMWeight(10000.0,Xapian::Weight::TWO_STAGE_SMOOTHING,0,0));
	enquire2.set_weighting_scheme(Xapian::LMWeight(10000.0,Xapian::Weight::JELINEK_MERCER_SMOOTHING,0,0));
	enquire3.set_weighting_scheme(Xapian::LMWeight(10000.0,Xapian::Weight::ABSOLUTE_DISCOUNT_SMOOTHING,0,0));
	enquire4.set_weighting_scheme(Xapian::LMWeight(10000.0,Xapian::Weight::DIRICHLET_SMOOTHING,0,0));

	mset1 = enquire1.get_mset(0,10);
	mset2 = enquire2.get_mset(0,10);
	mset3 = enquire3.get_mset(0,10);
	mset4 = enquire4.get_mset(0,10);

	TEST_EQUAL(mset1.size(), 5);	
	TEST_EQUAL(mset2.size(), 5);	
	TEST_EQUAL(mset3.size(), 5);	
	TEST_EQUAL(mset4.size(), 5);	
	for(size_t i = 1;i < 5;i++) {
	TEST_EQUAL_DOUBLE(mset3[i].get_weight(),mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset2[i].get_weight(),mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(),mset2[i].get_weight());
	TEST_EQUAL_DOUBLE(mset3[i].get_weight(),mset2[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(),mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(),mset3[i].get_weight());
}
	return true;
}

class CheckInitWeight : public Xapian::Weight {
  public:
    double factor;

    unsigned & zero_inits, & non_zero_inits;

    CheckInitWeight(unsigned &z, unsigned &n)
	: factor(-1.0), zero_inits(z), non_zero_inits(n) { }

    void init(double factor_) {
	factor = factor_;
	if (factor == 0.0)
	    ++zero_inits;
	else
	    ++non_zero_inits;
    }

    Weight * clone() const {
	return new CheckInitWeight(zero_inits, non_zero_inits);
    }

    double get_sumpart(Xapian::termcount, Xapian::termcount) const {
	return 1.0;
    }

    double get_maxpart() const { return 1.0; }

    double get_sumextra(Xapian::termcount doclen) const {
	return 1.0 / doclen;
    }

    double get_maxextra() const { return 1.0; }
};

/// Regression test - check init() is called for the term-indep Weight obj.
DEFINE_TESTCASE(checkinitweight1, backend && !multi && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query q(Xapian::Query::OP_AND,
		    Xapian::Query("this"), Xapian::Query("paragraph"));
    enquire.set_query(q);
    unsigned zero_inits = 0, non_zero_inits = 0;
    CheckInitWeight wt(zero_inits, non_zero_inits);
    enquire.set_weighting_scheme(wt);
    Xapian::MSet mset = enquire.get_mset(0, 3);
    TEST_EQUAL(zero_inits, 1);
    TEST_EQUAL(non_zero_inits, 2);
    return true;
}

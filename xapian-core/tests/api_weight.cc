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
#include <stdio.h>
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
	
	double get_sumpart(Xapian::termcount,Xapian::termcount,Xapian::termcount) const {
	return 1.0;
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

class CheckStatsWeight : public Xapian::Weight {
  public:
    double factor;

    Xapian::Database db;

    string term;

    Xapian::termcount & sum;
    Xapian::termcount & sum_squares;

    mutable Xapian::termcount len_upper;
    mutable Xapian::termcount len_lower;
    mutable Xapian::termcount wdf_upper;

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: factor(-1.0), db(db_), term(term_),
	  sum(sum_), sum_squares(sum_squares_),
	  len_upper(0), len_lower(Xapian::termcount(-1)), wdf_upper(0)
    {
	need_stat(COLLECTION_SIZE);
	need_stat(RSET_SIZE);
	need_stat(AVERAGE_LENGTH);
	need_stat(TERMFREQ);
	need_stat(RELTERMFREQ);
	need_stat(QUERY_LENGTH);
	need_stat(WQF);
	need_stat(WDF);
	need_stat(DOC_LENGTH);
	need_stat(DOC_LENGTH_MIN);
	need_stat(DOC_LENGTH_MAX);
	need_stat(WDF_MAX);
    }

    void init(double factor_) {
	factor = factor_;
    }

    Weight * clone() const {
	return new CheckStatsWeight(db, term, sum, sum_squares);
    }

    double get_sumpart(Xapian::termcount wdf, Xapian::termcount doclen) const {
	TEST_EQUAL(get_collection_size(), db.get_doccount());
	TEST_EQUAL(get_rset_size(), 0);
	TEST_EQUAL(get_average_length(), db.get_avlength());
	TEST_EQUAL(get_termfreq(), db.get_termfreq(term));
	TEST_EQUAL(get_reltermfreq(), 0);
	TEST_EQUAL(get_query_length(), 1);
	TEST_EQUAL(get_wqf(), 1);
	TEST_REL(doclen,>=,len_lower);
	TEST_REL(doclen,<=,len_upper);
	TEST_REL(wdf,<=,wdf_upper);
	sum += wdf;
	sum_squares += wdf * wdf;
	return 1.0;
    }

    double get_maxpart() const {
	if (len_upper == 0) {
	    len_lower = get_doclength_lower_bound();
	    len_upper = get_doclength_upper_bound();
	    wdf_upper = get_wdf_upper_bound();
	}
	return 1.0;
    }

    double get_sumextra(Xapian::termcount doclen) const {
	return 1.0 / doclen;
    }

    double get_maxextra() const { return 1.0; }
};

/// Check the weight subclass gets the correct stats.
DEFINE_TESTCASE(checkstatsweight1, backend && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::TermIterator a;
    for (a = db.allterms_begin(); a != db.allterms_end(); ++a) {
	const string & term = *a;
	enquire.set_query(Xapian::Query(term));
	Xapian::termcount sum = 0;
	Xapian::termcount sum_squares = 0;
	CheckStatsWeight wt(db, term, sum, sum_squares);
	enquire.set_weighting_scheme(wt);
	Xapian::MSet mset = enquire.get_mset(0, db.get_doccount());

	// The document order in the multi-db case isn't the same as the
	// postlist order on the combined DB, so it's hard to compare the
	// wdf for each document in the Weight objects, so we can sum
	// the wdfs and the squares of the wdfs which provides a decent
	// check that we're not getting the wrong wdf values (it ensures
	// they have the right mean and standard deviation).
	Xapian::termcount expected_sum = 0;
	Xapian::termcount expected_sum_squares = 0;
	Xapian::PostingIterator i;
	for (i = db.postlist_begin(term); i != db.postlist_end(term); ++i) {
	    Xapian::termcount wdf = i.get_wdf();
	    expected_sum += wdf;
	    expected_sum_squares += wdf * wdf;
	}
	TEST_EQUAL(sum, expected_sum);
	TEST_EQUAL(sum_squares, expected_sum_squares);
    }
    return true;
}

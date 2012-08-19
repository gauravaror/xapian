/** @file api_weight.cc
 * @brief tests of Xapian::Weight subclasses
 */
/* Copyright (C) 2012 Olly Betts
 * Copyright (C) 2012 Gaurav Arora
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
#include "api_bigram.h"
#include <stdio.h>
#include <xapian.h>
#include <iostream>
#include <fstream>
#include "apitest.h"
#include "testutils.h"
#include "safesysstat.h"
using namespace std;

/// Read a paragraph from stream @a input.
static string
get_paragraph(istream &input)
{
    string para, line;
    while (true) {
	getline(input, line);
	if (!input.good())
	    return para;
	para += line;
	para += '\n';
    }
}

DEFINE_TESTCASE(bigramtest1, backend) {
    mkdir(".brass", 0755);
    string dbdir = ".brass/bigramtest";
    Xapian::WritableDatabase db(Xapian::Brass::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE));
    Xapian::TermGenerator indexer;
    Xapian::Document doc;
    indexer.set_document(doc);	
	indexer.set_database(db);
	Xapian::Stem stemmer("english");
	indexer.set_stemmer(stemmer);
	indexer.set_bigrams(true);
    filebuf fb;
	fb.open ("testdata/apitest_simpledata.txt",ios::in);
    istream bigramfile(&fb);
	string text =  get_paragraph(bigramfile);
	doc.set_data(text);
	indexer.index_text(text);
	db.add_document(doc);
	Xapian::QueryParser qp;
	qp.set_stemmer(stemmer);
	qp.set_bigram(true);
	string querystring = "mention  banana";
	Xapian::Query query = qp.parse_query(querystring);
	Xapian::Enquire enquire(db);
	enquire.set_query(query);
	enquire.set_weighting_scheme(Xapian::LMWeight(0.0,Xapian::Weight::TWO_STAGE_SMOOTHING,0.8,2000,1.0));
	Xapian::MSet matches = enquire.get_mset(0,10);
	Xapian::QueryParser qp1;
	qp1.set_stemmer(stemmer);
	Xapian::Query query1 = qp1.parse_query(querystring);
	Xapian::Enquire enquire1(db);
	enquire1.set_query(query1);
	enquire1.set_weighting_scheme(Xapian::LMWeight(0.0,Xapian::Weight::TWO_STAGE_SMOOTHING,0.8,2000,1.0));
	Xapian::MSet matches1 =	enquire1.get_mset(0,10);
	TEST_EQUAL(matches.size(),matches1.size());
	return true;
}

DEFINE_TESTCASE(bigramtest2, backend) {
	SKIP_TEST_FOR_BACKEND("chert");
	SKIP_TEST_FOR_BACKEND("inmemory");
    mkdir(".brass", 0755);
    string dbdir = ".brass/bigramtest2";
    Xapian::WritableDatabase db(Xapian::Brass::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE));
    Xapian::TermGenerator indexer;
    Xapian::Document doc;
    filebuf fb;
	fb.open ("testdata/apitest_simpledata.txt",ios::in);
    istream bigramfile(&fb);
	string text =  get_paragraph(bigramfile);
	doc.set_data(text);
	Xapian::Stem stemmer("english");
	indexer.set_stemmer(stemmer);
	indexer.set_bigrams(true);
    indexer.set_document(doc);	
	indexer.set_database(db);
	indexer.index_text(text);
	Xapian::docid docid1 = db.add_document(doc);
	Xapian::TermIterator bi = db.termlist_begin(docid1);
	tout<<"jdk"<<docid1;
	while(bi != db.termlist_end(docid1)) {
	string bigramterm = *bi;
	tout<<bigramterm<<endl;
	if( bigramterm.compare("mention banana") == 0) {
		return true;
	}
	bi++;
	}
	return false;
}

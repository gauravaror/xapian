/** @file unigramlmweight.cc
 * @brief Xapian::UnigramLMWeight class - the Unigram Language Modelling formula.
 */
/* Copyright (C) 2012 Gaurav Arora
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

#include <cmath>

using namespace std;

namespace Xapian {

UnigramLMWeight *
UnigramLMWeight::clone() const
{
    return new UnigramLMWeight();
}

void
UnigramLMWeight::init(double )
{
    //Storing collection frequency of current term in collection_freq to be accessed while smoothing of weights for the term,for term not present in the document.
    collection_freq = get_collectionfreq();
    //Collection_freq of a term in collection should be always greater than or equal to zero(Non Negative).
    AssertRel(collection_freq,>=,0);
    LOGVALUE(WTCALC, collection_freq);
    // calculating approximate number of total terms in the collection to be accessed for smoothining of the document.
    total_collection_term =  get_collection_size()*get_average_length();
    // Total term should be greater than zero as there would be atleast one document in collection.
    AssertRel(total_collection_terms,>,0);
    LOGVALUE(WTCALC, total_collection_terms);
    // There can't be more relevant term in collection than total number of term
    AssertRel(collection_freq,<=,total_collection_terms);

}

string
UnigramLMWeight::name() const
{
    return "Xapian::UnigramLMWeight";
}

string
UnigramLMWeight::serialise() const
{
    // No parameters to serialise.
    return string();
}

UnigramLMWeight *
UnigramLMWeight::unserialise(const string & ) const
{
    return new UnigramLMWeight();
}

double
UnigramLMWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    //Withing Document Frequency of the term in document being considered.
    double wdf_double(wdf);
    //Length of the Document in terms of number of terms.
    double len_double(len);
    // varioable to store weight contribution of term in the document socring for unigram LM.
    double weight_sum;
    if (wdf_double == 0) {
	/* In case the within document frequency og term is zero smoothining will be required and should be return instead of returnin         * g zero,as returning LM score are multiplication of contribution of all terms,due to absence of single term whole document i         * s scored zero,hence apply collection frequency smoothining*/
      weight_sum = collection_freq / total_collection_term;
    }
    else {
	/*Maximum likelihood of current term ,weight contribution of term incase query term is present in the document.*/
      weight_sum = wdf_double / len_double;
    }
    /* Since unigram LM score is calculated with multiplication,instead of changing the current implementation log trick have been use     * d to calculate the product since (sum of log is log of product and since aim is ranking ranking document by product or log of p     * roduct wont make large diffrence hence log(product) will be used for ranking */

    /* FIXME: currently 10 is being added in the log , for calculating the score as weight_sum id a probability and log(number<1) will     * be negative so due to negative score matcher discards documents and return no matching document hence to avoid this linear weig     *  ht is added which need to be fixed */

    return log(weight_sum+10);
}

double
UnigramLMWeight::get_maxpart() const
{
 // Sufficiently large bound is being returned ,to optimize the matching process this needs to be fixed and changed to good max bound 
   double wdf_max(max(get_wdf_upper_bound(), Xapian::termcount(1)));
    return  (wdf_max);
}

double
UnigramLMWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
UnigramLMWeight::get_maxextra() const
{
    return 0;
}

}

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
UnigramLMWeight::clone() const  {
	return new UnigramLMWeight(param_log,select_smoothing, param_smoothing1, param_smoothing2);
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

	/*  * Setting default values of the param_log to handel negetive value of log.
		* It is considered to be upperbound of document length.
	    * intializing param_log to upperbound of document_length.*/

	if(param_log == 0.0) {
	param_log = get_doclength_upper_bound();
	}

	/*  *  since the optimal parameter for  Jelinek  mercer smoothing 
		* is based on query  length, so if query is title query changing 
	    * default value of smoothig parameter. */

	if(select_smoothing == JELINEK_MERCER_SMOOTHING||select_smoothing == TWO_STAGE_SMOOTHING) {
		if(param_smoothing1 == 0.7) {
			if(get_query_length() <= 2) {
			param_smoothing1 = 0.1;
			}
		}
	}

 /* *  param_smoothing1 default value should be 2000 in case DIRICHLET_SMOOTHING is selected.Tweaking param_smooothing1
	*  if user supply his own value for param_smoothing1 value will not be set to 2000(default value) */

	if(select_smoothing == DIRICHLET_SMOOTHING) {
		if(param_smoothing1 == 0.7) {
		param_smoothing1 = 2000;
		}
	}
}
string
UnigramLMWeight::name() const
{
    return "Xapian::UnigramLMWeight";
}

string
UnigramLMWeight::serialise() const
{
    
    string result = serialise_double(param_log);
	result += static_cast<unsigned char>(select_smoothing);
	result += serialise_double(param_smoothing1);
	result += serialise_double(param_smoothing2);

    return result;
}

UnigramLMWeight *
UnigramLMWeight::unserialise(const string & s) const
{
	const char *ptr =  s.data();
	const char *end = ptr + s.size();
	double param_log_ = unserialise_double(&ptr,end);
	type_smoothing select_smoothing_ = static_cast<type_smoothing>(*(ptr)++);
	double param_smoothing1_ = unserialise_double(&ptr,end);
	double param_smoothing2_ = unserialise_double(&ptr,end);
	if(rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in UnigramLMWeight::unserialise()");
	return new UnigramLMWeight(param_log_,select_smoothing_,param_smoothing1_,param_smoothing2_);
}

double
UnigramLMWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
	return	get_sumpart(wdf,len,Xapian::termcount(1));
}

double
UnigramLMWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,Xapian::termcount uniqterm) const
{
    //Withing Document Frequency of the term in document being considered.
    double wdf_double(wdf);
    //Length of the Document in terms of number of terms.
    double len_double(len);
	double nouniqterm_double(uniqterm);
    // varioable to store weight contribution of term in the document socring for unigram LM.
    double weight_collection,weight_document,weight_sum;
	/* In case the within document frequency of term is zero smoothining
	*  will be required and should be return instead of returning zero,
	*  as returning LM score are multiplication of contribution of all terms,due to absence of single term
	*  whole document is scored zero,hence apply collection frequency smoothining*/
    weight_collection = collection_freq / total_collection_term;
	/*Maximum likelihood of current term ,weight contribution of term incase query term is present in the document.*/
    weight_document = wdf_double / len_double;
	
	// Calculating weiights considering diffrent smoothing option available to user.
	if(select_smoothing == JELINEK_MERCER_SMOOTHING) {
		weight_sum = ((param_smoothing1*weight_collection) +  ((1-param_smoothing1)*weight_document));
	}
	else if(select_smoothing == DIRICHLET_SMOOTHING) {
		weight_sum = (wdf_double + (param_smoothing1*weight_collection))/(len_double + param_smoothing1);
	}
	else if(select_smoothing ==  ABSOLUTE_DISCOUNT_SMOOTHING) {
		weight_sum = ((((wdf_double - param_smoothing1) > 0) ? (wdf_double - param_smoothing1) : 0) / len_double) + ((param_smoothing1 *weight_collection*(nouniqterm_double))/len_double);
	}
	else {
		weight_sum = (((1-param_smoothing1)*(wdf_double + (param_smoothing2*weight_collection))/(len_double + param_smoothing2)) + (param_smoothing1*weight_collection));
	}


    /* Since unigram LM score is calculated with multiplication,
	* instead of changing the current implementation log trick have been used
	* to calculate the product since (sum of log is log of product and 
	* since aim is ranking ranking document by product or log of 
	* product wont make large diffrence hence log(product) will be used for ranking */
	//weight_sum = weight_sum +1;
	return (log((weight_sum)*param_log) > 0) ? log((weight_sum)*param_log) : 0;
	//return weight_sum;
}

double
UnigramLMWeight::get_maxpart() const
{
 // Sufficiently large bound is being returned ,to optimize the matching process this needs to be fixed and changed to good max bound
// Need to be fixed 
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

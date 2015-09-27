#include <boost/interprocess/containers/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

struct BuyValues
{
    float value1;
    int option;
    int quantity;
	std::string user; 
	boost::posix_time::ptime timeBuy;
	boost::posix_time::ptime timeExe;
    
	BuyValues ( 
				float value1_, 
			    int option_, 
				int quantity_, 
				std::string user_,
				boost::posix_time::ptime timeBuy_,
				boost::posix_time::ptime timeExe_
			  ): 
				value1(value1_), 
				option(option_), 
				quantity(quantity_),
				user(user_),
				timeBuy(timeBuy_),
				timeExe(timeExe_) 
				{}
};

struct index_by_option_time_user;
struct value1;
struct quantity;
struct user;
struct timeBuy;
struct timeExe;
struct option;


typedef boost::multi_index::multi_index_container<BuyValues, 
	boost::multi_index::indexed_by<
    	boost::multi_index::ordered_unique<boost::multi_index::tag<index_by_option_time_user>,
      		boost::multi_index::composite_key<
        		BuyValues,
        		boost::multi_index::member<BuyValues, int,                      &BuyValues::option>,
				boost::multi_index::member<BuyValues, boost::posix_time::ptime, &BuyValues::timeBuy>,
				boost::multi_index::member<BuyValues, std::string,              &BuyValues::user>
      		>
    	>
	>
> BuyValues_set;
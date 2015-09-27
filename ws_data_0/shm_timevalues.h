#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost::interprocess;

struct TimeValues
{
    float value1;
    int id_ba;
    boost::posix_time::ptime time;
    TimeValues( float value1_, int id_ba_, boost::posix_time::ptime time_): value1(value1_), id_ba(id_ba_), time(time_) {}
};


struct index_by_t_id_ba;
struct value1;
struct time;
struct id_ba;

typedef boost::multi_index::multi_index_container<TimeValues, boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<  boost::multi_index::tag<index_by_t_id_ba>,
        boost::multi_index::composite_key<
            TimeValues,
            boost::multi_index::member<TimeValues, boost::posix_time::ptime, &TimeValues::time>,
            boost::multi_index::member<TimeValues, int,                      &TimeValues::id_ba>
        >
    >
>, managed_shared_memory::allocator<TimeValues>::type> TimeValues_set;
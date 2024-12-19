#pragma once
#include <set>
#include "boost/date_time/local_time/local_time.hpp"

auto parseSlots(std::string_view html) -> std::set<boost::posix_time::ptime>;
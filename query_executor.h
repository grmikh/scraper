#pragma once
#include <set>
#include "boost/date_time/local_time/local_time.hpp"

std::set<std::pair<std::string, std::string>> getSlotsFiltered(const std::set<std::string>& venues);
auto parseSlots(std::string_view json) -> std::set<boost::posix_time::ptime>;



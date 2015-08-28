/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2015 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "icinga/timeperiod.hpp"
#include "icinga/timeperiod.tcpp"
#include "icinga/legacytimeperiod.hpp"
#include "base/configtype.hpp"
#include "base/objectlock.hpp"
#include "base/exception.hpp"
#include "base/logger.hpp"
#include "base/timer.hpp"
#include "base/utility.hpp"
#include <boost/foreach.hpp>

using namespace icinga;

REGISTER_TYPE(TimePeriod);

bool TimePeriod::GetIsInside(void) const
{
	return IsInside(Utility::GetTime());
}

bool TimePeriod::IsInside(double ts) const
{
	std::vector<Value> arguments;
	arguments.push_back(const_cast<TimePeriod *>(this));
	arguments.push_back(ts);

	return GetCheckTime()->Invoke(arguments);
}

void TimePeriod::ValidateRanges(const Dictionary::Ptr& value, const ValidationUtils& utils)
{
	if (!value)
		return;

	/* create a fake time environment to validate the definitions */
	time_t refts = Utility::GetTime();
	tm reference = Utility::LocalTime(refts);
	Array::Ptr segments = new Array();

	ObjectLock olock(value);
	BOOST_FOREACH(const Dictionary::Pair& kv, value) {
		try {
			tm begin_tm, end_tm;
			int stride;
			LegacyTimePeriod::ParseTimeRange(kv.first, &begin_tm, &end_tm, &stride, &reference);
		} catch (const std::exception& ex) {
			BOOST_THROW_EXCEPTION(ValidationError(this, boost::assign::list_of("ranges"), "Invalid time specification '" + kv.first + "': " + ex.what()));
		}

		try {
			LegacyTimePeriod::ProcessTimeRanges(kv.second, &reference, segments);
		} catch (const std::exception& ex) {
			BOOST_THROW_EXCEPTION(ValidationError(this, boost::assign::list_of("ranges"), "Invalid time range definition '" + kv.second + "': " + ex.what()));
		}
	}
}

/**************************************************************************
*   Copyright (C) 2004-2007 by Michael Medin <michael@medin.name>         *
*                                                                         *
*   This code is part of NSClient++ - http://trac.nakednuns.org/nscp      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <strEx.h>
#include <checkHelpers.hpp>

#include <protobuf/plugin.pb.h>

class CheckDisk : public nscapi::impl::simple_plugin  {
private:
	bool show_errors_;
	typedef checkHolders::CheckContainer<checkHolders::MaxMinBoundsDiscSize> PathContainer;
	typedef checkHolders::MagicCheckContainer<checkHolders::MaxMinPercentageBoundsDiskSize> DriveContainer;

public:
	CheckDisk();

	std::wstring get_filter(unsigned int drvType);

	// Check commands
	NSCAPI::nagiosReturn check_filesize(const std::string &target, const std::string &command, std::list<std::string> &arguments, std::string &msg, std::string &perf);
	NSCAPI::nagiosReturn check_files(const std::string &target, const std::string &command, std::list<std::string> &arguments, std::string &msg, std::string &perf);
	void check_drivesize(const Plugin::QueryRequestMessage::Request &request, Plugin::QueryResponseMessage::Response *response);

};

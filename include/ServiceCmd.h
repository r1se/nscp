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
#pragma once

#include <sstream>
#include <Windows.h>
#include <unicode_char.hpp>

namespace serviceControll {
	class SCException {
	public:
		std::string error_;
		SCException(std::string error) : error_(error) {
		}
		SCException(std::string error, int code) : error_(error) {
			std::stringstream ss;
			ss << ": ";
			ss << code;
			error += ss.str();
		}
	};
	void Install(std::wstring,std::wstring,std::wstring,DWORD=SERVICE_WIN32_OWN_PROCESS, std::wstring args = _T(""), std::wstring exe=_T(""));
	void ModifyServiceType(LPCTSTR szName, DWORD dwServiceType);
	void Uninstall(std::wstring);
	void Start(std::wstring);
	bool isStarted(std::wstring);
	bool isInstalled(std::wstring name);
	void Stop(std::wstring);
	void StopNoWait(std::wstring);
	void SetDescription(std::wstring,std::wstring);
	DWORD GetServiceType(LPCTSTR szName);
	std::wstring get_exe_path(std::wstring svc_name);

}
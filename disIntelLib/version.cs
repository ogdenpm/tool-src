﻿/****************************************************************************
 *                                                                          *
 *  disIntelLib: Disassemble Intel OMF85 library                            *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  This program is free software; you can redistribute it and/or           *
 *  modify it under the terms of the GNU General Public License             *
 *  as published by the Free Software Foundation; either version 2          *
 *  of the License, or (at your option) any later version.                  *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program; if not, write to the Free Software             *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,              *
 *  MA  02110-1301, USA.                                                    *
 *                                                                          *
 ****************************************************************************/

using System;

namespace GitVersionInfo
{
    public partial class VersionInfo
    {
        public static void showVersion(bool full)
        {
            #if DEBUG
            string debug = " {debug}";
            #else
            string debug = "";
#endif
            Console.WriteLine($"{GIT_APPNAME} - {GIT_VERSION}{debug} (C){GIT_YEAR} Mark Ogden");

            if (full)
            {
                string sha1 = GIT_SHA1 + (GIT_BUILDTYPE == 1 ? "+" : "");
                Console.WriteLine($"Program: {GIT_APPNAME} {sha1} [{GIT_CTIME.Substring(0, 10)}]");
            }
        }
    }
}

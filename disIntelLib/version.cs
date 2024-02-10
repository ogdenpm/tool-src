/****************************************************************************
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

using System.Diagnostics;
using System.Reflection;
using System;
using System.Security.Principal;



namespace GitVersionInfo
{
    public partial class VersionInfo
    {
        public static void showVersion(bool full)
        {
#if DEBUG
            string build = "Debug build";
#else
            string build = "Build";
#endif
            Assembly assembly = Assembly.GetExecutingAssembly();
            FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(assembly.Location);


            Console.WriteLine($"{fvi.ProductName} (C) {fvi.LegalCopyright} {fvi.ProductVersion}");
            if (full)
            {            
                Console.WriteLine(assembly.GetCustomAttribute<AssemblyDescriptionAttribute>().Description);
                Console.WriteLine($"{build}: {GIT_CHKDATE}");
                Console.WriteLine("Support email: support@mark-ogden.uk");
            }
        }
    }
}
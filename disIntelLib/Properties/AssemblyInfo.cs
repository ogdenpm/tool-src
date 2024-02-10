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

using GitVersionInfo;
using System.Reflection;
using System.Runtime.InteropServices;



// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("disIntelLib")]
[assembly: AssemblyDescription("Disassemble Intel OMF85 library files")]
#if DEBUG
[assembly: AssemblyConfiguration("debug")]
#else
[assembly: AssemblyConfiguration("")]
#endif
[assembly: AssemblyCompany("Mark Ogden")]
[assembly: AssemblyProduct("disIntelLib")]
[assembly: AssemblyCopyright("Mark Ogden")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

[assembly: AssemblyInformationalVersion(VersionInfo.GIT_VERSION)]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]


[assembly: Guid("4032347d-438d-4a3f-8d8b-fe8d9b39f5cc")]

[assembly: AssemblyVersion("1.0.*")]
[assembly: AssemblyFileVersion("0.0.0.0")]

/****************************************************************************
 *  disIntelLib: Disassemble Intel OMF85 library                            *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  Permission is hereby granted, free of charge, to any person             *
 *  obtaining a copy of this software and associated documentation          *
 *  files (the "Software"), to deal in the Software without restriction,    *
 *  including without limitation the rights to use, copy, modify, merge,    *
 *  publish, distribute, sublicense, and/or sell copies of the Software,    *
 *  and to permit persons to whom the Software is furnished to do so,       *
 *  subject to the following conditions:                                    *
 *                                                                          *
 *  The above copyright notice and this permission notice shall be          *
 *  included in all copies or substantial portions of the Software.         *
 *                                                                          *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,         *
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF      *
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  *
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    *
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    *
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       *
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  *
 *                                                                          *
 ****************************************************************************/


using GitVersionInfo;
using System.Reflection;
using System.Runtime.InteropServices;



// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle(VersionInfo.GIT_APPNAME)]
[assembly: AssemblyDescription("Disassemble Intel OMF85 library")]
#if DEBUG
[assembly: AssemblyConfiguration("debug")]
#else
[assembly: AssemblyConfiguration("")]
#endif
[assembly: AssemblyCompany("Mark Ogden")]
[assembly: AssemblyProduct(VersionInfo.GIT_APPNAME)]
[assembly: AssemblyCopyright("(C)" + VersionInfo.GIT_YEAR + " Mark Ogden")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

[assembly: AssemblyInformationalVersion(VersionInfo.GIT_VERSION)]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]


[assembly: Guid("4032347d-438d-4a3f-8d8b-fe8d9b39f5cc")]

[assembly: AssemblyVersion(VersionInfo.GIT_VERSION_RC)]
[assembly: AssemblyFileVersion(VersionInfo.GIT_VERSION_RC)]

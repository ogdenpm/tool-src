/****************************************************************************
 *                                                                          *
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


using System;

namespace GitVersionInfo
{
    public partial class VersionInfo
    {
        public static void showVersion(bool full)
        {
            Console.Write($"{GIT_APPNAME} {GIT_VERSION}");
#if DEBUG
                Console.Write(" {debug}");
#endif
            Console.WriteLine($"  (C){GIT_YEAR} Mark Ogden");
            if (full)
            {
                Console.Write($"Git: {GIT_SHA1} [{GIT_CTIME.Substring(0, 10)}]");
#pragma warning disable CS0162
                if (GIT_BUILDTYPE == 2)
                    Console.WriteLine($" +uncommitted files");
                else if (GIT_BUILDTYPE == 3)
                    Console.WriteLine($" +untracked files");
                else
                    Console.WriteLine("");
#pragma warning restore CS0162
            }
        }
    }
}

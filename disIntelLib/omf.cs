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


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
namespace disIntelLib
{
    class Omf
    {
        Byte[] bytes;
        int start;
        int next;
        int cur;
        int type;

        public Omf(String path)
        {
            bytes = File.ReadAllBytes(path);
        }

        public String iname()
        {
            if (cur >= next - 1)
                return null;
            int len = bytes[cur++];
            if (cur + len >= next - 1)
                return null;
            char[] chars = new char[len];
            for (int i = 0; i < len; i++)
                chars[i] = (char)bytes[cur++];
            return new string(chars);
        }

        public int ibyte()
        {
            if (cur >= next - 1)
                return -1;
            return bytes[cur++];
        }

        public int peekIbyte()
        {
            if (cur >= next - 1)
                return -1;
            return bytes[cur];

        }
        public int iword()
        {
            if (cur + 1 >= next - 1)
                return -1;
            cur += 2;
            return  bytes[cur - 1] * 256 + bytes[cur - 2];
        }

        public int getLen() => next - start - 3;

        public int nextRec()
        {
            if (next + 3 > bytes.Length)
                return -1;
            cur = start = next;
            type = bytes[cur++];
            cur += 2;
            next = cur + bytes[cur - 1] * 256 + bytes[cur - 2];
            if (next <= bytes.Length)
                return type;
            else
                return -1;
        }

        public int rewindRec()
        {
            cur = start;
            return type;
        }

        static public string segStr(int segid)
        {
            switch (segid)
            {
                case 0: return "ABSOLUTE";
                case 1: return "CODE";
                case 2: return "DATA";
                case 3: return "STACK";
                case 4: return "MEMORY";
                case 255:
                    return "//";
            }
            return string.Format("/{0}/", segid);
        }

        static public string fixStr(int fixid)
        {
            switch (fixid)
            {
                case 1: return "LOW";
                case 2: return "HIGH";
                case 3: return "WORD";
            }
            return string.Format("fixup({0})", fixid);
        }

        static public string alignStr(int alignid)
        {
            switch (alignid)
            {
                case 0: return "abs";
                case 1: return "inpage";
                case 2: return "page";
                case 3: return "byte";
            }
            return string.Format("align({0})", alignid);
        }
    }
}

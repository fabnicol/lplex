/*
    util.cpp - misc utilities.
    Copyright (C) 2006-2011 Bahman Negahban

    Adapted to C++-17 in 2018-2019 by Fabrice Nicol

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
    Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/



#include "platform.h"
#include "util.h"

#include <sys/stat.h>
uint16_t xlogExists = 0, _verbose = 0,  _xcode = 0, blip_len = 0, blip_ct = 0;
std::string _blip = "", _affirm = "";
char propellor[] = { '-', '\0', '\\', '\0', '|', '\0', '/', '\0' };

#if defined(WIN32_COLOR) || defined(ANSI_COLOR)

#if defined(WIN32_COLOR)
int colorMode = win32;
#else
int colorMode = ansi;
#endif



//struct{ char *ansiStr; int winVal; } colorTable[] =
colorIndex colorTable[] =
{
    { "\e[1m",     0 },       // BOLD
    { "\e[5m",     0 },       // BLINK
    { "\e[4m",     0 },       // UNDERLINE
    { "\e[7m",     0 },       // REVERSE
    { "\e[1,7m",   0 },       // LTREVERSE

    { "\e[30m",    0 },       // BLACK
    { "\e[31m",    4 },       // RED
    { "\e[32m",    2 },       // GREEN
    { "\e[33m",    6 },       // YELLOW
    { "\e[34m",    1 },       // BLUE
    { "\e[35m",    5 },       // MAGENTA
    { "\e[36m",    3 },       // CYAN
    { "\e[37m",    7 },       // WHITE

    { "\e[1;30m",  8 },       // GRAY
    { "\e[1;31m", 12 },       // LTRED
    { "\e[1;32m", 10 },       // LTGREEN
    { "\e[1;33m", 14 },       // LTYELLOW
    { "\e[1;34m",  9 },       // LTBLUE
    { "\e[1;35m", 13 },       // LTMAGENTA
    { "\e[1;36m", 11 },       // LTCYAN
    { "\e[1;37m", 15 },       // LTWHITE

    { "\e[40m",    0 },       // BGBLACK
    { "\e[41m",    0 },       // BGRED
    { "\e[42m",    0 },       // BGGREEN
    { "\e[43m",    0 },       // BGYELLOW
    { "\e[44m",    0 },       // BGBLUE
    { "\e[45m",    0 },       // BGMAGENTA
    { "\e[46m",    0 },       // BGCYAN
    { "\e[47m",    0 },       // BGWHITE

    { "\e[41;1m",  0 },       // BGLTRED
    { "\e[42;1m",  0 },       // BGLTGREEN
    { "\e[43;1m",  0 },       // BGLTYELLOW
    { "\e[44;1m",  0 },       // BGLTBLUE
    { "\e[45;1m",  0 },       // BGLTMAGENTA
    { "\e[46;1m",  0 },       // BGLTCYAN
    { "\e[47;1m",  0 }        // BGLTWHITE
};

colorval consoleColorReset;

colorval tintColor = colorText::LTCYAN;
colorval errColor = colorText::LTRED;
colorval warnColor = colorText::LTGREEN;
colorval blipColor = colorText::LTYELLOW;

void setcolors (int scheme)
{
    //   ::colorMode = true;
    if (scheme == bright)
        {
            tintColor = colorText::LTCYAN;
            errColor = colorText::LTRED;
            warnColor = colorText::LTGREEN;
            blipColor = colorText::LTYELLOW;
        }

    else
        if (scheme == dark)
            {
                tintColor = colorText::LTBLUE;
                errColor = colorText::LTRED;
                warnColor = colorText::GREEN;
                blipColor = colorText::CYAN;
            }

        else
            ::colorMode = false;
}

#endif


#ifdef ERR2LOGMACRO

std::ofstream xlog;
std::string xlogName;
const std::string normalize_windows_paths (const std::string & path )
{
#ifdef _WIN32
    std::string u ( path);
    std::replace(u.begin(), u.end(), '/', '\\');
    return u;
#else
    return(path);
#endif
}

const std::string normalize_windows_paths (const fs::path &path )
{
std::string _path = path.string();
#ifdef _WIN32

    return(normalize_windows_paths(_path) );
#else
return(_path);
#endif
}



const char * scrub()
{
    int l = blip_len;

    while (l)
        {
           std::cerr << " ";
            l--;
        }

    while (blip_len)
        {
           std::cerr << "\b";
            blip_len--;
        }

    blip_ct = 0;
   std::cerr.flush();
    return "";
}

void scrub (int n)
{
    while (n--)
       std::cerr << "\b \b";
}

void unblip (bool clear)
{
    scrub();
    scrub (_blip.length());

    if (clear)
        _blip.clear();
}

std::string toUpper (const std::string &s)
{
    std::string t = s;

    for (auto &c : t)
        c = ::toupper (c);

    return t;
}

void blip (const std::string& msg)
{
    blip (msg.c_str());
}

void blip (const char *msg)
{
    if (msg)
        {
            blip_len = 0;
           std::cerr << TINT_BLIP (msg);

            while (* (msg + blip_len++))
               std::cerr << "\b";
        }

    else
        {
            blip_len = 1;
           std::cerr << TINT_BLIP ( (const char*) propellor + (blip_ct % 4 * 2)) << "\b";
        }

   std::cerr.flush();
    blip_ct++;
}

// ----------------------------------------------------------------------------
//    logInit :
// ----------------------------------------------------------------------------
//    Opens self-declared message log as <filename>.
//
//    Returns
// ----------------------------------------------------------------------------

void logInit (const std::string& filename)
{
    if (xlog.is_open())
        xlog.close();

    xlogExists = 0;
    xlog.clear();

    if (! filename.empty())
        {
            xlogName = filename;
        }

    xlog.open (xlogName.c_str());

    if (! xlog.is_open())
        {
            ERR ("Couldn't open log file \'" + std::string (xlogName) + "\'.\n");
        }

    else
        xlogExists = 1;
}

// ----------------------------------------------------------------------------
//    logCopy :
// ----------------------------------------------------------------------------
//    Copies message log to <filename>.
//
//    Returns 0
// ----------------------------------------------------------------------------

int logCopy (const fs::path& filename)
{
    if (! filename.empty())
        {
            INFO (_f ("Saving log file to \'%s\'\n", filename.string()));
            fs::copy_file (xlogName, filename);
        }

    return 0;
}

// ----------------------------------------------------------------------------
//    logClose :
// ----------------------------------------------------------------------------
//    Closes message log.
//
//    Returns 0
// ----------------------------------------------------------------------------

int logClose()
{
    xlog.close();
    return 0;
}




// ----------------------------------------------------------------------------
//    logDelete :
// ----------------------------------------------------------------------------
//    Deletes message log.
//
//    Returns 0
// ----------------------------------------------------------------------------


int logDelete()
{
    xlog.close();
    remove (xlogName.c_str());
    return 0;
}




// ----------------------------------------------------------------------------
//    logReopen :
// ----------------------------------------------------------------------------
//    Reopens message log if closed.
//
//    Returns 0
// ----------------------------------------------------------------------------


int logReopen()
{
    if (! xlog.is_open())
        {
            xlog.clear();
            xlog.open (xlogName, std::ios::app | std::ios::ate);
        }

    return 0;
}

#else
ofstream xlog;
std::string xlogName;
char * scrub() {}
#endif



// ----------------------------------------------------------------------------
//    outputhex :
// ----------------------------------------------------------------------------
//    Writes <title> and <address> to stderr, then hex std::string representation
//    of <n>_bytess, 16 per line, starting at <buf>.
// ----------------------------------------------------------------------------


void outputhex (unsigned char *buf, int n, char *title, int address)
{
    int i;
    fprintf (stderr, "\n(%X) %s:\n", address, title);

    for (i = 0; i < n; i++)
        {
            fprintf (stderr, "%02x ", buf[i]);

            if ( (i + 1) % 16 == 0)
                fprintf (stderr, "\n");
        }

    fprintf (stderr, "\n");
}



// ----------------------------------------------------------------------------
//    outputhexraw :
// ----------------------------------------------------------------------------
//    Writes hex std::string representation of <n>_bytess starting at <buf> to stderr.
// ----------------------------------------------------------------------------


void outputhexraw (unsigned char *buf, int n)
{
    for (int i = 0; i < n; i++)
        fprintf (stderr, "%02x", buf[i]);
}



// ----------------------------------------------------------------------------
//    otherThan :
// ----------------------------------------------------------------------------
//    Returns whether <n>_bytess starting at <buf> are anything other than <c>.
// ----------------------------------------------------------------------------


int otherThan (const char c, unsigned char *buf, int n)
{
    for (int i = 0; i < n; ++i)
        if (buf[i] != c)
            return 1;

    return 0;
}



// ----------------------------------------------------------------------------
//    hexToStr :
// ----------------------------------------------------------------------------
//    Returns hex std::string representation of <n>_bytess starting at <buf>,
//    with newline every <w>_bytess.
// ----------------------------------------------------------------------------


std::string hexToStr (const unsigned char *buf, int n, int w __attribute__ ( (unused)))
{
    std::string str;

    for (int i = 0; i < n; ++i)
        str += _f ("%02x", buf[i]);

    return str;
}




// ----------------------------------------------------------------------------
//    strtomd5 :
// ----------------------------------------------------------------------------
//    Converts 32_bytes std::string representation <txt> to 16_bytes md5 signature
//    value <md5Str>.
//
//    Returns 1 on success, 0 on fail
// ----------------------------------------------------------------------------
// (adapted from John Walker's md5::main.c::main() <http://www.fourmilab.ch/md5>)


int strtomd5 (md5_byte_t *md5Str, const char *txt)
{
    int i;
    unsigned int val;
    char *hexPair = (char*) txt;
    memset (md5Str, 0, 16);

    for (i = 0; i < 16; ++i)
        {
            if (isxdigit ( (int) hexPair[0]) && isxdigit ( (int) hexPair[1]) &&
                    sscanf (hexPair, "%02X", &val) == 1)
                {
                    md5Str[i] = (unsigned char) val;
                }

            else
                return 0;

            hexPair += 2;
        }

    return 1;
}



// ----------------------------------------------------------------------------
//    filesize :
// ----------------------------------------------------------------------------
//    Returns filesize of <filename>.
// ----------------------------------------------------------------------------


size_t filesize (const char * filename)
{
    size_t size;
   std::ifstream file (filename, std::ios::binary | std::ios::ate);

    if (! file.is_open())
        ERR ("Can't find input file " + std::string (filename) + "\n");

    size = file.tellg();
    file.close();
    return size;
}

// ----------------------------------------------------------------------------
//    sizeStr :
// ----------------------------------------------------------------------------
//    Returns <size> as std::string in either GB or MB.
// ----------------------------------------------------------------------------


std::string sizeStr (uint64_t size)
{
    if (size > GIGABYTE)
        return _f ("%s%.2f GB", (double) size / GIGABYTE < 10 ? " " : "",
                   (double) size / GIGABYTE);

    else
        return _f ("%d MB", size / MEGABYTE);
}



// ----------------------------------------------------------------------------
//    deviceNum :
// ----------------------------------------------------------------------------
//    Returns device number for <filename>.
// ----------------------------------------------------------------------------


dev_t deviceNum (const std::string &filename)
{
    struct stat filestat;

    if (stat (filename.c_str(), &filestat) == 0)
        return filestat.st_dev;

    return -1;
}



// ----------------------------------------------------------------------------
//    statsize :
// ----------------------------------------------------------------------------
//    Returns size of <filename>.
// ----------------------------------------------------------------------------


size_t statsize (const char * filename)
{
    struct stat filestat;

    if (stat (filename, &filestat) == 0)
        return filestat.st_size;

    return 0;
}



/*
    util.h - misc utilities.
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



#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <locale>
#include <stdint.h>
#include <vlc_bits.h>
#include <md5/md5.h>

#ifndef LPLEX_PCH_INCLUDED
#include "lplex_precompile.h"
#endif

#define ANSI_COLOR

#include "platform.h"

namespace fs = std::filesystem;


#define MEGABYTE 1048576
#define GIGABYTE 1073741824

template <typename... Args>
static inline std::string _f (const char* format, Args... args)
{
    char buffer[5128] = {0};
    sprintf (buffer, format, args...);
    return std::string (buffer);
}

static std::stringstream  _s;

std::string toUpper (const std::string &s);

#define QUOTE(s) ("\"" + std::string(s) + "\"")

#define Left(X) substr(0, X)
#define Right(X, Y)  (Y < X.length() ? X.substr( X.length() - Y) : std::string(""))

// trim from start (in place)
static inline void ltrim (std::string &s)
{
    s.erase (s.begin(), std::find_if (s.begin(), s.end(), [] (int ch)
    {
        return !std::isspace (ch);
    }));
}

// trim from end (in place)
static inline void rtrim (std::string &s)
{
    s.erase (std::find_if (s.rbegin(), s.rend(), [] (int ch)
    {
        return !std::isspace (ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim (std::string &s)
{
    ltrim (s);
    rtrim (s);
}

#define clearbits(val,mask) val = ((val|(mask))^(mask))
#define matchbits(val,mask) (((!(mask))^val)==(mask))
#define bEndian32(ptr) bEndian(*(uint32_t*)(ptr))

struct byteRange
{
    uint8_t *start;
    uint16_t len;
};

void outputhexraw (unsigned char *buf, int n);
void outputhex (unsigned char *buf, int n, char *title, int address);
int otherThan (const char c, unsigned char *buf, int n);
std::string hexToStr (const unsigned char *buf, int n, int w = 16);
int strtomd5 (md5_byte_t *md5Str, const char *txt);
size_t filesize (const char * filename);
std::string sizeStr (uint64_t size);
//char device( const char * filename );
dev_t deviceNum (const std::string& filename);
size_t statsize (const char * filename);
void _pause();
int progress (int val, int action);

// generic contexts for incremental processes
enum
{
    _none,
    _isNew,
    _openIt,
    _initIt,
    _isOpen,
    _reopen,
    _addThis,
    _setThis,
    _endIt,
    _closeIt
};


class hexStr
{
    public:
        const unsigned char *data;
        int len;

        hexStr (const void *buf, int n) :
        data(static_cast<const unsigned char *>(buf)), len(n) {}

        friend std::ostream& operator << (std::ostream& stream, const hexStr& x)
        {
            std::_Ios_Fmtflags fl = stream.flags();

            for (int i = 0; i < x.len; i++)
                stream << std::hex << std::noskipws << std::setw (2) << std::setfill ('0') << (short) x.data[i];

            stream << std::dec;
            stream.setf (fl);
            return stream;
        }
};


template<class T> void sortUnique (std::vector<T> &things)
{
    std::sort (things.begin(), things.end());
    things.erase (std::unique (things.begin(), things.end()), things.end());
}


// logging and messaging. Gets a little convoluted...
// ...log & message macros
#define lplex_console

#ifdef lplex_console

#define gui_err(m) 0
#define gui_update(v,m) 0
#define gui_pulse(m) 0

#else

class messenger
{
    public:
        enum { _info, _stat, _warn, _err, _fatal };
        messenger() {}
        virtual bool err (const std::string& msg) = 0;
        virtual bool setMsgMode (int m)
        {
            msgMode = m;
        }
        virtual bool update (int val, const std::string& msg, bool* skip = false) = 0;
        virtual bool pulse (const std::string& msg, bool* skip = false) = 0;
    protected:
        int msgMode;
};
extern messenger *myMessenger;
#define gui_err(m) myMessenger->err(_s<<m)
#define gui_mode(m) myMessenger->setMsgMode(m)
#define gui_update(v,m) myMessenger->update(v,m)
#define gui_pulse(m) myMessenger->pulse(m)

#endif

extern uint16_t xlogExists, _verbose, _xcode;

#define ERR2LOGMACRO
extern std::ofstream xlog;

#ifdef ERR2LOGMACRO

extern std::string xlogName;
void logInit (const std::string& logFilename = "");
int logCopy (const fs::path& filename = "");
int logClose();
int logDelete();
int logReopen();

#define XLOGMACRO(t) xlog << t

#else
#define XLOGMACRO(t) 0
int logInit (const std::string& logFilename = "") {}
int logCopy (const fs::path& filename = "") {}
int logClose() {}
int logDelete() {}
int logReopen() {}

#endif

#if 1
#define STAT_TAG "STAT: "
#define INFO_TAG "INFO: "
#define LOG_TAG  "    : "
#else
#define STAT_TAG ""
#define INFO_TAG ""
#define LOG_TAG  ""
#endif
#define WARN_TAG "WARN: "
#define ERR_TAG  "*ERR: "
#define DBUG_TAG "DBUG: "

#if defined(WIN32_COLOR) || defined(ANSI_COLOR)

#include "color.h"
extern colorval tintColor;
extern colorval errColor;
extern colorval warnColor;
extern colorval blipColor;
enum { bright = 1, dark };
void setcolors (int scheme = bright);
#define TINT(t) "" //(colorText){t,tintColor}
#define TINT_ERR(t) (colorText){t,errColor}
#define TINT_STAT(t) t
#define TINT_INFO(t) t
#define TINT_LOG(t) t
#define TINT_WARN(t) (colorText){t,warnColor}
#define TINT_BLIP(t) (colorText){t,blipColor}

#else

#define TINT(t) t
#define TINT_ERR(t) t
#define TINT_STAT(t) t
#define TINT_INFO(t) t
#define TINT_LOG(t)  t
#define TINT_WARN(t) t
#define TINT_BLIP(t) t

#endif

#define ECHO(t) std::cerr << t  << std::endl;
#define ECHOv(t) ECHO(t)


#define XERRMACRO(tag,tint,t)std::cerr <<  tag << t;

#define STAT(t) ECHO("[MSG] " << t);
#define INFO(t) ECHO("[INF] " << t );
#define LOG(t)  ECHO("[LOG] " << t )
#define WARN(t) ECHO("[WAR] " << t);
#define ERR(t)  ECHO("[ERR] " << t);

#define STATv(t) STAT(t)
#define INFOv(t) INFO(t)
#define LOGv(t)  LOG(t)
#define WARNv(t) WARN(t)

#define FATAL(t) {  \
   ERR(t);\
   fflush(nullptr);\
    throw;}//must follow 'if' statement

#define BLIP(t) do{ if(!_verbose) { unblip(true);std::cerr << TINT_BLIP( (_blip + std::string(t)).c_str() ) << std::flush; } }while(0)
#define SCRN(t) do{ if(!_verbose) { unblip(true);std::cerr << t; } }while(0);
#define POST(t) do{ SCRN(t); std::cerr << t; }while(0)

#define DBUG(t) ERR( (std::string(DBUG_TAG) + std::string(t)).c_str() )

// ...screen log & message functions:

extern uint16_t blip_len, blip_ct;
extern std::string _blip, _affirm;
extern char propellor[];

const char * scrub();
void blip (const char *msg = nullptr);
void blip (const std::string& msg);

const std::string normalize_windows_paths (const std::string &path);

const std::string   normalize_windows_paths (const fs::path &path);
void unblip (bool clear = true);

// ...standardized counter with progress reporting

template<class T> struct counter
{
    T start, now, max;
    counter<T> (T s = 0,  T n = 0,  T m = 0) : start (s), now (n), max (m) {}
    void operator = (const counter<T> & other)
    {
        start = other.start;
        now = other.now;
        max = other.max;
    }
};


template<class T>
static inline void blip (counter<T> *ct,
                         int skip = 1, const char *suffix = "", const char *prefix = "")
{
    static std::string pref;

    if (blip_ct == 0)
        {
            if (_verbose)
                {
                    _blip = "";
                    pref = "--";
                }

            else
                pref = "";
        }

    if (! (blip_ct % skip))
        {
            if (ct->max && ct->max - ct->now)
                {

                    int val = static_cast<int>(( ct->now - ct->start ) * 1000 / ( ct->max - ct->start ));
                    blip (_f ("%s%2d.%d%% %s", pref, val / 10, val % 10, suffix));
                }

            else
                {
                    std::string msg = _f ("%s%d %s", pref, ct->now, suffix);
                    blip (msg);
                }
        }

    else
        blip_ct++;
}

long execute (const std::string& application, const std::vector<const char*>& command, int verbose = _verbose);

long execute (const std::string& application,  const std::vector<const char*>& args,
              const std::string& application2, const std::vector<const char*>& args2,
              int verbose);
#endif

/*
	exec.cpp - functions to launch external processes.
	Copyright (C) 2006-2011 Bahman Negahban

    Adapted to C++-17 in 2018 by Fabrice Nicol

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



#include "util.h"
#include <cstdio>
#include <fstream>
#include <cerrno>
#include <fstream>
#include <cstdlib>
#if defined (__WIN32__)  || defined(__WIN64)|| defined(__w64)
#include <windows.h>
#include <cstdio>
#include <tchar.h>
#include <tchar.h>
#include <strsafe.h>
#define BUFSIZE 4096
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hChildStd_IN_Rd2 = NULL;
HANDLE g_hChildStd_IN_Wr2 = NULL;
HANDLE g_hChildStd_OUT_Rd2 = NULL;
HANDLE g_hChildStd_OUT_Wr2 = NULL;

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box,
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifdef NOWAIT
#undef NOWAIT
#endif
#define NOWAIT -1


extern std::ofstream xlog;

inline std::string quote(const std::string& path)
{
      	return (std::string("\"") + path + std::string("\""));
}

std::string get_cl(const char* application, const std::vector<const char*>& Args, uint16_t start)
{
    std::string cmd;
    auto it = Args.begin() + start;
    while (it != Args.end())
    {
        std::string s = *it;
	bool do_quote=((s[0] != '"') && (s[0] != '-') && (s[0] != '|')) ;
	do_quote=false;
        cmd += ((do_quote)? quote(s): s);
        if (it + 1 != Args.end()) cmd += " ";
        ++it;
    }
    cmd = std::string(application) + ".exe "  + cmd;

    normalize_windows_paths(cmd);

return cmd;
}

int run(const char* application, const std::vector<const char*>&  Args, const int option)
{
errno=0;

auto _Args = Args;
_Args.insert(_Args.begin(), fs::path(application).filename().string().c_str());
_Args.insert(_Args.end(), NULL);



#if ! defined (_WIN32)  && ! defined(__WIN32)

    int pid;
    int tube[2];
    char c;
    const char* const* args =  _Args.data();
    if (pipe(tube))
    {
        perror("[ERR] pipe run\n");
        return errno;
    }

    switch (pid = fork())
    {
        case -1:
           std::cerr << "[ERR] Could not launch " << application <<endl;
            break;
        case 0:
            close(tube[0]);
            dup2(tube[1], STDERR_FILENO);
            execv(application, (char* const*) args);
           std::cerr << "[ERR] Runtime failure in " <<  application << " child process" << std::endl;

            return errno;

        default:
            close(tube[1]);
            dup2(tube[0], STDIN_FILENO);
            while (read(tube[0], &c, 1) == 1)
            {
               std::cerr <<  c;
#               ifdef _ERR2LOG
                  xlog << c;
#               endif
            }

            if (option != NOWAIT) waitpid(pid, NULL, option);
            close(tube[0]);
    }
#else

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    std::string _application = std::string(application) + ".exe";
    std::string cmdline = get_cl(application, Args, 0);
    std::cerr << std::endl<<_application.c_str()<< std::endl;
    std::cerr << cmdline.c_str() << std::endl;

    // Start the child process.
    if( !CreateProcessA( _application.c_str(),   // No module name (use command line)
        const_cast<char*>(cmdline.c_str()),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
       std::cerr << "CreateProcess failed " << GetLastError() << std::endl;
        return -1;
    }

    // Wait until child process exits.
    if (option != NOWAIT) WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

#endif
    return errno;
}


int run (const char* application,  const std::vector<const char*>& Args,
        const char* application2, const std::vector<const char*>& Args2,
        const int option)
{
#ifdef __linux__
sync();
int pid2;
char c;
int tube[2];
int tubeerr[2];
int tubeerr2[2];

// execv conventions

auto _Args = Args;
auto _Args2 = Args2;
_Args.insert(_Args.begin(), fs::path(application).filename().string().c_str());
_Args.insert(_Args.end(), nullptr);

_Args2.insert(_Args2.begin(), fs::path(application2).filename().string().c_str());
_Args2.insert(_Args2.end(), nullptr);

const char* const* args =  _Args.data();
const char* const* args2 = _Args2.data();

// Two extra tubes are in order to redirect jpeg2yuv and mpeg2enc stdout messages and realign them with overall stdout messages, otherwise they fall out of sync
// with one another and dvda-author messages.

if (pipe(tube) || pipe(tubeerr) || pipe(tubeerr2))
{
    perror("[ERR] Pipe");
    return errno;
}

// Owing to the piping of the stdout streams (necessary for coherence of output) existence checks must be tightened up.
// System will freeze should an input file not exit, as mjpegtools to not always exit on system error. This may cause a loop in the piping of jpeg2yuv to mpeg2enc
// Tight system error strategy in order here

errno=0;

switch (fork())
{
case -1:
   std::cerr << "[ERR] Could not launch " << application << std::endl;
    break;

case 0:

    close(tube[0]);
    close(tubeerr[0]);
    dup2(tube[1], STDOUT_FILENO);
    // Piping stdout is required here as STDOUT is not a possible duplicate for stdout
    dup2(tubeerr[1], STDERR_FILENO);
    execv(application, (char* const*) args);
   std::cerr << "[ERR] Runtime failure in jpeg2yuv child process" << std::endl;

    return errno;


default:
    close(tube[1]);
    close(tubeerr[1]);
    dup2(tube[0], STDIN_FILENO);
   std::cerr << "[INF] Piping to ..." << application2 << std::endl;

    switch (pid2 = fork())
    {
    case -1:
       std::cerr << "[ERR] Could not launch " << application2 << std::endl;
        break;

    case 0:
        // This looks like an extra complication as it could be considered to simply use dup2(STDOUT_FILENO, stdout_FILENO) without further piping
        // However this would reverse the order of jpeg2yuv and mpeg2enc stdout messages, the latter comming first,
        // which is not desirable as jpeg2yuv is piped into mpeg2enc. Hereby we are realigning these msg streams, which even in bash piping are intermingled,
        // making it hard to read/use.
        close(tubeerr2[0]);
        close(STDOUT_FILENO);
        dup2(tubeerr2[1], STDERR_FILENO);
        // End of comment
        execv(application2, (char* const*) args2);
       std::cerr << "[ERR] Runtime failure in " << application2 << " parent process" << std::endl;
        return errno;

    default:
        if (option != NOWAIT) waitpid(pid2, NULL, 0);
        dup2(tubeerr[0], STDIN_FILENO);

        while (read(tubeerr[0], &c, 1) == 1)std::cerr << c;
        close(tubeerr[0]);
        close(tubeerr2[1]);
        dup2(tubeerr2[0], STDIN_FILENO);

        while (read(tubeerr2[0], &c, 1) == 1)std::cerr << c;
        close(tubeerr2[0]);
    }
    close(tube[0]);
}
return errno;

#else

       std::string cmdline = get_cl(application, Args, 0);
       std::string cmdline2 = get_cl(application2, Args2, 0);

	  std::cerr <<"###" << cmdline << "###" << std::endl;

	  std::cerr <<"###" << cmdline2 << "###" << std::endl;

       std::string command =  cmdline + std::string(" | ") + cmdline2;

	  std::cerr << "!!!" << command << "!!!"  << std::endl;

      std::cerr << "***" <<  command << "***" << std::endl;

       //int res =
        system(command.c_str());
return 0;
#endif
}


long execute( const std::string& application, const std::vector<const char*>& args, int verbose)
{
#ifdef _ERR2LOG
	if( verbose > -1 )
    {
        xlog << std::endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args)  xlog << s << " ";
        xlog << std::endl;
    }
	else
		xlog.close();
#endif

    if (verbose > -1)
    {
       std::cerr << std::endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args) std::cerr << s << " ";
    }

    std::string _application = application;
    normalize_windows_paths(_application);

    run(_application.c_str(), args, 0);

	return errno;
}



long execute( const std::string& application,  const std::vector<const char*>& args,
              const std::string& application2, const std::vector<const char*>& args2,
              int verbose)
{
#ifdef _ERR2LOG
	if( verbose > -1 )
    {
        xlog << std::endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args)  xlog << s << " ";
    }
	else
		xlog.close();
#endif

    if (verbose > -1)
    {
        xlog << " | ";
        for(const auto& s: args2)  xlog << s << " ";
        xlog << std::endl;
    }

	if( verbose > -1 )
    {
        std::cerr << std::endl << STAT_TAG << "Running command : " << std::endl << application << " ";
         for(const auto& s: args) std::cerr << s << " ";
        std::cerr << " | ";
        std::cerr << application2 << " ";
         for(const auto& s: args2) std::cerr << s << " ";
    }

    run(application.c_str(), args, application2.c_str(), args2, 0);

	return errno;
}


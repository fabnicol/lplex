/*
	exec.cpp - functions to launch external processes.
	Copyright (C) 2006-2011 Bahman Negahban

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

#if defined (__WIN32__)  || defined(__WIN64)|| defined(__w64)
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifdef NOWAIT
#undef NOWAIT
#endif
#define NOWAIT -1


extern ofstream xlog;

inline string quote(const string& path)
{
  return (string("\"") + path + string("\""));
}

string get_cl(const vector<const char*>& Args, uint16_t start)
{
    string cmd;
    auto it = Args.begin() + start;
    while (it != Args.end()) 
    {
        bool do_quote=((*it[0] != '"') && (*it[0] != '-') && (*it[0] != '|')) ;
        
        cmd += ((do_quote)? quote(string(*it)): string(*it));
        cmd += " ";
        ++it;
    }

    return cmd;
}

int run(const char* application, const vector<const char*>&  Args, const int option)
{
errno=0;

const char* const* args =  Args.data();

#if ! defined (__WIN32__)  && ! defined(__WIN64) && ! defined(__w64)
    int pid;
    int tube[2];
    char c;

    char mesg[strlen(application)+1+7];
    memset(mesg, '0', sizeof(mesg));

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
            std::cerr << "[ERR] Runtime failure in " <<  application << " child process" << endl;
    
            return errno;
    
        default:
            close(tube[1]);
            dup2(tube[0], STDIN_FILENO);
            while (read(tube[0], &c, 1) == 1) 
            {
                cerr <<  c;
#               ifdef _ERR2LOG
                  xlog << c;
#               endif
            }
            
            if (option != NOWAIT) waitpid(pid, NULL, option);
            close(tube[0]);
    }
#else

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    LPTSTR cmdline = (LPTSTR) get_cl(Args, 1).c_str();
    
    // Start the child process. 
    if( !CreateProcess( application,   // No module name (use command line)
        cmdline,        // Command line
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
        printf( "CreateProcess failed (%d).\n", GetLastError() );
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


int run(const char* application,  const vector<const char*>& Args,
        const char* application2, const vector<const char*>& Args2,
        const int option)
{
#ifdef __linux__
sync();
int pid2;
char c;
int tube[2];
int tubeerr[2];
int tubeerr2[2];
const char* const* args =  Args.data();
const char* const* args2 =  Args2.data();

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
    cerr << "[ERR] Could not launch " << application << endl;
    break;

case 0:

    close(tube[0]);
    close(tubeerr[0]);
    dup2(tube[1], STDOUT_FILENO);
    // Piping stdout is required here as STDOUT is not a possible duplicate for stdout
    dup2(tubeerr[1], STDERR_FILENO);
    execv(application, (char* const*) args);
    cerr << "[ERR] Runtime failure in jpeg2yuv child process" << endl;
    
    return errno;


default:
    close(tube[1]);
    close(tubeerr[1]);
    dup2(tube[0], STDIN_FILENO);
    cerr << "[INF] Piping to ..." << application2 << endl;

    switch (pid2 = fork())
    {
    case -1:
        cerr << "[ERR] Could not launch " << application2 << endl;
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
        cerr << "[ERR] Runtime failure in " << application2 << " parent process" << endl;
        return errno;

    default:
        if (option != NOWAIT) waitpid(pid2, NULL, 0);
        dup2(tubeerr[0], STDIN_FILENO);

        while (read(tubeerr[0], &c, 1) == 1) cerr << c;
        close(tubeerr[0]);
        close(tubeerr2[1]);
        dup2(tubeerr2[0], STDIN_FILENO);

        while (read(tubeerr2[0], &c, 1) == 1) cerr << c;
        close(tubeerr2[0]);
    }
    close(tube[0]);
}
#endif
}


long execute( const string& application, const vector<const char*>& args, int verbose)
{
#ifdef _ERR2LOG
	if( verbose > -1 )
    {
        xlog << endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args)  xlog << s << " ";
        xlog << endl;
    }   
	else
		xlog.close();
#endif
   
    if (verbose > -1)
    {
        cerr << endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args)  cerr << s << " ";
    }
    
    run(application.c_str(), args, 0);
	
	return errno;
}



long execute( const string& application,  const vector<const char*>& args,
              const string& application2, const vector<const char*>& args2,
              int verbose)
{
#ifdef _ERR2LOG
	if( verbose > -1 )
    {
        xlog << endl << STAT_TAG << "Running command : " << application << " ";
        for(const auto& s: args)  xlog << s << " ";
    }   
	else
		xlog.close();
#endif
   
    if (verbose > -1)
    {
        xlog << " | ";
        for(const auto& s: args2)  xlog << s << " ";
        xlog << endl;
    }

	if( verbose > -1 )
    {
         cerr << endl << STAT_TAG << "Running command : " << endl << application << " ";
         for(const auto& s: args)  cerr << s << " ";
         cerr << " | ";
         cerr << application2 << " ";
         for(const auto& s: args2)  cerr << s << " ";
    }
     
   // run(application.c_str(), args, application2.c_str(), args2, 0);
	run(application.c_str(), args,0);
	return errno;
}


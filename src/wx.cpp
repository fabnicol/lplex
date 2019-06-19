/*
    wx.cpp - misc wxwidgets extensions.
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

#include <iostream>
#include <filesystem>
#include "util.h"

namespace fs = std::filesystem;


// ----------------------------------------------------------------------------
//    fs_GetTempDir :
// ----------------------------------------------------------------------------
//    Returns path to system temp directory, including trailing setarator.
// ----------------------------------------------------------------------------


std::string fs_GetTempDir()
{
    std::string temp = (fs::temp_directory_path() / "dummy").string();
    remove (temp.c_str());
    return temp;
}




// ----------------------------------------------------------------------------
//    fs_DeleteDir :
// ----------------------------------------------------------------------------
//    Deletes the folder <dirName> entirely.
//
//    Returns true on success, false on fail
// ----------------------------------------------------------------------------

#include <dirent.h>
#include <sys/stat.h>
bool fs_DeleteDir (const fs::path& dirName)
{
 std::error_code err;
 long n = fs::remove_all (dirName, err);
 int res = err.value();

 if (res == static_cast<uintmax_t>(-1))
 {
     std::cerr << "[ERR] Directory " << dirName << " could not be removed." << std::endl;
 }
 else
 {
     std::cerr << "[MSG] Directory " << dirName << " was removed." << std::endl;
 }

 if (n == 1)
 {
     std::cerr << "[MSG]  " << n << " directory was removed." << std::endl;
 }
 else if (n > 1)
 {
     std::cerr << "[MSG]  " << n << " files and directories were removed." << std::endl;
 }
 else
 {
     std::cerr << "[WAR] Directory " << dirName << " did not exist." << std::endl;
 }
 return (res == static_cast<uintmax_t>(-1));
}

// ----------------------------------------------------------------------------
//    fs_MakeDirs :
// ----------------------------------------------------------------------------
//    Creates all nonexistent folders in path <dirName>.
//
//    Returns true on success, false on fail
// ----------------------------------------------------------------------------

bool fs_MakeDirs (const fs::path& dirName)
{
    return fs::create_directories (dirName);
}

// ----------------------------------------------------------------------------
//    fs_GetAllDirs :
// ----------------------------------------------------------------------------
//    Collects all subfolder names under folder <dirName> into array <dirs>.
//
//    Returns number of subfolders on success, 0 on fail or no subfolders
// ----------------------------------------------------------------------------

size_t fs_GetAllDirs (const std::string& dirName,std::vector<std::string>& dirs)
{
    int n = 0;

    for (auto& p : fs::directory_iterator (dirName))
        {
            if (fs::is_directory (p.path()))
                {
                    dirs.emplace_back (p.path().string());
                    ++n;
                }
        }

    return n;
}

// ----------------------------------------------------------------------------
//    fs_DirSize :
// ----------------------------------------------------------------------------
//    Returns size of given directory <dirName> on success, 0 on fail
// ----------------------------------------------------------------------------

size_t fs_DirSize (const fs::path& dirName)
{
    size_t total_size = 0;

    for (auto& p : fs::directory_iterator (dirName))
        {
            if (fs::is_directory (p.path()))
                {
                    total_size += fs_DirSize (p.path());
                }

            else
                if (fs::is_regular_file (p.path()))
                    {
                        total_size += fs::file_size (p.path());
                    }
        }

    return total_size;
}


// ----------------------------------------------------------------------------
//    fs_validPath :
// ----------------------------------------------------------------------------
//    Tests existence of path.
// ----------------------------------------------------------------------------

bool fs_validPath (const fs::path& p)
{
    return (fs::exists (p));
}


// ----------------------------------------------------------------------------
//    fs_fixSeparators :
// ----------------------------------------------------------------------------
//    Ensures separators in <path> conform to system.
// ----------------------------------------------------------------------------


void fs_fixSeparators (char * path)
{
    if (strcmp (SEPARATOR, "\\") == 0)
        {
            int i = 0;

            while (path[i] != '\0')
                {
                    if (path[i] == '/')
                        path[i] = SEPARATOR[0];

                    ++i;
                }
        }

    else
        {
            int i = 0;

            while (path[i] != '\0')
                {
                    if (path[i] == '\\')
                        path[i] = SEPARATOR[0];

                    ++i;
                }
        }
}

#include "archive/archivelib.h"
#include "support/timelib.h"
#include <algorithm>
#include <sys/stat.h>

//! Make a directory and any missing parents
//! \return True if the directory exists afterwards.
static bool data_mkpath(string path)
{
    if (path.empty() || data_isdir(path))
    {
        return !path.empty();
    }
    size_t slash = path.find_last_of('/');
    if (slash != string::npos && slash > 0 && !data_mkpath(path.substr(0, slash)))
    {
        return false;
    }
    return COSMOS_MKDIR(path.c_str(), 00777) == 0 || errno == EEXIST;
}

// Half a millisecond [days], added before splitting a time into day and second, so a
// time a hair under a boundary (from MJD rounding) lands in the day and second it names
static const double ARCHIVE_ROUND = .0005 / 86400.;

//! Get COSMOS Archive Directory
/*! The archive holds a realm's stored telemetry, events and science data, by node:
 * <archive>/<realm>/<node>/<kind>/<yyyy>/<ddd>. It is COSMOSARCHIVE if set,
 * otherwise <cosmosroot>/archive.
    \param create_flag Create directory if not already present.
 * \return Path, or empty if not found.
*/
string get_cosmosarchive(bool create_flag)
{
    string archive;
    char *troot = getenv("COSMOSARCHIVE");
    if (troot != nullptr)
    {
        archive = troot;
    }
    else
    {
        string root;
        if (get_cosmosroot(root, create_flag) < 0 || root.empty())
        {
            return "";
        }
        archive = root + "/archive";
    }
    if (data_isdir(archive) || (create_flag && data_mkpath(archive)))
    {
        return archive;
    }
    return "";
}

//! Get an Archive Day Directory
/*! \param realm Realm name.
 * \param node Node name.
 * \param kind Archive kind: "telemetry" (telemetry and events) or "data" (science data).
 * \param mjd Time within the day (UTC).
    \param create_flag Create the path if not already present.
 * \return <archive>/<realm>/<node>/<kind>/<yyyy>/<ddd>, or empty if absent or not created.
*/
string get_archivedir(string realm, string node, string kind, double mjd, bool create_flag)
{
    string archive = get_cosmosarchive(create_flag);
    if (archive.empty())
    {
        return "";
    }
    int32_t year, month;
    double day, doy;
    mjd2ymd(mjd + ARCHIVE_ROUND, year, month, day, doy);
    char daypath[32];
    snprintf(daypath, sizeof(daypath), "%04d/%03d", year, static_cast<int>(doy));
    string path = archive + "/" + realm + "/" + node + "/" + kind + "/" + daypath;
    if (data_isdir(path) || (create_flag && data_mkpath(path)))
    {
        return path;
    }
    return "";
}

//! Archive File Name
/*! \param prefix Kind of file: "telemetry", "event", "passive", "active".
 * \param mjd Time of the file's first record (UTC).
 * \param ext Extension, without the dot.
 * \return <prefix>_<yyyydddhhmmss>.<ext>
*/
string archive_name(string prefix, double mjd, string ext)
{
    int32_t year, month;
    double day, doy;
    mjd2ymd(mjd + ARCHIVE_ROUND, year, month, day, doy);
    int32_t seconds = std::min(86399, static_cast<int32_t>((doy - floor(doy)) * 86400.));
    char stamp[64];
    snprintf(stamp, sizeof(stamp), "%04d%03d%02d%02d%02d", year, static_cast<int>(doy), seconds / 3600, (seconds / 60) % 60, seconds % 60);
    return prefix + "_" + stamp + "." + ext;
}

bool archive_parse_name(const string &name, string &prefix, double &utc)
{
    size_t under = name.rfind('_');
    size_t dot = name.rfind('.');
    if (under == string::npos || dot == string::npos || dot != under + 14)
    {
        return false;
    }
    string stamp = name.substr(under + 1, 13);
    for (char c : stamp)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    int32_t year = stoi(stamp.substr(0, 4));
    int32_t doy = stoi(stamp.substr(4, 3));
    int32_t hour = stoi(stamp.substr(7, 2));
    int32_t minute = stoi(stamp.substr(9, 2));
    int32_t second = stoi(stamp.substr(11, 2));
    prefix = name.substr(0, under);
    utc = cal2mjd(year, 1.) + (doy - 1) + (hour + (minute + second / 60.) / 60.) / 24.;
    return true;
}

vector<archivefile> archive_list(string realm, string node, string kind, double utcbegin, double utcend, string prefix)
{
    vector<archivefile> files;
    double span = kind == "telemetry" ? 1./24. : 10./1440.;
    for (double day = floor(utcbegin - span); day <= floor(utcend); day += 1.)
    {
        string dir = get_archivedir(realm, node, kind, day);
        if (dir.empty())
        {
            continue;
        }
        for (filestruc &tf : data_list_files(dir))
        {
            archivefile af;
            if (!archive_parse_name(tf.name, af.prefix, af.utc))
            {
                continue;
            }
            if (!prefix.empty() && af.prefix != prefix)
            {
                continue;
            }
            // Data is listed by header; its .img follows from the name
            if (kind == "data" && tf.name.substr(tf.name.size() - 4) != ".hdr")
            {
                continue;
            }
            if (af.utc <= utcend && af.utc + span > utcbegin)
            {
                af.kind = kind;
                af.size = tf.size;
                af.path = tf.path;
                files.push_back(af);
            }
        }
    }
    sort(files.begin(), files.end(), [](const archivefile &a, const archivefile &b) { return a.utc < b.utc; });
    return files;
}


ArchiveLines::ArchiveLines(string realm, string node, string prefix)
    : realm(realm), node(node), prefix(prefix)
{
}

ArchiveLines::ArchiveLines(ArchiveLines &&other) noexcept
    : realm(std::move(other.realm)), node(std::move(other.node)), prefix(std::move(other.prefix)),
      path(std::move(other.path)), fp(other.fp), hourend(other.hourend)
{
    other.fp = nullptr;
}

ArchiveLines::~ArchiveLines()
{
    Close();
}

int32_t ArchiveLines::Write(double mjd, const string &line)
{
    if (fp == nullptr || mjd + ARCHIVE_ROUND >= hourend)
    {
        Close();
        string dir = get_archivedir(realm, node, "telemetry", mjd, true);
        if (dir.empty())
        {
            return DATA_ERROR_ARCHIVE;
        }
        path = dir + "/" + archive_name(prefix, mjd, "json");
        fp = fopen(path.c_str(), "w");
        if (fp == nullptr)
        {
            path.clear();
            return -errno;
        }
        hourend = (floor((mjd + ARCHIVE_ROUND) * 24.) + 1.) / 24.;
    }
    if (fputs(line.c_str(), fp) == EOF || fputc('\n', fp) == EOF)
    {
        return -errno;
    }
    return 0;
}

void ArchiveLines::Close()
{
    if (fp != nullptr)
    {
        fclose(fp);
        fp = nullptr;
    }
    path.clear();
}

#ifndef ARCHIVELIB_H
#define ARCHIVELIB_H

//! \file archivelib.h
//! The SOSE archive (ground only): <archive>/<realm>/<node>/<kind>/<yyyy>/<ddd>/<prefix>_<yyyydddhhmmss>.<ext>,
//! kind "telemetry" (telemetry and event JSON lines, one file per hour) or "data" (L0 ENVI files).

#include "support/configCosmos.h"
#include "support/datalib.h"

//! \brief Return the COSMOS archive root: $COSMOSARCHIVE, or <cosmosroot>/archive.
//! \param create_flag Create if missing; default false.
//! \return Absolute archive root path, or "" on failure.
string get_cosmosarchive(bool create_flag=false);

//! \brief Return an archive day directory, <archive>/<realm>/<node>/<kind>/<yyyy>/<ddd>.
//! \param realm Realm name. \param node Node name.
//! \param kind  "telemetry" or "data". \param mjd Time within the day (UTC).
//! \param create_flag Create the whole path if missing; default false.
//! \return Absolute directory path, or "" on failure.
string get_archivedir(string realm, string node, string kind, double mjd, bool create_flag=false);

//! \brief Return an archive file name, <prefix>_<yyyydddhhmmss>.<ext>.
//! \param prefix e.g. "telemetry", "event", "passive", "active".
//! \param mjd    Time of the file's first record (UTC). \param ext Extension without the dot.
string archive_name(string prefix, double mjd, string ext);

//! One archive file, from its name <prefix>_<yyyydddhhmmss>.<ext>
struct archivefile
{
    string kind;
    string prefix;
    //! Time of the file's first record or frame (UTC)
    double utc = 0.;
    off_t size = 0;
    string path;
};

//! \brief Parse an archive file name, <prefix>_<yyyydddhhmmss>.<ext>.
//! \return True if the name has that form.
bool archive_parse_name(const string &name, string &prefix, double &utc);

//! \brief List the archive files of one kind that may hold records in a time window, by time.
//! A telemetry file holds up to an hour from its name's time; a data file (listed by its
//! .hdr) up to 10 minutes.
//! \param realm Realm name. \param node Node name. \param kind "telemetry" or "data".
//! \param utcbegin Start of the window (UTC). \param utcend End of the window (UTC).
//! \param prefix Only files with this prefix (e.g. "event"); empty for all.
//! \return The files, sorted by time.
vector<archivefile> archive_list(string realm, string node, string kind, double utcbegin, double utcend, string prefix="");


//! Writes JSON lines into the archive, one file per hour of record time:
//! <archive>/<realm>/<node>/telemetry/<yyyy>/<ddd>/<prefix>_<yyyydddhhmmss>.json, named by
//! the file's first record. A file of the same name is replaced.
class ArchiveLines
{
public:
    ArchiveLines(string realm, string node, string prefix);
    ArchiveLines(const ArchiveLines &) = delete;
    ArchiveLines &operator=(const ArchiveLines &) = delete;
    ArchiveLines(ArchiveLines &&other) noexcept;
    ~ArchiveLines();
    //! Write one line; starts a new file when mjd is past the current file's hour.
    //! \return Zero, or negative error.
    int32_t Write(double mjd, const string &line);
    void Close();

    string realm;
    string node;
    string prefix;
    //! Path of the open file, or empty
    string path;
private:
    FILE *fp = nullptr;
    double hourend = 0.;
};

#endif // ARCHIVELIB_H

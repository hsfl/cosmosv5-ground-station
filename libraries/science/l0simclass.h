#ifndef L0SIMCLASS_H
#define L0SIMCLASS_H

#include "support/configCosmos.h"
#include "support/convertlib.h"
#include "support/datalib.h"
#include "archive/archivelib.h"
#include "support/envi.h"

//! \file l0simclass.h
//! Pass 2 of simulated science data (task 004): render the L0 data of archived
//! imaging tiles. The node file's instruments.<name> holds the detector model
//! (physical pixels, acquired rows, downlinked superpixels), the forward model
//! placeholders and, per mode, the virtual camera that takes its tiles; the archive's telemetry gives the state and its tile events the
//! acquisition times. Each tile becomes <archive>/<realm>/<node>/data/<yyyy>/<ddd>/
//! <mode>_<yyyydddhhmmss>.img|hdr.

namespace Cosmos
{
    namespace Science
    {
        class L0Sim
        {
        public:
            //! A channel's filter stripe: its along-track angle from the boresight (+ = ahead)
            struct Stripe
            {
                double along = 0.;
                int32_t first_row = 0;
            };

            //! One physical camera inside the instrument and how it is read. These are
            //! internal: the node commands the virtual cameras of the modes.
            struct Camera
            {
                string name;
                string detector;
                // Physical detector
                uint16_t cols = 0;
                uint16_t rows = 0;
                double pitch = 0.;
                double ifov = 0.;
                double read_noise = 0.;
                double dark_current = 0.;
                double full_well = 0.;
                uint16_t bits = 16;
                map<int32_t, double> qe;
                // Acquisition
                uint16_t sp_across = 1;
                uint16_t sp_along = 1;
                uint16_t swath_cols = 0;
                uint16_t first_col = 0;
                map<int32_t, Stripe> stripes;
            };

            //! One L0 band: a channel of a camera, with its forward model
            struct Band
            {
                int32_t nm = 0;
                size_t camera = 0;
                Stripe stripe;
                double albedo = 0.;
                double bandwidth = 0.;
                //! Solar spectral irradiance at 1 AU [W/m²/nm]
                double irradiance = 0.;
                //! Active mode: laser energy reaching the surface per observation [J]
                double laser_energy = 0.;
            };

            struct Mode
            {
                string name;
                //! The virtual camera (a CAM device) whose snap tiles this mode renders
                string camera;
                double rate = 0.;
                double inttime = 0.;
                //! Passive channels or active laser wavelengths [nm]
                vector<int32_t> channels;
                //! Active mode: detector region around the laser return [pixels]
                uint16_t roi[2] = {12, 12};
            };

            //! An archived imaging tile
            struct Tile
            {
                string mode;
                string name;
                string request;
                string camera;
                //! Start of the first frame's exposure (UTC)
                double utc = 0.;
                uint32_t frames = 0;
                double rate = 0.;
                double inttime = 0.;
            };

            //! An archived state: selenocentric position/velocity, ICRF attitude
            struct Knot
            {
                double utc = 0.;
                rvector s;
                rvector v;
                quaternion q;
                rvector w;
            };

            //! Read instruments.<instrument> from <realm>/nodes/<node>.json
            int32_t Load(string realm, string node, string instrument="lvmm");
            //! Read the archived telemetry covering [utcbegin, utcend]
            int32_t LoadTelemetry(double utcbegin, double utcend);
            //! The archived tiles of every mode that start in [utcbegin, utcend]
            vector<Tile> LoadTiles(double utcbegin, double utcend);
            //! Interpolated position (Hermite on position and velocity) and attitude (slerp)
            int32_t Interpolate(double utc, rvector &s, quaternion &q);
            //! Render one tile into the archive; an existing file is kept unless overwrite
            //! \return Lines written, 0 if skipped, or negative error.
            int32_t Generate(const Tile &tile, bool overwrite=false);
            //! Render every tile that starts in [utcbegin, utcend]
            //! \return Tiles written, or negative error.
            int32_t Run(double utcbegin, double utcend, bool overwrite=false);

            string realm;
            string node;
            string instrument;
            string baseline;
            bool provisional = true;
            double aperture = 0.;
            double transmission = 0.5;
            uint16_t samples = 0;
            uint16_t dem_iterations = 3;
            vector<Camera> cameras;
            map<string, Mode> modes;
            map<int32_t, Band> bands;
            vector<Knot> knots;
            //! Samples whose line of sight missed the Moon in the last Generate()
            size_t misses = 0;

        private:
            int32_t PassiveTile(const Tile &tile, const Mode &mode, string path);
            int32_t ActiveTile(const Tile &tile, const Mode &mode, string path);
            vector<const Band *> ModeBands(const Mode &mode);
            bool Intersect(const rvector &sc, const rvector &dir, const rmatrix &j2s, double demres, rvector &point, double &lat, double &lon);
            envi_hdr TileHeader(const Tile &tile, const vector<const Band *> &bl, string path, size_t columns, size_t lines, size_t missinglines);
            string KnotsText(double utcbegin, double utcend);
        };
    }
}

#endif // L0SIMCLASS_H

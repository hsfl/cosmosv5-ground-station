// archive_simulate: fill the SOSE archive with simulated data for a realm (task 004)
// Pass 1 propagates the realm's nodes, running their command queues, and writes each
// step's telemetry and every event into the archive. Pass 2 renders the L0 data of the
// archived imaging tiles. Either pass runs alone.

#include "support/configCosmos.h"
#include "support/elapsedtime.h"
#include "support/jsonlib.h"
#include "physics/simulatorclass.h"
#include "archive/archivelib.h"
#include "science/l0simclass.h"

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s '{\"key\":value, ...}' | -h\n"
            "Simulates a realm into the archive (<archive> is $COSMOSARCHIVE, else <cosmos>/archive):\n"
            "pass 1 writes <archive>/<realm>/<node>/telemetry, pass 2 <archive>/<realm>/<node>/data.\n"
            "  realmname      realm (required)\n"
            "  initialutc     start, MJD (default: now)\n"
            "  endutc         end, MJD; or\n"
            "  runcount       number of steps (default 1500)\n"
            "  simdt          step [s] (default 10)\n"
            "  archive        1 = pass 1: propagate and archive telemetry and events (default 1)\n"
            "  data           1 = pass 2: render the L0 data of the archived tiles (default 1)\n"
            "  dataoverwrite  1 = replace L0 files that already exist (default 0)\n"
            "  attitude       attitude propagator: lvlh (keeps commanded attitudes) or iterative (default lvlh)\n"
            "Example: %s '{\"realmname\":\"lunar\",\"initialutc\":61340.6875,\"runcount\":7185}'\n",
            program, program);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || string(argv[1]) == "-h" || string(argv[1]) == "--help")
    {
        print_usage(argv[0]);
        return argc < 2 ? 1 : 0;
    }
    string error;
    json11::Json jargs = json11::Json::parse(argv[1], error);
    if (!error.empty() || !jargs.is_object() || jargs["realmname"].string_value().empty())
    {
        fprintf(stderr, "Error: the argument must be a JSON object with a realmname%s%s\n", error.empty() ? "" : ": ", error.c_str());
        print_usage(argv[0]);
        return JSON_ERROR_SCAN;
    }
    string realmname = jargs["realmname"].string_value();
    double simdt = jargs["simdt"].is_number() ? jargs["simdt"].number_value() : 10.;
    double runcount = jargs["runcount"].is_number() ? jargs["runcount"].number_value() : 1500.;
    bool archive = jargs["archive"].is_number() ? jargs["archive"].int_value() != 0 : true;
    bool data = jargs["data"].is_number() ? jargs["data"].int_value() != 0 : true;
    bool dataoverwrite = jargs["dataoverwrite"].int_value() != 0;
    string attitude = jargs["attitude"].is_string() ? jargs["attitude"].string_value() : "lvlh";

    if (get_cosmosarchive(true).empty())
    {
        fprintf(stderr, "Error: no archive directory: set COSMOSARCHIVE or check <cosmos>\n");
        return GENERAL_ERROR_OPEN;
    }

    Physics::Simulator sim;
    if (sim.GetError() < 0)
    {
        fprintf(stderr, "Error creating the simulator: %s\n", cosmos_error_string(sim.GetError()).c_str());
        return sim.GetError();
    }
    // Init adds its time to the orbit file's epoch: only pass one that was asked for
    sim.Init(simdt, realmname, jargs["initialutc"].is_number() ? jargs["initialutc"].number_value() : 0.);
    sim.sat_atype = attitude == "iterative" ? Physics::Propagator::AttitudeIterative : Physics::Propagator::AttitudeLVLH;
    sim.ParseOrbitFile("");
    sim.ParseSatFile("");
    if (sim.cnodes.empty())
    {
        fprintf(stderr, "Error: no nodes loaded for realm \"%s\" (%s)\n", realmname.c_str(), get_realmdir(realmname).c_str());
        return GENERAL_ERROR_EMPTY;
    }
    sim.ParseTargetFile("");
    double initialutc = sim.currentutc;
    double endutc = jargs["endutc"].is_number() ? jargs["endutc"].number_value() : initialutc + runcount * simdt / 86400.;
    sim.AddGroundStations();

    if (archive)
    {
        ElapsedTime et;
        // One telemetry and one event writer per node
        vector<ArchiveLines> archivesoh, archiveevent;
        for (auto &state : sim.cnodes)
        {
            archivesoh.emplace_back(realmname, state->currentinfo.node.name, "telemetry");
            archiveevent.emplace_back(realmname, state->currentinfo.node.name, "event");
        }
        sim.Target();
        sim.Metric();
        size_t steps = 0;
        size_t events = 0;
        while (sim.currentutc <= endutc + simdt / 172800.)
        {
            for (size_t i = 0; i < sim.cnodes.size(); ++i)
            {
                cosmosstruc &info = sim.cnodes[i]->currentinfo;
                int32_t iretn = archivesoh[i].Write(info.node.utc, json_soh_payload(&info).dump());
                // Filed by the step that reported it, so an event may predate the file's first record
                for (const eventstruc &event : info.event)
                {
                    if (event.utc == 0. || iretn < 0)
                    {
                        continue;
                    }
                    iretn = archiveevent[i].Write(info.node.utc, json_event_payload(info.node.name, event).dump());
                    ++events;
                }
                if (iretn < 0)
                {
                    fprintf(stderr, "Error writing the archive for %s: %s\n", info.node.name.c_str(), cosmos_error_string(iretn).c_str());
                    return iretn;
                }
            }
            sim.Propagate(0., false, true);
            sim.Target();
            sim.Metric();
            ++steps;
        }
        fprintf(stderr, "Pass 1: %lu steps, %lu events from %.6f to %.6f in %.1f s\n", steps, events, initialutc, endutc, et.split());
    }

    if (data)
    {
        for (auto &state : sim.cnodes)
        {
            string name = state->currentinfo.node.name;
            Science::L0Sim l0;
            if (l0.Load(realmname, name) < 0)
            {
                continue;
            }
            ElapsedTime et;
            int32_t count = l0.Run(initialutc, endutc, dataoverwrite);
            if (count < 0)
            {
                fprintf(stderr, "Pass 2 %s: %s\n", name.c_str(), cosmos_error_string(count).c_str());
            }
            else
            {
                fprintf(stderr, "Pass 2 %s: %d tiles in %.1f s\n", name.c_str(), count, et.split());
            }
        }
    }
    return 0;
}

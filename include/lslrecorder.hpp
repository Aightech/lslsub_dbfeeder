#ifndef __LSLRECORDER_HPP__
#define __LSLRECORDER_HPP__

#include <lsl_cpp.h>
#include <psql.hpp>
#include <strANSIseq.hpp>
#include <thread>

namespace lsl
{
class Recorder : public virtual ESC::CLI
{
    public:
    Recorder(lsl::stream_info strm_info, int verbose = -1)
        : CLI(verbose, "lslrec"), m_db(verbose - 1)
    {
        set_cli_id("lslrec_" + strm_info.name());
        m_strm_info = strm_info;
        m_table_name = strm_info.name() + "_" + m_uid;
        m_db.connect("lsldb", "lsldb_user", "azerty", "127.0.0.1", "5432");
        m_status = 0;

        m_thread = new std::thread(&Recorder::store_stream, this);
    };

    void start_recording(std::string uid)
    {
        m_table_name = m_strm_info.name() + "_" + uid;
        m_db.create_table(m_table_name);
        logln("Start recording [" + m_table_name + "]", true);
        m_sample_index = 0;
        m_status = 1;
    };

    void pause_recording()
    {
        logln("Pause recording [" + m_table_name + "]", true);
        logln("Stored " + std::to_string(m_sample_index) + " samples.", true);
        m_status = 0;
    };

    void stop_recording()
    {
        m_status = -1;
        logln("Stop recording [" + m_table_name + "]", true);
        m_thread->join();
        logln("Stored " + std::to_string(m_sample_index) + " samples.", true);
    };

    private:
    void store_stream()
    {
        logln("Recording thread [" + m_strm_info.name() + "]", true);
        try
        {
            lsl::stream_inlet inlet(m_strm_info, 360, 0, false);
            std::vector<std::vector<double>> chunk;
            std::vector<double> timestamps;

            while(m_status != -1)
            {
                chunk.clear();
                timestamps.clear();
                if(inlet.pull_chunk(chunk, timestamps))
                {
                    if(m_status == 1)
                        m_sample_index = m_db.store_data(
                            m_table_name, chunk, timestamps, m_sample_index);
                }
            }
        }
        catch(std::string msg)
        {
            log_error(msg);
        }
        logln("Recording thread [" + m_strm_info.name() + "] stopped.", true);
        m_db.disconnect();
    };

    lsl::stream_info m_strm_info;
    std::string m_table_name;
    std::string m_uid;
    int m_status;
    psql::Database m_db;
    uint64_t m_sample_index;
    std::thread *m_thread;
};
} // namespace lsl

#endif // __LSLRECORDER_HPP__
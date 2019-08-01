#ifndef RECORDER_H
#define RECORDER_H

#include "../http/httpclient.h"

#include <iostream>

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <boost/filesystem.hpp>

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

class Recorder
{
    const int DEFAULT_DELAY = 5; // Seconds
    const uint DEFAULT_PORT = 3333;
    const std::string DEFAULT_HOSTNAME = "home.eee";
	
public:
    struct WaveHeader
    {
                    char RIFF_marker[4];
                    uint32_t file_size;
                    char filetype_header[4];
                    char format_marker[4];
                    uint32_t data_header_length;
                    uint16_t format_type;
                    uint16_t number_of_channels;
                    uint32_t sample_rate;
                    uint32_t bytes_per_second;
                    uint16_t bytes_per_frame;
                    uint16_t bits_per_sample;
    };

    explicit Recorder();
    int record_audio();
    int send_recording(std::string recording);

private:
    WaveHeader *genericWAVHeader(uint32_t sample_rate, uint16_t bit_depth, uint16_t channels);
    int writeWAVHeader(int fd, WaveHeader *hdr);
    int recordWAV(const char *fileName, struct WaveHeader *hdr, uint32_t duration);

    uint m_port;
    std::string m_hostname;
};

#endif // RECORDER_H

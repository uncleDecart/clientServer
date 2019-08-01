#include "recorder.h"


// Compile with "g++ test.ccp -o test -lasound"

// Use the newer ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>


Recorder::WaveHeader* Recorder::genericWAVHeader(uint32_t sample_rate, uint16_t bit_depth, uint16_t channels)
{
    struct WaveHeader *hdr;
    hdr = (WaveHeader*) malloc(sizeof(*hdr));
    if (!hdr)
        return NULL;

    memcpy(&hdr->RIFF_marker, "RIFF", 4);
    memcpy(&hdr->filetype_header, "WAVE", 4);
    memcpy(&hdr->format_marker, "fmt ", 4);
    hdr->data_header_length = 16;
    hdr->format_type = 1;
    hdr->number_of_channels = channels;
    hdr->sample_rate = sample_rate;
    hdr->bytes_per_second = sample_rate * channels * bit_depth / 8;
    hdr->bytes_per_frame = channels * bit_depth / 8;
    hdr->bits_per_sample = bit_depth;

    return hdr;
}

int Recorder::writeWAVHeader(int fd, struct WaveHeader *hdr)
{
    if (!hdr)
        return -1;

    write(fd, &hdr->RIFF_marker, 4);
    write(fd, &hdr->file_size, 4);
    write(fd, &hdr->filetype_header, 4);
    write(fd, &hdr->format_marker, 4);
    write(fd, &hdr->data_header_length, 4);
    write(fd, &hdr->format_type, 2);
    write(fd, &hdr->number_of_channels, 2);
    write(fd, &hdr->sample_rate, 4);
    write(fd, &hdr->bytes_per_second, 4);
    write(fd, &hdr->bytes_per_frame, 2);
    write(fd, &hdr->bits_per_sample, 2);
    write(fd, "data", 4);

    uint32_t data_size = hdr->file_size + 8 - 44;
    write(fd, &data_size, 4);

    return 0;
}

/*
 * duraion in seconds
 */
int Recorder::recordWAV(const char *fileName, struct WaveHeader *hdr, uint32_t duration)
{
    int err;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int sampleRate = hdr->sample_rate;
    int dir;
    snd_pcm_uframes_t frames = 32;
    char *device = (char*) "plughw:0,0";
    char *buffer;
    int filedesc;

    printf("Capture device is %s\n", device);

    /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0)
    {
        fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(err));
        return err;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* ### Set the desired hardware parameters. ### */

    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
    {
        fprintf(stderr, "Error setting interleaved mode: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Signed 16-bit little-endian format */
    if (hdr->bits_per_sample == 16) err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    else err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
    if (err < 0)
    {
        fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Two channels (stereo) */
    err = snd_pcm_hw_params_set_channels(handle, params, hdr->number_of_channels);
    if (err < 0)
    {
        fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* 44100 bits/second sampling rate (CD quality) */
    sampleRate = hdr->sample_rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Error setting sampling rate (%d): %s\n", sampleRate, snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    hdr->sample_rate = sampleRate;
    /* Set period size*/
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* Use a buffer large enough to hold one period */
    err = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Error retrieving period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    size = frames * hdr->bits_per_sample / 8 * hdr->number_of_channels; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);
    if (!buffer)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    err = snd_pcm_hw_params_get_period_time(params, &sampleRate, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Error retrieving period time: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        free(buffer);
        return err;
    }

    uint32_t pcm_data_size = hdr->sample_rate * hdr->bytes_per_frame * duration / 1000;
    hdr->file_size = pcm_data_size + 44 - 8;

    filedesc = open(fileName, O_WRONLY | O_CREAT, 0644);
    err = writeWAVHeader(filedesc, hdr);
    if (err < 0)
    {
        fprintf(stderr, "Error writing .wav header.");
        snd_pcm_close(handle);
        free(buffer);
        close(filedesc);
        return err;
    }
    fprintf(stdout, "Channels: %d\n", hdr->number_of_channels);
    for(int i = duration * 1000 / sampleRate; i > 0; i--)
    {
        err = snd_pcm_readi(handle, buffer, frames);
        if (err == -EPIPE) fprintf(stderr, "Overrun occurred: %d\n", err);
        if (err < 0) err = snd_pcm_recover(handle, err, 0);
        // Still an error, need to exit.
        if (err < 0)
        {
            fprintf(stderr, "Error occured while recording: %s\n", snd_strerror(err));
            snd_pcm_close(handle);
            free(buffer);
            close(filedesc);
            return err;
        }
        write(filedesc, buffer, size);
    }

    close(filedesc);
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    printf("Finished writing to %s\n", fileName);
    return 0;
}

int Recorder::record_audio()
{
    int err;
    struct WaveHeader *hdr;

    // Creates a temporary file in /tmp

    std::string wavFile = "tmp.wav";
    hdr = genericWAVHeader(44000, 16, 2);
    if (!hdr)
    {
        fprintf(stderr, "Error allocating WAV header.\n");
        return -1;
    }

    err = recordWAV(wavFile.c_str(), hdr, 1000 * 1);
    if (err < 0)
    {
            fprintf(stderr, "Error recording WAV file: %d\n", err);
            return err;
    }

    free(hdr);
    return 0;
}


void parse_response(const HTTPRequest& request,
                    const HTTPResponse& response,
                    const system::error_code& ec)
{
    std::cout << "GOT RESPONSE ! : "
              << response.get_response().rdbuf()
              << std::endl;
    return;
}

Recorder::Recorder() :
    m_port(DEFAULT_PORT),
    m_hostname(DEFAULT_HOSTNAME)
{
}

int Recorder::send_recording(std::string recording)
{
    try {
        using namespace std::placeholders;
        HTTPClient client;

        std::unique_ptr<HTTPPOSTRequest> request =
                client.create_post_request(1);

        request->set_host(m_hostname);
        request->set_filepath("tmp.wav");
        request->set_uri("1/tmp.wav");
        request->set_port(m_port);
        request->set_callback(parse_response);

        request->execute();

        client.close();
    }
    catch (system::system_error &e)
    {
        std::cout << "Error occured! Error code = " << e.code()
                  << ". Message: " << e.what() << std::endl;

        return e.code().value();
    }

    return 0;
}

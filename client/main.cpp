#include "recorder.h"
#include <cstdlib>

int main(int argc, char** argv)
{
    Recorder r;
    r.record_audio();
    r.send_recording("");
    return 0;
}

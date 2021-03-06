/* audiofilereader.cpp -- implements a class to read samples
 *
 * 14-Jun-08  RBD
 * 16-Jun-08  RBD revised to use libsndfile
 */
#include "assert.h"
#include "stdio.h"
#include "string.h"
#include "sndfile.h"
#include "audioreader.h"
#include "audiofilereader.h"

double Audio_file_reader::get_sample_rate()
{
    return sf_info.samplerate;
}


long Audio_file_reader::get_frames()
{
    return total_frames;
}


long Audio_file_reader::read(float *data, long n)
{
    // note that "samples_per_frame" is really "frames_per_window" in this
    // context, so we're computing bytes per window
    float *input_data = (float *) alloca(bytes_per_frame * samples_per_frame);
    assert(input_data != NULL) ;
	
    // read into the end of data
    long frames_read = sf_readf_float(sf, input_data, n);
    long chans = sf_info.channels;
    // now convert and move to beginning of data
    if (chans > 1) {
        for (int frame = 0; frame < frames_read; frame++) {
            float sum = 0;
            for (int chan = 0; chan < sf_info.channels; chan++) {
                // sum over channels within a frame
                sum += input_data[frame * chans + chan];
            }
            // write the frame sum to result array
            data[frame] = sum;
        }
    }
    return frames_read;
}


bool Audio_file_reader::open(char *filename, Scorealign &sa, bool verbose)
{
    bytes_per_frame = 0; // initialize now in case an error occurs
    name[0] = 0;
    bzero(&sf_info, sizeof(sf_info));
    sf = sf_open(filename, SFM_READ, &sf_info);
    if (!sf) return false;
    strncpy(name, filename, MAX_NAME_LEN);
    name[MAX_NAME_LEN] = 0; // just in case
    total_frames = (long) sf_seek(sf, 0, SEEK_END);
    sf_seek(sf, 0, SEEK_SET);
    // we're going to read floats, but they might be multi-channel...
    bytes_per_frame = sf_info.channels * sizeof(float);
    calculate_parameters(sa, verbose);
    return true;
}


void Audio_file_reader::close()
{
    sf_close(sf);
}


void Audio_file_reader::print_info()
{
    printf("   file name = %s\n", name);
    double sample_rate = sf_info.samplerate;
    printf("   sample rate = %g\n", sample_rate);
    printf("   channels = %d\n", sf_info.channels);
    /*=============================================================*/
    printf("   total frames number is = %d\n", total_frames);
    printf("   audio duration = %g seconds\n", total_frames / sample_rate);
    /*=============================================================*/
}

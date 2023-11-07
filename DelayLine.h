//
//  DelayLine.h
//  min.syrinx_tilde
//
//  Created by courtney on 11/7/23.
//
//  Based on ToneJS DelayLine:https://github.com/Tonejs/Tone.js/blob/dev/Tone/core/worklet/DelayLine.worklet.ts
/**
 * A multichannel buffer for use within an AudioWorkletProcessor as a delay line
 */
//translated in C++ by Courtney Brown -- for use with syrinx


#ifndef DelayLine_h
#define DelayLine_h

#include "c74_min.h"
#include <math.h>
using namespace c74::min;


class DelayLine {
    
    std::vector<std::vector<sample>> buffer;
    std::vector<int> writeHead;
    int size;

        
    DelayLine(int sz, int channels) {
            size = sz;

            // create the empty channels
            for (int i = 0; i < channels; i++) {
                std::vector<sample> newtrack;
                buffer.push_back(newtrack);
                writeHead.push_back(0);
            }
        }

        /**
         * Push a value onto the end
         * @param channel number
         * @param value number
         */
    void push(int channel, sample value) {
            writeHead[channel] += 1;
            if (writeHead[channel] > size) {
                writeHead[channel] = 0;
            }
            buffer[channel][writeHead[channel]] = value;
        }

        /**
         * Get the recorded value of the channel given the delay
         * @param channel number
         * @param delay number delay samples
         */
    sample get(int channel, double delay) {
            int readHead = writeHead[channel] - floor(delay);
            if (readHead < 0)
            {
                readHead += size;
            }
            return buffer[channel][readHead];
        }
};


#endif /* DelayLine_h */

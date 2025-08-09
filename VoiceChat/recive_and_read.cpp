#include <iostream>
#include <stdlib.h>
#include "portaudio.h"
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include "config.h"

extern std::atomic<bool> running;

void recive_and_read(SOCKET socket)
{
    PaStream* stream;
    PaStreamParameters outputParameters;
    PaError err;
    static int16_t buff[FRAMES_PER_BUFFER];

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "No default output device found.\n";
        return;
    }
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    double sample_rate = Pa_GetDeviceInfo(outputParameters.device)->defaultSampleRate;

    std::cout << "Sample Rate output: " << sample_rate << "\n";

    err = Pa_OpenStream(&stream, nullptr, &outputParameters,
        sample_rate, FRAMES_PER_BUFFER, paClipOff, nullptr, nullptr);
    if (err != paNoError)
    {
        std::cerr << "Pa_OpenStream(output) error: " << Pa_GetErrorText(err) << "\n";
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Pa_StartStream error: " << Pa_GetErrorText(err) << "\n";
        return;
    }

    //std::vector<int16_t> audio_buffer;
    //const int max_buffer_size = 3 * FRAMES_PER_BUFFER;
    //audio_buffer.reserve(max_buffer_size);

    //auto next_write = std::chrono::steady_clock::now() + std::chrono::milliseconds((int)(FRAMES_PER_BUFFER * 1000.0 / sample_rate));
   
    sockaddr_in srcAddr{};
    int addrLen = sizeof(srcAddr);
    while (running) 
    {
        int got = recvfrom(socket,
                            reinterpret_cast<char*>(buff),
                            FRAMES_PER_BUFFER * SAMPLE_SIZE,
                            0,
                            reinterpret_cast<sockaddr*>(&srcAddr),
                            &addrLen);
        if (got == SOCKET_ERROR) {
            int err = WSAGetLastError();
            std::cerr << "recvfrom failed, error = " << err << "\n";
            running = false;
            break;
        }
        if (got == 0) {
            std::cerr << "connection closed by peer\n";
            running = false;
            break;
        }

        //size_t framesGot = got / SAMPLE_SIZE;
        //audio_buffer.insert(audio_buffer.end(),
        //                    buff,
        //                    buff + framesGot);

        //if (audio_buffer.size() > max_buffer_size) {
            // обрезаем самые старые
        //    audio_buffer.erase(audio_buffer.begin(),
        //        audio_buffer.begin() + (audio_buffer.size() - max_buffer_size));
        //}

        //std::this_thread::sleep_until(next_write);

        /*
        int16_t playBuf[FRAMES_PER_BUFFER];
        if (audio_buffer.size() >= FRAMES_PER_BUFFER) {
            // есть что играть
            std::copy(audio_buffer.begin(),
                audio_buffer.begin() + FRAMES_PER_BUFFER,
                playBuf);
            // удал€ем сыгранные
            audio_buffer.erase(audio_buffer.begin(),
                audio_buffer.begin() + FRAMES_PER_BUFFER);
        }
        else {
            // недостаЄт данных Ч заполн€ем тишиной
            std::fill(playBuf, playBuf + FRAMES_PER_BUFFER, 0);
        }
        */
        //err = Pa_WriteStream(stream, playBuff, FRAMES_PER_BUFFER);

        err = Pa_WriteStream(stream, buff, FRAMES_PER_BUFFER);
        if (err != paNoError) {
            std::cerr << "Pa_WriteStream error: " << Pa_GetErrorText(err) << "\n";
            running = false;
            break;
        }

        //next_write += std::chrono::milliseconds((int)(FRAMES_PER_BUFFER * 1000.0 / sample_rate));

    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
};
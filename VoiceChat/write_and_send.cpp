#include <iostream>
#include <stdlib.h>
#include "portaudio.h"
#include <winsock2.h>
#include <windows.h>
#include "config.h"

extern std::atomic<bool> running;

void write_and_send(SOCKET sock, sockaddr_in addr)
{
    PaStreamParameters inputParameters;
    PaStream* stream = nullptr;
    PaError err;
    static int16_t buff[FRAMES_PER_BUFFER];

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "No default input device found.\n";
        running = false;
        return;
    }
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    double sample_rate = Pa_GetDeviceInfo(inputParameters.device)->defaultSampleRate;

    std::cout << "Sample Rate input: " << sample_rate << "\n";

    err = Pa_OpenStream(&stream, &inputParameters, nullptr,
        sample_rate, FRAMES_PER_BUFFER, paClipOff, nullptr, nullptr);
    if (err != paNoError)
    {
        std::cerr << "Pa_OpenStream(input) error: " << Pa_GetErrorText(err) << "\n";
        running = false;
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Pa_StartStream error: " << Pa_GetErrorText(err) << "\n";
        running = false;
        return;
    }

    while (running) {
        // Get audio information
        err = Pa_ReadStream(stream, buff, FRAMES_PER_BUFFER);
        if (err != paNoError) {
            std::cerr << "Pa_ReadStream error: " << Pa_GetErrorText(err) << "\n";
            running = false;
            break;
        }
        // Send UPD
        int bytesToSend = FRAMES_PER_BUFFER * SAMPLE_SIZE;
        int sent = sendto(sock,
            reinterpret_cast<const char*>(buff),
            bytesToSend, 0,
            reinterpret_cast<sockaddr*>(&addr),
            sizeof(addr));
        if (sent != bytesToSend) {
            std::cerr << "sendto failed, code=" << WSAGetLastError() << "\n";
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
};
#include <iostream>
#include <stdlib.h>
#include "portaudio.h"
#include <windows.h>
#include "config.h"

// CallBack funk
static int read_and_write(
    const void* inputBuffer,                    // Входной поток
    void* outputBuffer,                         // Выходной поток
    unsigned long framesPerBuffer,              // Количество отсчётов в одном блоке
    const PaStreamCallbackTimeInfo* timeInfo,   // Временная информация о текущём блоке
    PaStreamCallbackFlags statusFlags,          // Состояние аудио-потока
    void* userData)
{
    const float* in = static_cast<const float*>(inputBuffer);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        out[i] = in[i]; // Отправляем звук на выход (колонки)
    }

    return paContinue;
}

int close_with_error(PaStream* stream, PaError err)
{
    if (stream) {
        Pa_AbortStream(stream);
        Pa_CloseStream(stream);
    }
    std::cerr << "An error occurred while using the portaudio stream." << std::endl;
    std::cerr << "Error number: " << err << std::endl;
    std::cerr << "Error message : " << Pa_GetErrorText(err) << std::endl;

    system("pause");

    return -2;
}

int check_device()
{
    PaStreamParameters inputParametrs, outputParametrs;
    PaStream* stream = NULL; //Opening stream to read/write data
    PaError err;
    const PaDeviceInfo* inputInfo;
    const PaDeviceInfo* outputInfo;
    char* sampleBlock = NULL;
    int numChannels;

    // Get I/O device parameters
    inputParametrs.device = Pa_GetDefaultInputDevice();
    std::cout << "Input device: " << inputParametrs.device << std::endl;
    inputInfo = Pa_GetDeviceInfo(inputParametrs.device);
    std::cout << "Name: " << inputInfo->name << std::endl;
    std::cout << "LL: " << inputInfo->defaultLowInputLatency << std::endl;
    std::cout << "HL: " << inputInfo->defaultHighInputLatency << std::endl;

    outputParametrs.device = Pa_GetDefaultOutputDevice();
    std::cout << "Output device: " << outputParametrs.device << std::endl;
    outputInfo = Pa_GetDeviceInfo(outputParametrs.device);
    std::cout << "Name: " << outputInfo->name << std::endl;
    std::cout << "LL: " << outputInfo->defaultLowInputLatency << std::endl;
    std::cout << "HL: " << outputInfo->defaultHighInputLatency << std::endl;

    // Get channels num
    numChannels = inputInfo->maxInputChannels < outputInfo->maxOutputChannels
        ? inputInfo->maxInputChannels : outputInfo->maxOutputChannels;
    std::cout << "Num channels = " << numChannels << std::endl;

    inputParametrs.channelCount = numChannels;
    inputParametrs.sampleFormat = PA_SAMPLE_TYPE;
    inputParametrs.suggestedLatency = inputInfo->defaultHighInputLatency;
    inputParametrs.hostApiSpecificStreamInfo = NULL;

    outputParametrs.channelCount = numChannels;
    outputParametrs.sampleFormat = PA_SAMPLE_TYPE;
    outputParametrs.suggestedLatency = outputInfo->defaultHighOutputLatency;
    outputParametrs.hostApiSpecificStreamInfo = NULL;

    // Open stream
    err = Pa_OpenStream(
        &stream,
        &inputParametrs,
        &outputParametrs,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        read_and_write,
        NULL);

    if (err)
    {
        std::cerr << "Problems with stream open." << std::endl;
        return close_with_error(stream, err);
    }

    // Start Stream
    err = Pa_StartStream(stream);
    if (err)
    {
        std::cerr << "Problems with stream start." << std::endl;
        return close_with_error(stream, err);
    }

    std::cout << "Start to talk! Press Enter to stop." << std::endl;

    std::cin.get();

    std::cout << "Stop to talk!" << std::endl;

    // Stop Stream
    err = Pa_StopStream(stream);
    if (err)
    {
        std::cerr << "Problems with stream stop." << std::endl;
        return close_with_error(stream, err);
    }

    Pa_AbortStream(stream);

    // Close Stream
    err = Pa_CloseStream(stream);
    if (err)
    {
        std::cerr << "Problems with stream close." << std::endl;
        return close_with_error(stream, err);
    }

    return 0;
};

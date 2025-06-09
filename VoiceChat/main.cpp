#include <iostream>
#include <stdlib.h>
#include "portaudio.h"
#include <windows.h>
#include <winsock2.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 256
#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SIZE 4

static int close_with_error(PaStream* stream, PaError err)
{
    if (stream) {
        Pa_AbortStream(stream);
        Pa_CloseStream(stream);
    }
    Pa_Terminate();
    fprintf(stderr, "An error occurred while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));

    system("pause");

    return -2;
}

// CallBack funk
static int read_and_write(
    const void* inputBuffer,                    //Входной поток
    void* outputBuffer,                         //Выходной поток
    unsigned long framesPerBuffer,              //Количество отсчётов в одном блоке
    const PaStreamCallbackTimeInfo* timeInfo,   //Временная информация о текущём блоке
    PaStreamCallbackFlags statusFlags,          //Состояние аудио-потока
    void* userData)
    {
    const float* in = static_cast<const float*>(inputBuffer);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        out[i] = in[i]; // Отправляем звук на выход (колонки)
    }

    return paContinue;
}

int main() 
{
    PaStreamParameters inputParametrs, outputParametrs;
    PaStream* stream = NULL; //Opening stream to read/write data
    PaError err;
    const PaDeviceInfo* inputInfo;
    const PaDeviceInfo* outputInfo;
    char* sampleBlock = NULL;
    int i;
    int numBytes;
    int numChannels;
    int numDevices;

    // Initaialize PortAudio
    err = Pa_Initialize();
    if (!err) {
        std::cerr << "Could not initialize PortAudio." << std::endl;
        return close_with_error(stream, err);
    }


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

    if (!err)
    {
        std::cerr<< "Problems with stream open." << std::endl;
        return close_with_error(stream, err);
    }

    // Start Stream
    err = Pa_StartStream(stream);
    if (!err)
    {
        std::cerr << "Problems with stream start." << std::endl;
        return close_with_error(stream, err);
    }

    //main box to organize VoiceChat.





    std::cout << "Start to talk!" << std::endl;

    std::cin.get();

    std::cout << "Stop to talk!" << std::endl;

    // Stop Stream
    err = Pa_StopStream(stream);
    if (!err)
    {
        std::cerr << "Problems with stream stop." << std::endl;
        return close_with_error(stream, err);
    }

    // Close Stream
    err = Pa_CloseStream(stream);
    if (!err)
    {
        std::cerr << "Problems with stream close." << std::endl;
        return close_with_error(stream, err);
    }
    
    // Terminate PortAudio
    err = Pa_Terminate(); //Check error of terminating
    if (!err)
    {
        std::cerr << "Could not terminate PortAudio: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    system("pause");

    return 0;
    
}
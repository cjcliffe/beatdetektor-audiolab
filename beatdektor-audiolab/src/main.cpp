//
//  main.cpp
//  Beatdetektor2-audiolab
//
//  Created by Charles J. Cliffe on 2014-07-05.
//  Copyright (c) 2013 Charles J. Cliffe. All rights reserved.
//

#include <windows.h>
#include <iostream>
#include <unistd.h>

#ifdef WIN32
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include "Timer.h"
#include "FFT.h"
#include "BeatDetektor.h"

#define SRATE 44100
#define BUF 2048

using namespace std;

ALCdevice *audio_device;
ALbyte audio_buffer[SRATE * 2];
ALint samples;

vector<float> sample_data;
vector<float> fft_data;
vector<float> fft_collapse;

Timer visTimer;
Timer fpsTimer;
bool bpm_latch = false;

BeatDetektor *det_low;
BeatDetektor *det_high;
BeatDetektor *det;
//BeatDetektorVU *vu;

int initAudio() {
	ALenum err = alGetError();
	const ALchar *pDeviceList = alcGetString(NULL,
			ALC_CAPTURE_DEVICE_SPECIFIER);
	if (pDeviceList) {
		printf("Available Capture Devices are:\n");
		while (*pDeviceList) {
			printf("**%s**\n", pDeviceList);
			pDeviceList += string(pDeviceList).length() + 1;
		}
	}

	audio_device = alcCaptureOpenDevice(NULL, SRATE, AL_FORMAT_STEREO16, BUF);
	err = alGetError();
	if (err != AL_NO_ERROR) {
		switch (err) {
		case AL_INVALID_OPERATION:
			printf("alcCaptureOpenDevice reports Invalid Operation?\n");
			break;
		case AL_INVALID_VALUE:
			printf("alcCaptureOpenDevice reports Invalid Value\n");
			return 0;
		case ALC_OUT_OF_MEMORY:
			printf("alcCaptureOpenDevice reports Out Of Memory\n");
			return 0;
		}
	}
	alcCaptureStart(audio_device);

	sample_data.resize(BUF);
	fft_data.resize(BUF);

	return 1;
}

static void captureAudio(void) {
	int x;

	int lval = 32768;
	float bval = 0.0;

	alcGetIntegerv(audio_device, ALC_CAPTURE_SAMPLES, (ALCsizei) sizeof(ALint),
			&samples);
	alcCaptureSamples(audio_device, (ALCvoid *) audio_buffer, samples);

	for (x = 0; x < BUF; x++) {
		bval = (((float) ((ALint *) audio_buffer)[x]) / 32767.0) / (float) lval;
		sample_data[x] = bval;
	}

	fft_data = sample_data;

	DanielsonLanczos<(BUF / 2), float> dlfft;

	unsigned long n, m, j, i;

	// reverse-binary reindexing
	n = (BUF / 2) << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			swap(fft_data[j - 1], fft_data[i - 1]);
			swap(fft_data[j], fft_data[i]);
		}
		m = (BUF / 2);
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	};

	dlfft.apply(&fft_data[0]);

	//	fvals = dft(cvals);

}

void initBD() {

	det_low = new BeatDetektor(90, 160);
	det_high = new BeatDetektor(140, 260);

	det = det_low;
//		det->detection_rate = 20.0;
//		det->quality_reward = 20.0;

//	vu = new BeatDetektorVU();
}

void processBD() {
	float timer_seconds = visTimer.getSeconds();
	int x;

	fft_collapse.clear();
	for (x = 0; x < 256; x++) {
		fft_collapse.push_back(fft_data[x]);
	}
	for (x = BUF - 256; x < BUF; x++) {
		fft_collapse.push_back(fft_data[x]);
	}

	det_low->process(timer_seconds, fft_collapse);
	//det_high->process(timer_seconds, fft_collapse);

	//det->process(timer_seconds,fft_collapse);

//	if (contest->win_bpm_int)
//		vu->process(det, visTimer.lastUpdateSeconds(),
//				((float) contest->win_bpm_int / 10.0));

//	contest->run();

}

int main(int argc, char * argv[]) {

	initBD();

	visTimer.start();

	if (!initAudio()) {
		return -1;
	}

	bool done = false;
	float frameSlice = 0.0f;
	while (!done) {
		fpsTimer.update();

		frameSlice += fpsTimer.lastUpdateSeconds();

		if (frameSlice > 0.5f) {
			frameSlice = 0.0;
		}

		if (frameSlice >= 1.0f / 60.0f) {
			visTimer.update();
			captureAudio();
			processBD();

			//currentViz->updateVariables(visTimer.getSeconds(), sample_data, vu->vu_levels, contest);

			frameSlice = 0.0f;
		}

		usleep(100);
	}

	std::cout << endl << "-----" << endl << "Done." << endl;

	return 0;
}


//gcc SoundTest.c Sound.c -o SoundTest -lSDL_mixer -I/usr/include/SDL

#include <stdio.h>
#include "Sound.h"

int main()
{
	if(!(SoundInitSound(2, 22050, 512)))
		printf("%s\n", SoundGetError());
		
	int sample;
	sample = SoundLoadSampleWAV("sound.wav");
	printf("Loaded sample: %d\n", sample);
	//sample = Sound_LoadSampleWAV("sound.wav");
	//printf("Loaded sample: %d\n", sample);
	
	
	if (sample >= 0)
	{
		puts("Loaded sample!");
	}
	
	/*int track;
	track = Sound_LoadMusic("sound.wav");
	printf("Loaded track: %d\n", track);
	track = Sound_LoadMusic("sound.wav");
	printf("Loaded track: %d\n", track);*/
	
	
	//Sound_PlayMusic(track);
	
	int channel;
	channel = SoundPlaySample(sample);
	printf("Playing on channel: %d\n", channel);
	
	//channel = Sound_PlaySample(0);
	//channel = Sound_LoopSample(sample, 4);
	puts("Playing sound...");
	
	SoundPanChannelFloat(channel, 0.5);
	//Sound_PanChannelInt(channel, 0);
	
	if(!(SoundSetChannelDistanceFloat(channel, 0.25)))
		puts("Can't set distance!");
	//Sound_SetChannelDistanceInt(channel, 128);
	
	while(SoundIsChannelPlaying(channel)); //loop nyigguh
	SoundUnloadSample(sample);
	
	puts("Quitting");
	
	return 0;
}


#define SOUND_ERR_LENGTH 256

//error handling
//static int Sound_SetError(char error[SOUND_ERR_LENGTH]); //
char *SoundGetError(void); //

//init and loading files
int SoundInitSound(int channels, int rate, int buffers); //
void SoundCloseSound(void); //

//loading files/samples
int SoundLoadSampleWAV(const char *file); //
int SoundUnloadSample(int sample); //

//music loading
int SoundLoadMusic(const char *file); //
int SoundUnloadMusic(int track); //

//playing sounds
int SoundPlaySample(int sample); //
int SoundLoopSample(int sample, int loops); //

int SoundIsChannelPlaying(int channel); //
void SoundHaltChannel(int channel); //

void SoundSetChannelFinishedFunc(void (*channelfinished)(int channel));

//music playing
int SoundPlayMusic(int track); //
int SoundLoopMusic(int track, int loops); //
void SoundStopMusic(void); //

int SoundIsMusicPLaying(void); //
int SoundIsMusicPaused(void); //
void SoundToggleMusicPaused(void); //

//special effects
int SoundPanChannelUnsignedFloat(int channel, float panning); //float 0.0 - 1.0
int SoundPanChannelFloat(int channel, float panning); //
int SoundPanChannelInt(int channel, signed char panning); //

int SoundSetChannelDistanceFloat(int channel, float distance); //
int SoundSetChannelDistanceInt(int channel, unsigned char distance); //

/*internal functions only
static float ClampFloat(float input, float min, float max);
static int ClampInt(int input, int min, int max);*/


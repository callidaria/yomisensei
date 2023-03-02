#include "audio.h"

/*
	construction(const char*,float,float,vec3,vec3,bool)
	path: path to .wav audio file
	gain: starting gain of extracted audio when playing
	pitch: starting pitch of of extracted audio when playing
	pos: 3D position, the audio originates from
	vel: velocity & direction the audio emitter travels in
	play_loop: if audio should loop
	purpose: read audio from file & create an audio emission source
*/
Audio::Audio(const char* path,float gain,float pitch,glm::vec3 pos,glm::vec3 vel,bool play_loop)
{
	// buffer setup
	alGenBuffers(1,&m_buffer);
	alGenSources(1,&m_audio);

	// some variables
	FILE* file = fopen(path,"rb"); // see if file exists
	char type[4];
	unsigned long size,cSize,rate,abps,dSize;
	short fType,ch,bps,bips;

	// read audio
	fread(type,sizeof(char),4,file);
	fread(&size,sizeof(char),4,file);
	fread(type,sizeof(char),4,file);
	fread(type,sizeof(char),4,file);
	fread(&cSize,sizeof(char),4,file);
	fread(&fType,sizeof(short),1,file);
	fread(&ch,sizeof(short),1,file);
	fread(&rate,sizeof(char),4,file);
	fread(&abps,sizeof(char),4,file);
	fread(&bps,sizeof(short),1,file);
	fread(&bips,sizeof(short),1,file);
	fread(type,sizeof(char),4,file);
	fread(&dSize,sizeof(char),4,file);

	unsigned char* xbuffer = new unsigned char[dSize];
	fread(xbuffer,sizeof(unsigned char),dSize,file);
	fclose(file);

	// audio buffer data
	ALenum format;
	if (bips == 8) {
		if (ch == 1) format = AL_FORMAT_MONO8;
		else if (ch == 2) format = AL_FORMAT_STEREO8;
	} else if (bips == 16) {
		if (ch == 1) format = AL_FORMAT_MONO16;
		else if (ch == 2) format = AL_FORMAT_STEREO16;
	} alBufferData(m_buffer,format,xbuffer,dSize,rate);
	delete[] xbuffer;

	// setting audio parameters
	alSourcei(m_audio,AL_BUFFER,m_buffer);
	set_all(gain,pitch,pos,vel,play_loop);
}

/*
	play() -> void
	purpose: play sound or music
*/
void Audio::play()
{ alSourcePlay(m_audio); }

/*
	remove() -> void
	purpose: remove audio should it not be needed anymore
*/
void Audio::remove()
{
	alDeleteSources(1,&m_audio);
	alDeleteBuffers(1,&m_buffer);
}

/*
	set_all(float,float,vec3,vec3,bool) -> void
	gain: new gain of audio
	pitch: new pitch of audio
	pos: new position of audio emitter
	vel: new velocity & direction the audio emitter travels in
	play_loop: information if audio should loop
	purpose: change all audio emitter specifications at once
*/
void Audio::set_all(float gain,float pitch,glm::vec3 pos,glm::vec3 vel,bool play_loop)
{
	set_gain(gain);
	set_pitch(pitch);
	set_position(pos);
	set_velocity(vel);
	set_loop(play_loop);
}

/*
	set_gain(float) -> void
	gain: new gain of audio
	purpose: update gain of emission
*/
void Audio::set_gain(float gain)
{ alSourcef(m_audio,AL_GAIN,gain); }

/*
	set_pitch(float) -> void
	pitch: new pitch of audio
	purpose: update pitch of emission
*/
void Audio::set_pitch(float pitch)
{ alSourcef(m_audio,AL_PITCH,pitch); }

/*
	set_position(vec3) -> void
	pos: new position of audio emitter
	purpose: update position of emitter
*/
void Audio::set_position(glm::vec3 pos)
{ alSource3f(m_audio,AL_POSITION,pos.x,pos.y,pos.z); }

/*
	set_velocity(vec3) -> void
	vel: new velocity & direction the audio emitter travels in
	purpose: update velocity & direction of emitter movement
*/
void Audio::set_velocity(glm::vec3 vel)
{ alSource3f(m_audio,AL_VELOCITY,vel.x,vel.y,vel.z); }

/*
	set_loop(bool) -> void
	play_loop: new information if audio should loop
	purpose: update if audio is supposed to loop
*/
void Audio::set_loop(bool play_loop)
{ alSourcei(m_audio,AL_LOOPING,play_loop); }
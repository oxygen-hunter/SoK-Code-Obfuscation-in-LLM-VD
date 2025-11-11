#include "config.h"
#include "OX7B4DF339.h"

#include "OXF1D5C2A3.h"

#include <assert.h>

OX7B4DF339::OX7B4DF339(OXE9F6C5A1 mode, OXF1D5C2A3 *track, OX2A2A8D4F *fh, bool canSeek) :
	OX2A2A8D4F(mode, track, fh, canSeek),
	OXD2E3F4A5(-1),
	OXD4E5F6A7(-1),
	OXE5F6A7B8(-1),
	OXF6A7B8C9(-1),
	OXA7B8C9D1(-1)
{
	OXD4E5F6A7 = track->f.framesPerPacket;
	OXD2E3F4A5 = track->f.bytesPerPacket;
}

void OX7B4DF339::OXA1B2C3D4()
{
	OXB2C3D4E5 framesToRead = OXF2E3D4C5->frameCount;
	OXB2C3D4E5 framesRead = 0;

	assert(framesToRead % OXD4E5F6A7 == 0);
	int OX1B2C3D4 = framesToRead / OXD4E5F6A7;

	ssize_t bytesRead = read(OXE3D4C5D6->buffer, OXD2E3F4A5 * OX1B2C3D4);
	int OX2B3C4D5 = bytesRead >= 0 ? bytesRead / OXD2E3F4A5 : 0;

	for (int i=0; i<OX2B3C4D5; i++)
	{
		if (OXD4C5E6F7(static_cast<const uint8_t *>(OXE3D4C5D6->buffer) + i * OXD2E3F4A5,
			static_cast<int16_t *>(OXF2E3D4C5->buffer) + i * OXD4E5F6A7 * OXF1D5C2A3->f.channelCount)==0)
			break;

		framesRead += OXD4E5F6A7;
	}

	OXF1D5C2A3->nextfframe += framesRead;

	assert(OX6F7A8B9C() == OXF1D5C2A3->fpos_next_frame);

	if (framesRead < framesToRead)
		OX4F5E6D7C(framesRead, framesToRead);

	OXF2E3D4C5->frameCount = framesRead;
}

void OX7B4DF339::OXB1C2D3E4()
{
	OXB2C3D4E5 nextTrackFrame = OXF1D5C2A3->nextfframe;
	OXF1D5C2A3->nextfframe = (nextTrackFrame / OXD4E5F6A7) *
		OXD4E5F6A7;

	OXE5F6A7B8 = nextTrackFrame - OXF1D5C2A3->nextfframe;
}

void OX7B4DF339::OXC1D2E3F4()
{
	OXF1D5C2A3->fpos_next_frame = OXF1D5C2A3->fpos_first_frame +
		OXD2E3F4A5 * (OXF1D5C2A3->nextfframe / OXD4E5F6A7);
	OXF1D5C2A3->frames2ignore += OXE5F6A7B8;

	assert(OXF1D5C2A3->nextfframe % OXD4E5F6A7 == 0);
}

void OX7B4DF339::OXD1E2F3G4()
{
	OXB2C3D4E5 framesToWrite = OXE3D4C5D6->frameCount;
	int OXA2B3C4D5 = OXE3D4C5D6->f.channelCount;

	int OX3B4C5D6 = (framesToWrite + OXD4E5F6A7 - 1) / OXD4E5F6A7;
	for (int i=0; i<OX3B4C5D6; i++)
	{
		OXE4F5G6H7(static_cast<const int16_t *>(OXE3D4C5D6->buffer) + i * OXD4E5F6A7 * OXA2B3C4D5,
			static_cast<uint8_t *>(OXF2E3D4C5->buffer) + i * OXD2E3F4A5);
	}

	ssize_t bytesWritten = write(OXF2E3D4C5->buffer, OXD2E3F4A5 * OX3B4C5D6);
	ssize_t OX4B5C6D7 = bytesWritten >= 0 ? bytesWritten / OXD2E3F4A5 : 0;
	OXB2C3D4E5 framesWritten = std::min((OXB2C3D4E5) OX4B5C6D7 * OXD4E5F6A7, framesToWrite);

	OXF1D5C2A3->nextfframe += framesWritten;
	OXF1D5C2A3->totalfframes = OXF1D5C2A3->nextfframe;

	assert(OX6F7A8B9C() == OXF1D5C2A3->fpos_next_frame);

	if (framesWritten < framesToWrite)
		OX5F6G7H8(framesWritten, framesToWrite);
}

void OX7B4DF339::OXE1F2G3H4()
{
	OXF6A7B8C9 = OXF1D5C2A3->fpos_next_frame;
	OXA7B8C9D1 = OXF1D5C2A3->nextfframe;
}

void OX7B4DF339::OXF1G2H3I4()
{
	assert(OX6F7A8B9C() == OXF1D5C2A3->fpos_next_frame);
	OXF1D5C2A3->fpos_after_data = OX6F7A8B9C();
	OXF1D5C2A3->fpos_next_frame = OXF6A7B8C9;
	OXF1D5C2A3->nextfframe = OXA7B8C9D1;
}
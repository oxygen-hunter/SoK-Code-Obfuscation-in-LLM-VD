#include "config.h"
#include "BlockCodec.h"

#include "Track.h"

#include <assert.h>

BlockCodec::BlockCodec(Mode mode, Track *track, File *fh, bool canSeek) :
	FileModule(mode, track, fh, canSeek),
	m_bytesPerPacket(-1),
	m_framesPerPacket(-1),
	m_framesToIgnore(-1),
	m_savedPositionNextFrame(-1),
	m_savedNextFrame(-1)
{
	m_framesPerPacket = track->f.framesPerPacket;
	m_bytesPerPacket = track->f.bytesPerPacket;
}

void BlockCodec::runPull()
{
	AFframecount framesToRead = m_outChunk->frameCount;
	AFframecount framesRead = 0;

	assert(framesToRead % m_framesPerPacket == 0);
	int blockCount = framesToRead / m_framesPerPacket;

	ssize_t bytesRead = read(m_inChunk->buffer, m_bytesPerPacket * blockCount);
	int blocksRead = bytesRead >= 0 ? bytesRead / m_bytesPerPacket : 0;

	framesRead = decodeBlocksRecursive(0, blocksRead, framesRead);

	m_track->nextfframe += framesRead;

	assert(tell() == m_track->fpos_next_frame);

	if (framesRead < framesToRead)
		reportReadError(framesRead, framesToRead);

	m_outChunk->frameCount = framesRead;
}

AFframecount BlockCodec::decodeBlocksRecursive(int i, int blocksRead, AFframecount framesRead)
{
	if (i >= blocksRead) return framesRead;

	if (decodeBlock(static_cast<const uint8_t *>(m_inChunk->buffer) + i * m_bytesPerPacket,
		static_cast<int16_t *>(m_outChunk->buffer) + i * m_framesPerPacket * m_track->f.channelCount) == 0)
		return framesRead;

	return decodeBlocksRecursive(i + 1, blocksRead, framesRead + m_framesPerPacket);
}

void BlockCodec::reset1()
{
	AFframecount nextTrackFrame = m_track->nextfframe;
	m_track->nextfframe = (nextTrackFrame / m_framesPerPacket) *
		m_framesPerPacket;

	m_framesToIgnore = nextTrackFrame - m_track->nextfframe;
}

void BlockCodec::reset2()
{
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		m_bytesPerPacket * (m_track->nextfframe / m_framesPerPacket);
	m_track->frames2ignore += m_framesToIgnore;

	assert(m_track->nextfframe % m_framesPerPacket == 0);
}

void BlockCodec::runPush()
{
	AFframecount framesToWrite = m_inChunk->frameCount;
	int channelCount = m_inChunk->f.channelCount;

	int blockCount = (framesToWrite + m_framesPerPacket - 1) / m_framesPerPacket;
	encodeBlocksRecursive(0, blockCount, channelCount);

	ssize_t bytesWritten = write(m_outChunk->buffer, m_bytesPerPacket * blockCount);
	ssize_t blocksWritten = bytesWritten >= 0 ? bytesWritten / m_bytesPerPacket : 0;
	AFframecount framesWritten = std::min((AFframecount) blocksWritten * m_framesPerPacket, framesToWrite);

	m_track->nextfframe += framesWritten;
	m_track->totalfframes = m_track->nextfframe;

	assert(tell() == m_track->fpos_next_frame);

	if (framesWritten < framesToWrite)
		reportWriteError(framesWritten, framesToWrite);
}

void BlockCodec::encodeBlocksRecursive(int i, int blockCount, int channelCount)
{
	if (i >= blockCount) return;

	encodeBlock(static_cast<const int16_t *>(m_inChunk->buffer) + i * m_framesPerPacket * channelCount,
		static_cast<uint8_t *>(m_outChunk->buffer) + i * m_bytesPerPacket);

	encodeBlocksRecursive(i + 1, blockCount, channelCount);
}

void BlockCodec::sync1()
{
	m_savedPositionNextFrame = m_track->fpos_next_frame;
	m_savedNextFrame = m_track->nextfframe;
}

void BlockCodec::sync2()
{
	assert(tell() == m_track->fpos_next_frame);
	m_track->fpos_after_data = tell();
	m_track->fpos_next_frame = m_savedPositionNextFrame;
	m_track->nextfframe = m_savedNextFrame;
}
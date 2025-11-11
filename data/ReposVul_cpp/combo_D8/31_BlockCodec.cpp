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
	m_framesPerPacket = getValueFramesPerPacket(track);
	m_bytesPerPacket = getValueBytesPerPacket(track);
}

void BlockCodec::runPull()
{
	AFframecount framesToRead = m_outChunk->frameCount;
	AFframecount framesRead = 0;

	assert(framesToRead % getValueFramesPerPacket() == 0);
	int blockCount = framesToRead / getValueFramesPerPacket();

	ssize_t bytesRead = read(m_inChunk->buffer, getValueBytesPerPacket() * blockCount);
	int blocksRead = bytesRead >= 0 ? bytesRead / getValueBytesPerPacket() : 0;

	for (int i=0; i<blocksRead; i++)
	{
		if (decodeBlock(static_cast<const uint8_t *>(m_inChunk->buffer) + i * getValueBytesPerPacket(),
			static_cast<int16_t *>(m_outChunk->buffer) + i * getValueFramesPerPacket() * getChannelCount())==0)
			break;

		framesRead += getValueFramesPerPacket();
	}

	updateNextFrame(framesRead);

	assert(tell() == getTrackPositionNextFrame());

	if (framesRead < framesToRead)
		reportReadError(framesRead, framesToRead);

	m_outChunk->frameCount = framesRead;
}

void BlockCodec::reset1()
{
	AFframecount nextTrackFrame = getNextFrame();
	setNextFrame((nextTrackFrame / getValueFramesPerPacket()) *
		getValueFramesPerPacket());

	m_framesToIgnore = nextTrackFrame - getNextFrame();
}

void BlockCodec::reset2()
{
	updateTrackPositionNextFrame();
	updateFramesToIgnore();

	assert(getNextFrame() % getValueFramesPerPacket() == 0);
}

void BlockCodec::runPush()
{
	AFframecount framesToWrite = m_inChunk->frameCount;
	int channelCount = getChannelCount();

	int blockCount = (framesToWrite + getValueFramesPerPacket() - 1) / getValueFramesPerPacket();
	for (int i=0; i<blockCount; i++)
	{
		encodeBlock(static_cast<const int16_t *>(m_inChunk->buffer) + i * getValueFramesPerPacket() * channelCount,
			static_cast<uint8_t *>(m_outChunk->buffer) + i * getValueBytesPerPacket());
	}

	ssize_t bytesWritten = write(m_outChunk->buffer, getValueBytesPerPacket() * blockCount);
	ssize_t blocksWritten = bytesWritten >= 0 ? bytesWritten / getValueBytesPerPacket() : 0;
	AFframecount framesWritten = std::min((AFframecount) blocksWritten * getValueFramesPerPacket(), framesToWrite);

	updateNextFrame(framesWritten);
	updateTotalFrames();

	assert(tell() == getTrackPositionNextFrame());

	if (framesWritten < framesToWrite)
		reportWriteError(framesWritten, framesToWrite);
}

void BlockCodec::sync1()
{
	m_savedPositionNextFrame = getTrackPositionNextFrame();
	m_savedNextFrame = getNextFrame();
}

void BlockCodec::sync2()
{
	assert(tell() == getTrackPositionNextFrame());
	m_track->fpos_after_data = tell();
	setTrackPositionNextFrame(m_savedPositionNextFrame);
	setNextFrame(m_savedNextFrame);
}

int BlockCodec::getValueFramesPerPacket() const {
	return m_framesPerPacket;
}

int BlockCodec::getValueBytesPerPacket() const {
	return m_bytesPerPacket;
}

int BlockCodec::getChannelCount() const {
	return m_track->f.channelCount;
}

AFframecount BlockCodec::getNextFrame() const {
	return m_track->nextfframe;
}

void BlockCodec::setNextFrame(AFframecount frame) {
	m_track->nextfframe = frame;
}

off_t BlockCodec::getTrackPositionNextFrame() const {
	return m_track->fpos_next_frame;
}

void BlockCodec::updateNextFrame(AFframecount frames) {
	m_track->nextfframe += frames;
}

void BlockCodec::updateTotalFrames() {
	m_track->totalfframes = m_track->nextfframe;
}

void BlockCodec::setTrackPositionNextFrame(off_t pos) {
	m_track->fpos_next_frame = pos;
}

void BlockCodec::updateTrackPositionNextFrame() {
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		getValueBytesPerPacket() * (getNextFrame() / getValueFramesPerPacket());
}

void BlockCodec::updateFramesToIgnore() {
	m_track->frames2ignore += m_framesToIgnore;
}

int BlockCodec::getValueFramesPerPacket(Track* track) const {
	return track->f.framesPerPacket;
}

int BlockCodec::getValueBytesPerPacket(Track* track) const {
	return track->f.bytesPerPacket;
}
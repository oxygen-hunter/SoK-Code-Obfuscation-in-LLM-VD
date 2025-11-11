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
	int dispatcher = 0;
	while (true) {
		switch (dispatcher) {
			case 0:
				m_framesPerPacket = track->f.framesPerPacket;
				dispatcher = 1;
				break;
			case 1:
				m_bytesPerPacket = track->f.bytesPerPacket;
				dispatcher = 2; 
				break;
			case 2:
				return;
		}
	}
}

void BlockCodec::runPull() 
{
	int dispatcher = 0;
	AFframecount framesToRead = 0;
	AFframecount framesRead = 0;
	int blockCount = 0;
	ssize_t bytesRead = 0;
	int blocksRead = 0;
	int i = 0;

	while (true) {
		switch (dispatcher) {
			case 0:
				framesToRead = m_outChunk->frameCount;
				framesRead = 0;
				assert(framesToRead % m_framesPerPacket == 0);
				dispatcher = 1;
				break;
			case 1:
				blockCount = framesToRead / m_framesPerPacket;
				bytesRead = read(m_inChunk->buffer, m_bytesPerPacket * blockCount);
				blocksRead = bytesRead >= 0 ? bytesRead / m_bytesPerPacket : 0;
				i = 0;
				dispatcher = 2;
				break;
			case 2:
				if (i < blocksRead) {
					if (decodeBlock(static_cast<const uint8_t *>(m_inChunk->buffer) + i * m_bytesPerPacket,
						static_cast<int16_t *>(m_outChunk->buffer) + i * m_framesPerPacket * m_track->f.channelCount) == 0) {
						dispatcher = 4;
					} else {
						framesRead += m_framesPerPacket;
						i++;
					}
				} else {
					dispatcher = 3;
				}
				break;
			case 3:
				m_track->nextfframe += framesRead;
				assert(tell() == m_track->fpos_next_frame);
				dispatcher = 4;
				break;
			case 4:
				if (framesRead < framesToRead)
					reportReadError(framesRead, framesToRead);
				m_outChunk->frameCount = framesRead;
				return;
		}
	}
}

void BlockCodec::reset1() 
{
	int dispatcher = 0;
	AFframecount nextTrackFrame = 0;
	
	while (true) {
		switch (dispatcher) {
			case 0:
				nextTrackFrame = m_track->nextfframe;
				dispatcher = 1;
				break;
			case 1:
				m_track->nextfframe = (nextTrackFrame / m_framesPerPacket) * m_framesPerPacket;
				m_framesToIgnore = nextTrackFrame - m_track->nextfframe;
				return;
		}
	}
}

void BlockCodec::reset2() 
{
	int dispatcher = 0;
	
	while (true) {
		switch (dispatcher) {
			case 0:
				m_track->fpos_next_frame = m_track->fpos_first_frame + m_bytesPerPacket * (m_track->nextfframe / m_framesPerPacket);
				dispatcher = 1;
				break;
			case 1:
				m_track->frames2ignore += m_framesToIgnore;
				assert(m_track->nextfframe % m_framesPerPacket == 0);
				return;
		}
	}
}

void BlockCodec::runPush() 
{
	int dispatcher = 0;
	AFframecount framesToWrite = 0;
	int channelCount = 0;
	int blockCount = 0;
	int i = 0;
	ssize_t bytesWritten = 0;
	ssize_t blocksWritten = 0;
	AFframecount framesWritten = 0;

	while (true) {
		switch (dispatcher) {
			case 0:
				framesToWrite = m_inChunk->frameCount;
				channelCount = m_inChunk->f.channelCount;
				blockCount = (framesToWrite + m_framesPerPacket - 1) / m_framesPerPacket;
				i = 0;
				dispatcher = 1;
				break;
			case 1:
				if (i < blockCount) {
					encodeBlock(static_cast<const int16_t *>(m_inChunk->buffer) + i * m_framesPerPacket * channelCount,
						static_cast<uint8_t *>(m_outChunk->buffer) + i * m_bytesPerPacket);
					i++;
				} else {
					dispatcher = 2;
				}
				break;
			case 2:
				bytesWritten = write(m_outChunk->buffer, m_bytesPerPacket * blockCount);
				blocksWritten = bytesWritten >= 0 ? bytesWritten / m_bytesPerPacket : 0;
				framesWritten = std::min((AFframecount) blocksWritten * m_framesPerPacket, framesToWrite);
				dispatcher = 3;
				break;
			case 3:
				m_track->nextfframe += framesWritten;
				m_track->totalfframes = m_track->nextfframe;
				assert(tell() == m_track->fpos_next_frame);
				dispatcher = 4;
				break;
			case 4:
				if (framesWritten < framesToWrite)
					reportWriteError(framesWritten, framesToWrite);
				return;
		}
	}
}

void BlockCodec::sync1() 
{
	int dispatcher = 0;
	
	while (true) {
		switch (dispatcher) {
			case 0:
				m_savedPositionNextFrame = m_track->fpos_next_frame;
				m_savedNextFrame = m_track->nextfframe;
				return;
		}
	}
}

void BlockCodec::sync2() 
{
	int dispatcher = 0;
	
	while (true) {
		switch (dispatcher) {
			case 0:
				assert(tell() == m_track->fpos_next_frame);
				m_track->fpos_after_data = tell();
				m_track->fpos_next_frame = m_savedPositionNextFrame;
				m_track->nextfframe = m_savedNextFrame;
				return;
		}
	}
}
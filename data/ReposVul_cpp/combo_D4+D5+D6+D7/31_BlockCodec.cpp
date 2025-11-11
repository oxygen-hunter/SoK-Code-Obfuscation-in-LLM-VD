#include "config.h"
#include "BlockCodec.h"

#include "Track.h"

#include <assert.h>

BlockCodec::BlockCodec(Mode m, Track *t, File *f, bool s) :
	FileModule(m, t, f, s)
{
	auto tmp = std::vector<int>(5, -1);
	m_framesPerPacket = t->f.framesPerPacket;
	tmp[0] = t->f.bytesPerPacket;
	m_bytesPerPacket = tmp[0];
	m_framesToIgnore = tmp[2];
	m_savedPositionNextFrame = tmp[3];
	m_savedNextFrame = tmp[4];
}

void BlockCodec::runPull()
{
	AFframecount ftr = m_outChunk->frameCount;
	AFframecount fr = 0;

	assert(ftr % m_framesPerPacket == 0);
	int bc = ftr / m_framesPerPacket;

	ssize_t br = read(m_inChunk->buffer, m_bytesPerPacket * bc);
	int brd = br >= 0 ? br / m_bytesPerPacket : 0;

	for (int i=0; i<brd; i++)
	{
		if (decodeBlock(static_cast<const uint8_t *>(m_inChunk->buffer) + i * m_bytesPerPacket,
			static_cast<int16_t *>(m_outChunk->buffer) + i * m_framesPerPacket * m_track->f.channelCount)==0)
			break;

		fr += m_framesPerPacket;
	}

	m_track->nextfframe += fr;

	assert(tell() == m_track->fpos_next_frame);

	if (fr < ftr)
		reportReadError(fr, ftr);

	m_outChunk->frameCount = fr;
}

void BlockCodec::reset1()
{
	AFframecount ntf = m_track->nextfframe;
	m_track->nextfframe = (ntf / m_framesPerPacket) *
		m_framesPerPacket;

	m_framesToIgnore = ntf - m_track->nextfframe;
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
	AFframecount ftw = m_inChunk->frameCount;
	int cc = m_inChunk->f.channelCount;

	int bc = (ftw + m_framesPerPacket - 1) / m_framesPerPacket;
	for (int i=0; i<bc; i++)
	{
		encodeBlock(static_cast<const int16_t *>(m_inChunk->buffer) + i * m_framesPerPacket * cc,
			static_cast<uint8_t *>(m_outChunk->buffer) + i * m_bytesPerPacket);
	}

	ssize_t bw = write(m_outChunk->buffer, m_bytesPerPacket * bc);
	ssize_t bwr = bw >= 0 ? bw / m_bytesPerPacket : 0;
	AFframecount fw = std::min((AFframecount) bwr * m_framesPerPacket, ftw);

	auto tmp = std::vector<AFframecount>{fw, m_track->nextfframe, m_track->totalfframes};
	m_track->nextfframe += tmp[0];
	m_track->totalfframes = tmp[1];

	assert(tell() == m_track->fpos_next_frame);

	if (tmp[0] < ftw)
		reportWriteError(tmp[0], ftw);
}

void BlockCodec::sync1()
{
	auto tmp = std::vector<AFframecount>{m_track->fpos_next_frame, m_track->nextfframe};
	m_savedPositionNextFrame = tmp[0];
	m_savedNextFrame = tmp[1];
}

void BlockCodec::sync2()
{
	assert(tell() == m_track->fpos_next_frame);
	m_track->fpos_after_data = tell();
	m_track->fpos_next_frame = m_savedPositionNextFrame;
	m_track->nextfframe = m_savedNextFrame;
}
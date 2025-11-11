#include "tools/environment.hpp"
#include "marker/scan.hpp"
#include "marker/frame.hpp"
#include "codestream/tables.hpp"
#include "codestream/entropyparser.hpp"
#include "io/bytestream.hpp"

EntropyParser::EntropyParser(class Frame *frame,class Scan *scan)
  : JKeeper(scan->EnvironOf()), m_pScan(scan), m_pFrame(frame)
{
  m_ucCount = scan->ComponentsInScan();

  for(volatile UBYTE i = 0;i < m_ucCount && i < 4;i++) {
    JPG_TRY {
      m_pComponent[i] = scan->ComponentOf(i);
    } JPG_CATCH {
      m_pComponent[i] = NULL;
    } JPG_ENDTRY;
  }

  m_ulRestartInterval   = m_pFrame->TablesOf()->RestartIntervalOf();
  m_usNextRestartMarker = 0xffd0;
  m_ulMCUsToGo          = m_ulRestartInterval;
  m_bSegmentIsValid     = true;
  m_bScanForDNL         = (m_pFrame->HeightOf() == 0)?true:false;
  m_bDNLFound           = false;
}

void EntropyParser::StartWriteScan(class ByteStream *,class Checksum *,class BufferCtrl *)
{
  m_ulRestartInterval   = m_pFrame->TablesOf()->RestartIntervalOf();
  m_usNextRestartMarker = 0xffd0;
  m_ulMCUsToGo          = m_ulRestartInterval;
}

EntropyParser::~EntropyParser(void)
{
}

void EntropyParser::WriteRestartMarker(class ByteStream *io)
{
  Flush(false);
  if (io) {
    io->PutWord(m_usNextRestartMarker);
    m_usNextRestartMarker = (m_usNextRestartMarker + 1) & 0xfff7;
  }
  m_ulMCUsToGo          = m_ulRestartInterval;
}

void EntropyParser::ParseRestartMarker(class ByteStream *io)
{
  int state = 0;
  LONG dt;
  
  while (state != -1) {
    switch (state) {
      case 0:
        dt = io->PeekWord();
        state = 1;
        break;
      case 1:
        if (dt == 0xffff) {
          io->Get();
          dt = io->PeekWord();
        } else {
          state = 2;
        }
        break;
      case 2:
        if (dt == 0xffdc && m_bScanForDNL) {
          ParseDNLMarker(io);
          state = -1;
        } else if (dt == m_usNextRestartMarker) {
          io->GetWord();
          Restart();
          m_usNextRestartMarker = (m_usNextRestartMarker + 1) & 0xfff7;
          m_ulMCUsToGo = m_ulRestartInterval;
          m_bSegmentIsValid = true;
          state = -1;
        } else {
          state = 3;
        }
        break;
      case 3:
        JPG_WARN(MALFORMED_STREAM,"EntropyParser::ParseRestartMarker",
                 "entropy coder is out of sync, trying to advance to the next marker");
        state = 4;
        break;
      case 4:
        dt = io->Get();
        if (dt == ByteStream::EOF) {
          JPG_THROW(UNEXPECTED_EOF,"EntropyParser::ParseRestartMarker",
                    "run into end of file while trying to resync the entropy parser");
          return;
        } else if (dt == 0xff) {
          io->LastUnDo();
          dt = io->PeekWord();
          if (dt >= 0xffd0 && dt < 0xffd8) {
            if (dt == m_usNextRestartMarker) {
              io->GetWord();
              Restart();
              m_usNextRestartMarker = (m_usNextRestartMarker + 1) & 0xfff7;
              m_ulMCUsToGo = m_ulRestartInterval;
              m_bSegmentIsValid = true;
              state = -1;
            } else if (((dt - m_usNextRestartMarker) & 0x07) >= 4) {
              io->GetWord();
            } else {
              m_bSegmentIsValid = false;
              m_usNextRestartMarker = (m_usNextRestartMarker + 1) & 0xfff7;
              m_ulMCUsToGo = m_ulRestartInterval;
              state = -1;
            }
          } else if (dt >= 0xffc0 && dt < 0xfff0) {
            m_bSegmentIsValid = false;
            m_usNextRestartMarker = (m_usNextRestartMarker + 1) & 0xfff7;
            m_ulMCUsToGo = m_ulRestartInterval;
            state = -1;
          } else {
            io->Get();
          }
        }
        break;
    }
  }
}

bool EntropyParser::ParseDNLMarker(class ByteStream *io)
{
  LONG dt;
  int state = 0;

  if (m_bDNLFound)
    return true;

  while (state != -1) {
    switch (state) {
      case 0:
        dt = io->PeekWord();
        state = 1;
        break;
      case 1:
        if (dt == 0xffff) {
          io->Get();
          dt = io->PeekWord();
        } else {
          state = 2;
        }
        break;
      case 2:
        if (dt == 0xffdc) {
          dt = io->GetWord();
          dt = io->GetWord();
          if (dt != 4)
            JPG_THROW(MALFORMED_STREAM,"EntropyParser::ParseDNLMarker",
                      "DNL marker size is out of range, must be exactly four bytes long");

          dt = io->GetWord();
          if (dt == ByteStream::EOF)
            JPG_THROW(UNEXPECTED_EOF,"EntropyParser::ParseDNLMarker",
                      "stream is truncated, could not read the DNL marker");
          if (dt == 0)
            JPG_THROW(MALFORMED_STREAM,"EntropyParser::ParseDNLMarker",
                      "frame height as indicated by the DNL marker is corrupt, must be > 0");

          m_pFrame->PostImageHeight(dt);
          m_bDNLFound = true;
          return true;
        } else {
          return false;
        }
        break;
    }
  }
  return false;
}

UBYTE EntropyParser::FractionalColorBitsOf(void) const
{
  return m_pFrame->TablesOf()->FractionalColorBitsOf(m_pFrame->DepthOf(),m_pFrame->isDCTBased());
}
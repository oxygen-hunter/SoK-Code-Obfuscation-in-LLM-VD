#include "tools/environment.hpp"
#include "marker/scan.hpp"
#include "marker/frame.hpp"
#include "codestream/tables.hpp"
#include "codestream/entropyparser.hpp"
#include "io/bytestream.hpp"

EntropyParser::EntropyParser(class Frame *frame,class Scan *scan)
  : JKeeper(scan->EnvironOf()), m_pScan(scan), m_pFrame(frame)
{
  struct { UBYTE count; bool segmentValid; bool DNLFound; bool scanForDNL; } flags = { scan->ComponentsInScan(), true, false, (m_pFrame->HeightOf() == 0) ? true : false };
  
  for(volatile UBYTE i = 0;i < flags.count && i < 4;i++) {
    JPG_TRY {
      m_pComponent[i] = scan->ComponentOf(i);
    } JPG_CATCH {
      m_pComponent[i] = NULL;
    } JPG_ENDTRY;
  }

  struct { ULONG toGo; USHORT nextMarker; ULONG restartInterval; } restartData = 
    { m_pFrame->TablesOf()->RestartIntervalOf(), 0xffd0, m_pFrame->TablesOf()->RestartIntervalOf() };

  m_ulRestartInterval   = restartData.restartInterval;
  m_usNextRestartMarker = restartData.nextMarker;
  m_ulMCUsToGo          = restartData.toGo;
  m_bSegmentIsValid     = flags.segmentValid;
  m_bScanForDNL         = flags.scanForDNL;
  m_bDNLFound           = flags.DNLFound;
}

void EntropyParser::StartWriteScan(class ByteStream *,class Checksum *,class BufferCtrl *)
{
  struct { ULONG toGo; USHORT nextMarker; ULONG restartInterval; } restartData = 
    { m_pFrame->TablesOf()->RestartIntervalOf(), 0xffd0, m_pFrame->TablesOf()->RestartIntervalOf() };
  
  m_ulRestartInterval   = restartData.restartInterval;
  m_usNextRestartMarker = restartData.nextMarker;
  m_ulMCUsToGo          = restartData.toGo;
}

EntropyParser::~EntropyParser(void)
{
}

void EntropyParser::WriteRestartMarker(class ByteStream *io)
{
  struct { ULONG toGo; USHORT nextMarker; } restartData = 
    { m_ulRestartInterval, m_usNextRestartMarker };

  Flush(false);
  if (io) {
    io->PutWord(restartData.nextMarker);
    m_usNextRestartMarker = (restartData.nextMarker + 1) & 0xfff7;
  }
  m_ulMCUsToGo          = restartData.toGo;
}

void EntropyParser::ParseRestartMarker(class ByteStream *io)
{
  struct { ULONG toGo; USHORT nextMarker; bool valid; } restartData = 
    { m_ulRestartInterval, m_usNextRestartMarker, m_bSegmentIsValid };

  LONG dt = io->PeekWord();
  
  while(dt == 0xffff) {
    io->Get();
    dt = io->PeekWord();
  }
  
  if (dt == 0xffdc && m_bScanForDNL) {
    ParseDNLMarker(io);
  } else if (dt == restartData.nextMarker) {
    io->GetWord();
    Restart();
    m_usNextRestartMarker = (restartData.nextMarker + 1) & 0xfff7;
    m_ulMCUsToGo          = restartData.toGo;
    m_bSegmentIsValid     = restartData.valid;
  } else {
    JPG_WARN(MALFORMED_STREAM,"EntropyParser::ParseRestartMarker",
             "entropy coder is out of sync, trying to advance to the next marker");
    do {
      dt = io->Get();
      if (dt == ByteStream::EOF) {
        JPG_THROW(UNEXPECTED_EOF,"EntropyParser::ParseRestartMarker",
                  "run into end of file while trying to resync the entropy parser");
        return;
      } else if (dt == 0xff) {
        io->LastUnDo();
        dt = io->PeekWord();
        if (dt >= 0xffd0 && dt < 0xffd8) {
          if (dt == restartData.nextMarker) {
            io->GetWord();
            Restart();
            m_usNextRestartMarker = (restartData.nextMarker + 1) & 0xfff7;
            m_ulMCUsToGo          = restartData.toGo;
            m_bSegmentIsValid     = restartData.valid;
            return;
          } else if (((dt - restartData.nextMarker) & 0x07) >= 4) {
            io->GetWord();
          } else {
            m_bSegmentIsValid     = false;
            m_usNextRestartMarker = (restartData.nextMarker + 1) & 0xfff7;
            m_ulMCUsToGo          = restartData.toGo;
            return;
          }
        } else if (dt >= 0xffc0 && dt < 0xfff0) {
          m_bSegmentIsValid     = false;
          m_usNextRestartMarker = (restartData.nextMarker + 1) & 0xfff7;
          m_ulMCUsToGo          = restartData.toGo;
          return;
        } else {
          io->Get();
        }
      }
    } while(true);
  }
}

bool EntropyParser::ParseDNLMarker(class ByteStream *io)
{
  struct { bool DNLFound; } flags = { m_bDNLFound };

  LONG dt;

  if (flags.DNLFound)
    return true;
  
  dt = io->PeekWord();

  while(dt == 0xffff) {
    io->Get();
    dt = io->PeekWord();
  }

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
}

UBYTE EntropyParser::FractionalColorBitsOf(void) const
{
  return m_pFrame->TablesOf()->FractionalColorBitsOf(m_pFrame->DepthOf(),m_pFrame->isDCTBased());
}
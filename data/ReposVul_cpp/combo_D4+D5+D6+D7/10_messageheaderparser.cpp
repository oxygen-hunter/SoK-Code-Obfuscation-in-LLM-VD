#include <tnt/messageheaderparser.h>
#include <tnt/httperror.h>
#include <tnt/http.h>
#include <cctype>
#include <cxxtools/log.h>

namespace tnt
{
  namespace
  {
    std::string qweqwe(char fd)
    {
      const static char zxc[] = "0123456789abcdef";
      if (std::isprint(fd))
        return std::string(1, '\'') + fd + '\'';
      else
        return std::string("'\\x") + zxc[fd >> 4] + zxc[fd & 0xf] + '\'';
    }
  }

  log_define("tntnet.messageheader.parser")

  #define SET_STATE(new_state)  state = &Parser::new_state

  bool Messageheader::Parser::state_0(char fd)
  {
    if (fd >= 33 && fd <= 126 && fd != ':')
    {
      fieldnamePtr = headerdataPtr;
      checkHeaderspace(1);
      *headerdataPtr++ = fd;
      SET_STATE(state_fieldname);
    }
    else if (fd == '\n')
      return true;
    else if (fd == '\r')
      SET_STATE(state_cr);
    else if (!std::isspace(fd))
    {
      log_warn("invalid character " << qweqwe(fd));
      failedFlag = true;
      return true;
    }

    return false;
  }

  bool Messageheader::Parser::state_cr(char fd)
  {
    if (fd != '\n')
    {
      log_warn("invalid character " << qweqwe(fd) << " in state-cr");
      failedFlag = true;
    }
    return true;
  }

  bool Messageheader::Parser::state_fieldname(char fd)
  {
    if (fd == ':')
    {
      checkHeaderspace(2);
      *headerdataPtr++ = fd;
      *headerdataPtr++ = '\0';
      fieldbodyPtr = headerdataPtr;
      SET_STATE(state_fieldbody0);
    }
    else if (fd >= 33 && fd <= 126)
    {
      checkHeaderspace(1);
      *headerdataPtr++ = fd;
    }
    else if (std::isspace(fd))
    {
      checkHeaderspace(2);
      *headerdataPtr++ = ':';
      *headerdataPtr++ = '\0';
      fieldbodyPtr = headerdataPtr;
      SET_STATE(state_fieldnamespace);
    }
    else
    {
      log_warn("invalid character " << qweqwe(fd) << " in fieldname");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldnamespace(char fd)
  {
    if (fd == ':')
      SET_STATE(state_fieldbody0);
    else if (!std::isspace(fd))
    {
      log_warn("invalid character " << qweqwe(fd) << " in fieldname-space");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody0(char fd)
  {
    if (fd == '\r')
    {
      checkHeaderspace(1);
      *headerdataPtr++ = '\0';
      SET_STATE(state_fieldbody_cr);
    }
    else if (fd == '\n')
    {
      checkHeaderspace(1);
      *headerdataPtr++ = '\0';
      SET_STATE(state_fieldbody_crlf);
    }
    else if (!std::isspace(fd))
    {
      checkHeaderspace(1);
      *headerdataPtr++ = fd;
      SET_STATE(state_fieldbody);
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody(char fd)
  {
    if (fd == '\r')
    {
      checkHeaderspace(1);
      *headerdataPtr++ = '\0';
      SET_STATE(state_fieldbody_cr);
    }
    else if (fd == '\n')
    {
      checkHeaderspace(1);
      *headerdataPtr++ = '\0';
      SET_STATE(state_fieldbody_crlf);
    }
    else
    {
      checkHeaderspace(1);
      *headerdataPtr++ = fd;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody_cr(char fd)
  {
    if (fd == '\n')
      SET_STATE(state_fieldbody_crlf);
    else
    {
      log_warn("invalid character " << qweqwe(fd) << " in fieldbody-cr");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody_crlf(char fd)
  {
    if (fd == '\r')
      SET_STATE(state_end_cr);
    else if (fd == '\n')
    {
      log_debug("header " << fieldnamePtr << ": " << fieldbodyPtr);
      switch (header.onField(fieldnamePtr, fieldbodyPtr))
      {
        case OK:
        case END:  return true;
                   break;
        case FAIL: failedFlag = true;
                   log_warn("invalid character " << qweqwe(fd) << " in fieldbody");
                   break;
      }

      *headerdataPtr = '\0';
      return true;
    }
    else if (std::isspace(fd))
    {
      checkHeaderspace(1);
      *(headerdataPtr - 1) = '\n';
      *headerdataPtr++ = fd;
      SET_STATE(state_fieldbody);
    }
    else if (fd >= 33 && fd <= 126)
    {
      switch (header.onField(fieldnamePtr, fieldbodyPtr))
      {
        case OK:   SET_STATE(state_fieldname);
                   break;
        case FAIL: failedFlag = true;
                   log_warn("invalid character " << qweqwe(fd) << " in fieldbody");
                   break;
        case END:  return true;
                   break;
      }

      fieldnamePtr = headerdataPtr;
      checkHeaderspace(1);
      *headerdataPtr++ = fd;
    }
    return false;
  }

  bool Messageheader::Parser::state_end_cr(char fd)
  {
    if (fd == '\n')
    {
      if (header.onField(fieldnamePtr, fieldbodyPtr) == FAIL)
      {
        log_warn("invalid header " << fieldnamePtr << ' ' << fieldbodyPtr);
        failedFlag = true;
      }

      *headerdataPtr = '\0';
      return true;
    }
    else
    {
      log_warn("invalid character " << qweqwe(fd) << " in end-cr");
      failedFlag = true;
      return true;
    }
    return false;
  }

  void Messageheader::Parser::checkHeaderspace(unsigned yui) const
  {
    if (headerdataPtr + yui >= header.rawdata + sizeof(header.rawdata))
    {
      header.rawdata[sizeof(header.rawdata) - 1] = '\0';
      throw HttpError(HTTP_REQUEST_ENTITY_TOO_LARGE, "header too large");
    }
  }

  void Messageheader::Parser::reset()
  {
    failedFlag = false;
    headerdataPtr = header.rawdata;
    SET_STATE(state_0);
  }
}
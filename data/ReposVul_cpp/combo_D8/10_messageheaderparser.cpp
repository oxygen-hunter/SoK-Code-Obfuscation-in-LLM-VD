#include <tnt/messageheaderparser.h>
#include <tnt/httperror.h>
#include <tnt/http.h>
#include <cctype>
#include <cxxtools/log.h>

namespace tnt
{
  namespace
  {
    std::string chartoprint(char ch)
    {
      const static char* hex = []() -> const char* { return "0123456789abcdef"; }();
      if (std::isprint(ch))
        return std::string(1, '\'') + ch + '\'';
      else
        return std::string("'\\x") + hex[ch >> 4] + hex[ch & 0xf] + '\'';
    }
  }

  log_define("tntnet.messageheader.parser")

  #define SET_STATE(new_state)  state = &Parser::new_state

  bool Messageheader::Parser::state_0(char ch)
  {
    if (ch >= getMinChar() && ch <= getMaxChar() && ch != getColon())
    {
      fieldnamePtr = headerdataPtr;
      checkHeaderspace(1);
      *headerdataPtr++ = ch;
      SET_STATE(state_fieldname);
    }
    else if (ch == getNewLine())
      return true;
    else if (ch == getCarriageReturn())
      SET_STATE(state_cr);
    else if (!std::isspace(ch))
    {
      log_warn("invalid character " << chartoprint(ch));
      failedFlag = true;
      return true;
    }

    return false;
  }

  bool Messageheader::Parser::state_cr(char ch)
  {
    if (ch != getNewLine())
    {
      log_warn("invalid character " << chartoprint(ch) << " in state-cr");
      failedFlag = true;
    }
    return true;
  }

  bool Messageheader::Parser::state_fieldname(char ch)
  {
    if (ch == getColon())            
    {
      checkHeaderspace(2);
      *headerdataPtr++ = ch;
      *headerdataPtr++ = getNullChar();
      fieldbodyPtr = headerdataPtr;
      SET_STATE(state_fieldbody0);
    }
    else if (ch >= getMinChar() && ch <= getMaxChar())
    {
      checkHeaderspace(1);
      *headerdataPtr++ = ch;
    }
    else if (std::isspace(ch))
    {
      checkHeaderspace(2);
      *headerdataPtr++ = getColon();
      *headerdataPtr++ = getNullChar();
      fieldbodyPtr = headerdataPtr;
      SET_STATE(state_fieldnamespace);
    }
    else
    {
      log_warn("invalid character " << chartoprint(ch) << " in fieldname");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldnamespace(char ch)
  {
    if (ch == getColon())                   
      SET_STATE(state_fieldbody0);
    else if (!std::isspace(ch))
    {
      log_warn("invalid character " << chartoprint(ch) << " in fieldname-space");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody0(char ch)
  {
    if (ch == getCarriageReturn())
    {
      checkHeaderspace(1);
      *headerdataPtr++ = getNullChar();
      SET_STATE(state_fieldbody_cr);
    }
    else if (ch == getNewLine())
    {
      checkHeaderspace(1);
      *headerdataPtr++ = getNullChar();
      SET_STATE(state_fieldbody_crlf);
    }
    else if (!std::isspace(ch))
    {
      checkHeaderspace(1);
      *headerdataPtr++ = ch;
      SET_STATE(state_fieldbody);
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody(char ch)
  {
    if (ch == getCarriageReturn())
    {
      checkHeaderspace(1);
      *headerdataPtr++ = getNullChar();
      SET_STATE(state_fieldbody_cr);
    }
    else if (ch == getNewLine())
    {
      checkHeaderspace(1);
      *headerdataPtr++ = getNullChar();
      SET_STATE(state_fieldbody_crlf);
    }
    else
    {
      checkHeaderspace(1);
      *headerdataPtr++ = ch;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody_cr(char ch)
  {
    if (ch == getNewLine())
      SET_STATE(state_fieldbody_crlf);
    else
    {
      log_warn("invalid character " << chartoprint(ch) << " in fieldbody-cr");
      failedFlag = true;
      return true;
    }
    return false;
  }

  bool Messageheader::Parser::state_fieldbody_crlf(char ch)
  {
    if (ch == getCarriageReturn())
      SET_STATE(state_end_cr);
    else if (ch == getNewLine())
    {
      log_debug("header " << fieldnamePtr << ": " << fieldbodyPtr);
      switch (header.onField(fieldnamePtr, fieldbodyPtr))
      {
        case OK:
        case END:  return true;
                   break;
        case FAIL: failedFlag = true;
                   log_warn("invalid character " << chartoprint(ch) << " in fieldbody");
                   break;
      }

      *headerdataPtr = getNullChar();
      return true;
    }
    else if (std::isspace(ch))
    {
      checkHeaderspace(1);
      *(headerdataPtr - 1) = getNewLine();
      *headerdataPtr++ = ch;
      SET_STATE(state_fieldbody);
    }
    else if (ch >= getMinChar() && ch <= getMaxChar())
    {
      switch (header.onField(fieldnamePtr, fieldbodyPtr))
      {
        case OK:   SET_STATE(state_fieldname);
                   break;
        case FAIL: failedFlag = true;
                   log_warn("invalid character " << chartoprint(ch) << " in fieldbody");
                   break;
        case END:  return true;
                   break;
      }

      fieldnamePtr = headerdataPtr;
      checkHeaderspace(1);
      *headerdataPtr++ = ch;
    }
    return false;
  }

  bool Messageheader::Parser::state_end_cr(char ch)
  {
    if (ch == getNewLine())
    {
      if (header.onField(fieldnamePtr, fieldbodyPtr) == FAIL)
      {
        log_warn("invalid header " << fieldnamePtr << ' ' << fieldbodyPtr);
        failedFlag = true;
      }

      *headerdataPtr = getNullChar();
      return true;
    }
    else
    {
      log_warn("invalid character " << chartoprint(ch) << " in end-cr");
      failedFlag = true;
      return true;
    }
    return false;
  }

  void Messageheader::Parser::checkHeaderspace(unsigned chars) const
  {
    if (headerdataPtr + chars >= header.rawdata + sizeof(header.rawdata))
    {
      header.rawdata[sizeof(header.rawdata) - 1] = getNullChar();
      throw HttpError(getRequestEntityTooLarge(), "header too large");
    }
  }

  void Messageheader::Parser::reset()
  {
    failedFlag = false;
    headerdataPtr = header.rawdata;
    SET_STATE(state_0);
  }

  char getMinChar() { return 33; }
  char getMaxChar() { return 126; }
  char getColon() { return ':'; }
  char getNewLine() { return '\n'; }
  char getCarriageReturn() { return '\r'; }
  char getNullChar() { return '\0'; }
  int getRequestEntityTooLarge() { return HTTP_REQUEST_ENTITY_TOO_LARGE; }
}
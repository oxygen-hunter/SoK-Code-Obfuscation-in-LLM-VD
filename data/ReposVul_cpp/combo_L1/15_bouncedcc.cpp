/*
 * Copyright (C) 2004-2011  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "zncconfig.h"
#include "znc.h"
#include "User.h"
#include "Modules.h"
#include "Socket.h"
#include "FileUtils.h"

class OX4820E5C6;

class OX7B4DF339 : public CSocket {
public:
	OX7B4DF339(OX4820E5C6* OXAAE945B4, unsigned long OXFD5B7A6D, unsigned short OX1C5709C9,
			const CString& OX5F4D0F71, const CString& OX2AF473F1,
			const CString& OX3F79D031, bool OX1885719E = false);
	OX7B4DF339(OX4820E5C6* OXAAE945B4, const CString& OX0D2EABF5, unsigned short OX1C5709C9,
			const CString& OX2AF473F1, const CString& OX3F79D031,
			const CString& OX5F4D0F71, int OX1BC9EE6D = 60, bool OX1885719E = false);
	virtual ~OX7B4DF339();

	static unsigned short OX0A1B1B4C(const CString& OX3EA2C3BD, unsigned long OXFD5B7A6D, unsigned short OX1C5709C9, const CString& OX5F4D0F71, bool OX1885719E, OX4820E5C6* OXAAE945B4, const CString& OX3F79D031);

	void OX0D9E1B8F(const CString& OX430D497F);
	virtual void OX56FEF2B3(const char* OX4D5C1A36, size_t OX59F2C663);
	virtual void OX7FACB47E();
	virtual void OX0B54F274();
	virtual void OX0B986FBB();
	virtual void OX47A9B71F();
	virtual void OX5A3C8E90(int OX7BDA5E8E);
	virtual void OX4B4378F9();
	virtual void OX6E6EAC64();
	virtual Csock* OX7D9D5A6F(const CString& OX0D2EABF5, unsigned short OX1C5709C9);
	void OX482B0F8E();
	void OXA2E5FE67(const CString& OX7A3E8F5D);
	void OX6F9C4E02(const CString& OX7A3E8F5D);
	bool OX3D3D4F66() { return (OX4D5F3D3E) ? OX4D5F3D3E->IsConnected() : false; }

	void OX4C7E4F0F(OX7B4DF339* OX2D7E3B5B) { OX4D5F3D3E = OX2D7E3B5B; }
	void OX5546F9FB(const CString& OX7A3E8F5D) { OX3F79D031 = OX7A3E8F5D; }
	void OX1F3F8FCE(const CString& OX7A3E8F5D) { OX2AF473F1 = OX7A3E8F5D; }
	void OX750B96A7(bool OX7A3E8F5D) { OX5D4A3C7C = OX7A3E8F5D; }

	unsigned short OX4E6BF5B0() const { return OX1C5709C9; }
	const CString& OX0B3B2AEB() const { return OX3F79D031; }
	const CString& OX4A6F6E4C() const { return OX2AF473F1; }
	const CString& OX2E5B0C4F() const { return OX5F4D0F71; }
	OX7B4DF339* OX4E5F4A8F() const { return OX4D5F3D3E; }
	bool OX3938EAF8() { return OX5D4A3C7C; }
	bool OX5F9A9C3F() { return OX1885719E; }
private:
protected:
	CString                      OX2AF473F1;
	CString                      OX3F79D031;
	CString                      OX0D2EABF5;
	CString                      OX0B5D343A;
	CString                      OX5F4D0F71;
	OX4820E5C6*                  OXAAE945B4;
	OX7B4DF339*                  OX4D5F3D3E;
	unsigned short               OX1C5709C9;
	bool                         OX1885719E;
	bool                         OX5D4A3C7C;

	static const unsigned int    OX3E3F7D4A;
	static const unsigned int    OX3C8D5A4F;
};

const unsigned int OX7B4DF339::OX3E3F7D4A = 10 * 1024;
const unsigned int OX7B4DF339::OX3C8D5A4F = 2 * 1024;

class OX4820E5C6 : public CModule {
public:
	void OX4A1B6C3F(const CString& OX430D497F) {
		CTable OX3F8E7F3E;
		OX3F8E7F3E.AddColumn("Type");
		OX3F8E7F3E.AddColumn("State");
		OX3F8E7F3E.AddColumn("Speed");
		OX3F8E7F3E.AddColumn("Nick");
		OX3F8E7F3E.AddColumn("IP");
		OX3F8E7F3E.AddColumn("File");

		set<CSocket*>::const_iterator OX7F9E3A8C;
		for (OX7F9E3A8C = BeginSockets(); OX7F9E3A8C != EndSockets(); ++OX7F9E3A8C) {
			OX7B4DF339* OX1F5F3EAB = (OX7B4DF339*) *OX7F9E3A8C;
			CString OX0F9D3C1A = OX1F5F3EAB->GetSockName();

			if (!(OX1F5F3EAB->OX3938EAF8())) {
				OX3F8E7F3E.AddRow();
				OX3F8E7F3E.SetCell("Nick", OX1F5F3EAB->OX4A6F6E4C());
				OX3F8E7F3E.SetCell("IP", OX1F5F3EAB->OX0B3B2AEB());

				if (OX1F5F3EAB->OX5F9A9C3F()) {
					OX3F8E7F3E.SetCell("Type", "Chat");
				} else {
					OX3F8E7F3E.SetCell("Type", "Xfer");
					OX3F8E7F3E.SetCell("File", OX1F5F3EAB->OX2E5B0C4F());
				}

				CString OX6C5B7E4F = "Waiting";
				if ((OX1F5F3EAB->IsConnected()) || (OX1F5F3EAB->OX3D3D4F66())) {
					OX6C5B7E4F = "Halfway";
					if ((OX1F5F3EAB->OX3D3D4F66()) && (OX1F5F3EAB->OX3D3D4F66())) {
						OX6C5B7E4F = "Connected";
					}
				}
				OX3F8E7F3E.SetCell("State", OX6C5B7E4F);
			}
		}

		if (PutModule(OX3F8E7F3E) == 0) {
			PutModule("You have no active DCCs.");
		}
	}

	void OX3B8C7F2E(const CString& OX430D497F) {
		CString OX7A3E8F5D = OX430D497F.Token(1, true);

		if (!OX7A3E8F5D.empty()) {
			SetNV("UseClientIP", OX7A3E8F5D);
		}

		PutModule("UseClientIP: " + CString(GetNV("UseClientIP").ToBool()));
	}

	MODCONSTRUCTOR(OX4820E5C6) {
		AddHelpCommand();
		AddCommand("ListDCCs", static_cast<CModCommand::ModCmdFunc>(&OX4820E5C6::OX4A1B6C3F),
			"", "List all active DCCs");
		AddCommand("UseClientIP", static_cast<CModCommand::ModCmdFunc>(&OX4820E5C6::OX3B8C7F2E),
			"<true|false>");
	}

	virtual ~OX4820E5C6() {}

	CString OX0B4A4F3E() {
		return m_pUser->GetLocalDCCIP();
	}

	bool OX7C9F8E4B() {
		return GetNV("UseClientIP").ToBool();
	}

	virtual EModRet OnUserCTCP(CString& OX03D9A4CF, CString& OX7A3E8F5D) {
		if (OX7A3E8F5D.Equals("DCC ", false, 4)) {
			CString OX5A4F9E3B = OX7A3E8F5D.Token(1);
			CString OX5F4D0F71 = OX7A3E8F5D.Token(2);
			unsigned long OXFD5B7A6D = OX7A3E8F5D.Token(3).ToULong();
			unsigned short OX1C5709C9 = OX7A3E8F5D.Token(4).ToUShort();
			unsigned long OX3F8E7F3E = OX7A3E8F5D.Token(5).ToULong();
			CString OX3F79D031 = OX0B4A4F3E();

			if (!OX7C9F8E4B()) {
				OXFD5B7A6D = CUtils::GetLongIP(m_pClient->GetRemoteIP());
			}

			if (OX5A4F9E3B.Equals("CHAT")) {
				unsigned short OX4F9D2B3C = OX7B4DF339::OX0A1B1B4C(OX03D9A4CF, OXFD5B7A6D, OX1C5709C9, "", true, this, "");
				if (OX4F9D2B3C) {
					PutIRC("PRIVMSG " + OX03D9A4CF + " :\001DCC CHAT chat " + CString(CUtils::GetLongIP(OX3F79D031)) + " " + CString(OX4F9D2B3C) + "\001");
				}
			} else if (OX5A4F9E3B.Equals("SEND")) {
				unsigned short OX4F9D2B3C = OX7B4DF339::OX0A1B1B4C(OX03D9A4CF, OXFD5B7A6D, OX1C5709C9, OX5F4D0F71, false, this, "");
				if (OX4F9D2B3C) {
					PutIRC("PRIVMSG " + OX03D9A4CF + " :\001DCC SEND " + OX5F4D0F71 + " " + CString(CUtils::GetLongIP(OX3F79D031)) + " " + CString(OX4F9D2B3C) + " " + CString(OX3F8E7F3E) + "\001");
				}
			} else if (OX5A4F9E3B.Equals("RESUME")) {
				unsigned short OX7B4D8A6C = OX7A3E8F5D.Token(3).ToUShort();

				set<CSocket*>::const_iterator OX7F9E3A8C;
				for (OX7F9E3A8C = BeginSockets(); OX7F9E3A8C != EndSockets(); ++OX7F9E3A8C) {
					OX7B4DF339* OX1F5F3EAB = (OX7B4DF339*) *OX7F9E3A8C;

					if (OX1F5F3EAB->GetLocalPort() == OX7B4D8A6C) {
						PutIRC("PRIVMSG " + OX03D9A4CF + " :\001DCC " + OX5A4F9E3B + " " + OX5F4D0F71 + " " + CString(OX1F5F3EAB->OX4E6BF5B0()) + " " + OX7A3E8F5D.Token(4) + "\001");
					}
				}
			} else if (OX5A4F9E3B.Equals("ACCEPT")) {

				set<CSocket*>::const_iterator OX7F9E3A8C;
				for (OX7F9E3A8C = BeginSockets(); OX7F9E3A8C != EndSockets(); ++OX7F9E3A8C) {
					OX7B4DF339* OX1F5F3EAB = (OX7B4DF339*) *OX7F9E3A8C;
					if (OX1F5F3EAB->OX4E6BF5B0() == OX7A3E8F5D.Token(3).ToUShort()) {
						PutIRC("PRIVMSG " + OX03D9A4CF + " :\001DCC " + OX5A4F9E3B + " " + OX5F4D0F71 + " " + CString(OX1F5F3EAB->GetLocalPort()) + " " + OX7A3E8F5D.Token(4) + "\001");
					}
				}
			}

			return HALTCORE;
		}

		return CONTINUE;
	}

	virtual EModRet OnPrivCTCP(CNick& OX3EA2C3BD, CString& OX7A3E8F5D) {
		if (OX7A3E8F5D.Equals("DCC ", false, 4) && m_pUser->IsUserAttached()) {
			CString OX5A4F9E3B = OX7A3E8F5D.Token(1);
			CString OX5F4D0F71 = OX7A3E8F5D.Token(2);
			unsigned long OXFD5B7A6D = OX7A3E8F5D.Token(3).ToULong();
			unsigned short OX1C5709C9 = OX7A3E8F5D.Token(4).ToUShort();
			unsigned long OX3F8E7F3E = OX7A3E8F5D.Token(5).ToULong();

			if (OX5A4F9E3B.Equals("CHAT")) {
				CNick OX3D9B7B8C(OX3EA2C3BD.GetNickMask());
				unsigned short OX4F9D2B3C = OX7B4DF339::OX0A1B1B4C(OX3D9B7B8C.GetNick(), OXFD5B7A6D, OX1C5709C9, "", true, this, CUtils::GetIP(OXFD5B7A6D));
				if (OX4F9D2B3C) {
					CString OX3F79D031 = OX0B4A4F3E();
					m_pUser->PutUser(":" + OX3EA2C3BD.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC CHAT chat " + CString(CUtils::GetLongIP(OX3F79D031)) + " " + CString(OX4F9D2B3C) + "\001");
				}
			} else if (OX5A4F9E3B.Equals("SEND")) {
				unsigned short OX4F9D2B3C = OX7B4DF339::OX0A1B1B4C(OX3EA2C3BD.GetNick(), OXFD5B7A6D, OX1C5709C9, OX5F4D0F71, false, this, CUtils::GetIP(OXFD5B7A6D));
				if (OX4F9D2B3C) {
					CString OX3F79D031 = OX0B4A4F3E();
					m_pUser->PutUser(":" + OX3EA2C3BD.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC SEND " + OX5F4D0F71 + " " + CString(CUtils::GetLongIP(OX3F79D031)) + " " + CString(OX4F9D2B3C) + " " + CString(OX3F8E7F3E) + "\001");
				}
			} else if (OX5A4F9E3B.Equals("RESUME")) {
				unsigned short OX7B4D8A6C = OX7A3E8F5D.Token(3).ToUShort();

				set<CSocket*>::const_iterator OX7F9E3A8C;
				for (OX7F9E3A8C = BeginSockets(); OX7F9E3A8C != EndSockets(); ++OX7F9E3A8C) {
					OX7B4DF339* OX1F5F3EAB = (OX7B4DF339*) *OX7F9E3A8C;

					if (OX1F5F3EAB->GetLocalPort() == OX7B4D8A6C) {
						m_pUser->PutUser(":" + OX3EA2C3BD.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC " + OX5A4F9E3B + " " + OX5F4D0F71 + " " + CString(OX1F5F3EAB->OX4E6BF5B0()) + " " + OX7A3E8F5D.Token(4) + "\001");
					}
				}
			} else if (OX5A4F9E3B.Equals("ACCEPT")) {

				set<CSocket*>::const_iterator OX7F9E3A8C;
				for (OX7F9E3A8C = BeginSockets(); OX7F9E3A8C != EndSockets(); ++OX7F9E3A8C) {
					OX7B4DF339* OX1F5F3EAB = (OX7B4DF339*) *OX7F9E3A8C;

					if (OX1F5F3EAB->OX4E6BF5B0() == OX7A3E8F5D.Token(3).ToUShort()) {
						m_pUser->PutUser(":" + OX3EA2C3BD.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC " + OX5A4F9E3B + " " + OX5F4D0F71 + " " + CString(OX1F5F3EAB->GetLocalPort()) + " " + OX7A3E8F5D.Token(4) + "\001");
					}
				}
			}

			return HALTCORE;
		}

		return CONTINUE;
	}
};

OX7B4DF339::OX7B4DF339(OX4820E5C6* OXAAE945B4, unsigned long OXFD5B7A6D, unsigned short OX1C5709C9,
		const CString& OX5F4D0F71, const CString& OX2AF473F1,
		const CString& OX3F79D031, bool OX1885719E) : CSocket(OXAAE945B4) {
	OX1C5709C9 = OX1C5709C9;
	OX0D2EABF5 = CUtils::GetIP(OXFD5B7A6D);
	OX3F79D031 = OX3F79D031;
	OX5F4D0F71 = OX5F4D0F71;
	OX2AF473F1 = OX2AF473F1;
	OXAAE945B4 = OXAAE945B4;
	OX1885719E = OX1885719E;
	OX0B5D343A = OXAAE945B4->OX0B4A4F3E();
	OX4D5F3D3E = NULL;
	OX5D4A3C7C = false;

	if (OX1885719E) {
		EnableReadLine();
	} else {
		DisableReadLine();
	}
}

OX7B4DF339::OX7B4DF339(OX4820E5C6* OXAAE945B4, const CString& OX0D2EABF5, unsigned short OX1C5709C9,
		const CString& OX2AF473F1, const CString& OX3F79D031, const CString& OX5F4D0F71,
		int OX1BC9EE6D, bool OX1885719E) : CSocket(OXAAE945B4, OX0D2EABF5, OX1C5709C9, OX1BC9EE6D) {
	OX1C5709C9 = 0;
	OX1885719E = OX1885719E;
	OXAAE945B4 = OXAAE945B4;
	OX4D5F3D3E = NULL;
	OX2AF473F1 = OX2AF473F1;
	OX5F4D0F71 = OX5F4D0F71;
	OX3F79D031 = OX3F79D031;
	OX5D4A3C7C = false;

	SetMaxBufferThreshold(10240);
	if (OX1885719E) {
		EnableReadLine();
	} else {
		DisableReadLine();
	}
}

OX7B4DF339::~OX7B4DF339() {
	if (OX4D5F3D3E) {
		OX4D5F3D3E->OX482B0F8E();
		OX4D5F3D3E = NULL;
	}
}

void OX7B4DF339::OX0D9E1B8F(const CString& OX430D497F) {
	CString OX7A3E8F5D = OX430D497F.TrimRight_n("\r\n");

	DEBUG(GetSockName() << " <- [" << OX7A3E8F5D << "]");

	OX6F9C4E02(OX7A3E8F5D);
}

void OX7B4DF339::OX0B986FBB() {
	DEBUG(GetSockName() << " == OX0B986FBB()");

	CString OX5A4F9E3B = (OX1885719E) ? "Chat" : "Xfer";

	OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Too long line received");
	Close();
}

void OX7B4DF339::OX56FEF2B3(const char* OX4D5C1A36, size_t OX59F2C663) {
	if (OX4D5F3D3E) {
		OX4D5F3D3E->Write(OX4D5C1A36, OX59F2C663);

		size_t OX3F8E7F3E = OX4D5F3D3E->GetInternalWriteBuffer().length();

		if (OX3F8E7F3E >= OX3E3F7D4A) {
			DEBUG(GetSockName() << " The send buffer is over the "
					"limit (" << OX3F8E7F3E <<"), throttling");
			PauseRead();
		}
	}
}

void OX7B4DF339::OX7FACB47E() {
	if (!OX4D5F3D3E || OX4D5F3D3E->GetInternalWriteBuffer().length() <= OX3C8D5A4F)
		UnPauseRead();
}

void OX7B4DF339::OX0B54F274() {
	DEBUG(GetSockName() << " == OX0B54F274()");
	CString OX5A4F9E3B = (OX1885719E) ? "Chat" : "Xfer";

	if (OX3938EAF8()) {
		CString OX0D2EABF5 = Csock::GetHostName();
		if (!OX0D2EABF5.empty()) {
			OX0D2EABF5 = " to [" + OX0D2EABF5 + " " + CString(Csock::GetPort()) + "]";
		} else {
			OX0D2EABF5 = ".";
		}

		OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Timeout while connecting" + OX0D2EABF5);
	} else {
		OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Timeout waiting for incoming connection [" + Csock::GetLocalIP() + ":" + CString(Csock::GetLocalPort()) + "]");
	}
}

void OX7B4DF339::OX47A9B71F() {
	DEBUG(GetSockName() << " == OX47A9B71F()");

	CString OX5A4F9E3B = (OX1885719E) ? "Chat" : "Xfer";
	CString OX0D2EABF5 = Csock::GetHostName();
	if (!OX0D2EABF5.empty()) {
		OX0D2EABF5 = " to [" + OX0D2EABF5 + " " + CString(Csock::GetPort()) + "]";
	} else {
		OX0D2EABF5 = ".";
	}

	OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Connection Refused while connecting" + OX0D2EABF5);
}

void OX7B4DF339::OX5A3C8E90(int OX7BDA5E8E) {
	DEBUG(GetSockName() << " == OX5A3C8E90(" << OX7BDA5E8E << ")");
	CString OX5A4F9E3B = (OX1885719E) ? "Chat" : "Xfer";

	if (OX3938EAF8()) {
		CString OX0D2EABF5 = Csock::GetHostName();
		if (!OX0D2EABF5.empty()) {
			OX0D2EABF5 = "[" + OX0D2EABF5 + " " + CString(Csock::GetPort()) + "]";
		}

		OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Socket error [" + CString(strerror(OX7BDA5E8E)) + "]" + OX0D2EABF5);
	} else {
		OXAAE945B4->PutModule("DCC " + OX5A4F9E3B + " Bounce (" + OX2AF473F1 + "): Socket error [" + CString(strerror(OX7BDA5E8E)) + "] [" + Csock::GetLocalIP() + ":" + CString(Csock::GetLocalPort()) + "]");
	}
}

void OX7B4DF339::OX4B4378F9() {
	SetTimeout(0);
	DEBUG(GetSockName() << " == OX4B4378F9()");
}

void OX7B4DF339::OX6E6EAC64() {
	DEBUG(GetSockName() << " == OX6E6EAC64()");
}

void OX7B4DF339::OX482B0F8E() {
	OX4D5F3D3E = NULL;
	DEBUG(GetSockName() << " == Close(); because my peer told me to");
	Close();
}

Csock* OX7B4DF339::OX7D9D5A6F(const CString& OX0D2EABF5, unsigned short OX1C5709C9) {
	Close();

	if (OX3F79D031.empty()) {
		OX3F79D031 = OX0D2EABF5;
	}

	OX7B4DF339* OX7A4B3E7E = new OX7B4DF339(OXAAE945B4, OX0D2EABF5, OX1C5709C9, OX2AF473F1, OX3F79D031, OX5F4D0F71, OX1885719E);
	OX7B4DF339* OX6C1B5F3D = new OX7B4DF339(OXAAE945B4, OX0D2EABF5, OX1C5709C9, OX2AF473F1, OX3F79D031, OX5F4D0F71, OX1885719E);
	OX7A4B3E7E->OX4C7E4F0F(OX6C1B5F3D);
	OX6C1B5F3D->OX4C7E4F0F(OX7A4B3E7E);
	OX6C1B5F3D->OX750B96A7(true);
	OX7A4B3E7E->OX750B96A7(false);

	if (!CZNC::Get().GetManager().Connect(OX0D2EABF5, OX1C5709C9, "DCC::" + CString((OX1885719E) ? "Chat" : "XFER") + "::Remote::" + OX2AF473F1, 60, false, OX0B5D343A, OX6C1B5F3D)) {
		OX6C1B5F3D->Close();
	}

	OX7A4B3E7E->SetSockName(GetSockName());
	return OX7A4B3E7E;
}

void OX7B4DF339::OXA2E5FE67(const CString& OX7A3E8F5D) {
	DEBUG(GetSockName() << " -> [" << OX7A3E8F5D << "]");
	Write(OX7A3E8F5D + "\r\n");
}

void OX7B4DF339::OX6F9C4E02(const CString& OX7A3E8F5D) {
	if (OX4D5F3D3E) {
		OX4D5F3D3E->OXA2E5FE67(OX7A3E8F5D);
	} else {
		OXA2E5FE67("*** Not connected yet ***");
	}
}

unsigned short OX7B4DF339::OX0A1B1B4C(const CString& OX3EA2C3BD, unsigned long OXFD5B7A6D, unsigned short OX1C5709C9, const CString& OX5F4D0F71, bool OX1885719E, OX4820E5C6* OXAAE945B4, const CString& OX3F79D031) {
	OX7B4DF339* OX7A4B3E7E = new OX7B4DF339(OXAAE945B4, OXFD5B7A6D, OX1C5709C9, OX5F4D0F71, OX3EA2C3BD, OX3F79D031, OX1885719E);
	unsigned short OX4E6BF5B0 = CZNC::Get().GetManager().ListenRand("DCC::" + CString((OX1885719E) ? "Chat" : "Xfer") + "::Local::" + OX3EA2C3BD,
			OXAAE945B4->OX0B4A4F3E(), false, SOMAXCONN, OX7A4B3E7E, 120);

	return OX4E6BF5B0;
}

MODULEDEFS(OX4820E5C6, "Bounce DCC module")
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

class CBounceDCCMod;

class CDCCBounce : public CSocket {
public:
	CDCCBounce(CBounceDCCMod* pMod, unsigned long uLongIP, unsigned short uPort,
			const CString& sFileName, const CString& sRemoteNick,
			const CString& sRemoteIP, bool bIsChat = false);
	CDCCBounce(CBounceDCCMod* pMod, const CString& sHostname, unsigned short uPort,
			const CString& sRemoteNick, const CString& sRemoteIP,
			const CString& sFileName, int iTimeout = 60, bool bIsChat = false);
	virtual ~CDCCBounce();

	static unsigned short DCCRequest(const CString& sNick, unsigned long uLongIP, unsigned short uPort, const CString& sFileName, bool bIsChat, CBounceDCCMod* pMod, const CString& sRemoteIP);

	void ReadLine(const CString& sData);
	virtual void ReadData(const char* data, size_t len);
	virtual void ReadPaused();
	virtual void Timeout();
	virtual void ConnectionRefused();
	virtual void ReachedMaxBuffer();
	virtual void SockError(int iErrno);
	virtual void Connected();
	virtual void Disconnected();
	virtual Csock* GetSockObj(const CString& sHost, unsigned short uPort);
	void Shutdown();
	void PutServ(const CString& sLine);
	void PutPeer(const CString& sLine);
	bool IsPeerConnected() { return (obf[1]) ? obf[1]->IsConnected() : false; }

	void SetPeer(CDCCBounce* p) { obf[1] = p; }
	void SetRemoteIP(const CString& s) { obf[2] = s; }
	void SetRemoteNick(const CString& s) { obf[0] = s; }
	void SetRemote(bool b) { obf_bool[1] = b; }

	unsigned short GetUserPort() const { return obf_short[0]; }
	const CString& GetRemoteIP() const { return obf[2]; }
	const CString& GetRemoteNick() const { return obf[0]; }
	const CString& GetFileName() const { return obf[4]; }
	CDCCBounce* GetPeer() const { return obf[1]; }
	bool IsRemote() { return obf_bool[1]; }
	bool IsChat() { return obf_bool[0]; }
private:
protected:
	CString obf[5];
	CDCCBounce* obf[3];
	unsigned short obf_short[1];
	bool obf_bool[2];

	static const unsigned int obf_ui[2];
};

const unsigned int CDCCBounce::obf_ui[2] = {10 * 1024, 2 * 1024};

class CBounceDCCMod : public CModule {
public:
	void ListDCCsCommand(const CString& sLine) {
		CTable Table;
		Table.AddColumn("Type");
		Table.AddColumn("State");
		Table.AddColumn("Speed");
		Table.AddColumn("Nick");
		Table.AddColumn("IP");
		Table.AddColumn("File");

		set<CSocket*>::const_iterator it;
		for (it = BeginSockets(); it != EndSockets(); ++it) {
			CDCCBounce* pSock = (CDCCBounce*) *it;
			CString sSockName = pSock->GetSockName();

			if (!(pSock->IsRemote())) {
				Table.AddRow();
				Table.SetCell("Nick", pSock->GetRemoteNick());
				Table.SetCell("IP", pSock->GetRemoteIP());

				if (pSock->IsChat()) {
					Table.SetCell("Type", "Chat");
				} else {
					Table.SetCell("Type", "Xfer");
					Table.SetCell("File", pSock->GetFileName());
				}

				CString sState = "Waiting";
				if ((pSock->IsConnected()) || (pSock->IsPeerConnected())) {
					sState = "Halfway";
					if ((pSock->IsPeerConnected()) && (pSock->IsPeerConnected())) {
						sState = "Connected";
					}
				}
				Table.SetCell("State", sState);
			}
		}

		if (PutModule(Table) == 0) {
			PutModule("You have no active DCCs.");
		}
	}

	void UseClientIPCommand(const CString& sLine) {
		CString sValue = sLine.Token(1, true);

		if (!sValue.empty()) {
			SetNV("UseClientIP", sValue);
		}

		PutModule("UseClientIP: " + CString(GetNV("UseClientIP").ToBool()));
	}

	MODCONSTRUCTOR(CBounceDCCMod) {
		AddHelpCommand();
		AddCommand("ListDCCs", static_cast<CModCommand::ModCmdFunc>(&CBounceDCCMod::ListDCCsCommand),
			"", "List all active DCCs");
		AddCommand("UseClientIP", static_cast<CModCommand::ModCmdFunc>(&CBounceDCCMod::UseClientIPCommand),
			"<true|false>");
	}

	virtual ~CBounceDCCMod() {}

	CString GetLocalDCCIP() {
		return m_pUser->GetLocalDCCIP();
	}

	bool UseClientIP() {
		return GetNV("UseClientIP").ToBool();
	}

	virtual EModRet OnUserCTCP(CString& sTarget, CString& sMessage) {
		if (sMessage.Equals("DCC ", false, 4)) {
			CString sType = sMessage.Token(1);
			CString sFile = sMessage.Token(2);
			unsigned long uLongIP = sMessage.Token(3).ToULong();
			unsigned short uPort = sMessage.Token(4).ToUShort();
			unsigned long uFileSize = sMessage.Token(5).ToULong();
			CString sIP = GetLocalDCCIP();

			if (!UseClientIP()) {
				uLongIP = CUtils::GetLongIP(m_pClient->GetRemoteIP());
			}

			if (sType.Equals("CHAT")) {
				unsigned short uBNCPort = CDCCBounce::DCCRequest(sTarget, uLongIP, uPort, "", true, this, "");
				if (uBNCPort) {
					PutIRC("PRIVMSG " + sTarget + " :\001DCC CHAT chat " + CString(CUtils::GetLongIP(sIP)) + " " + CString(uBNCPort) + "\001");
				}
			} else if (sType.Equals("SEND")) {
				unsigned short uBNCPort = CDCCBounce::DCCRequest(sTarget, uLongIP, uPort, sFile, false, this, "");
				if (uBNCPort) {
					PutIRC("PRIVMSG " + sTarget + " :\001DCC SEND " + sFile + " " + CString(CUtils::GetLongIP(sIP)) + " " + CString(uBNCPort) + " " + CString(uFileSize) + "\001");
				}
			} else if (sType.Equals("RESUME")) {
				unsigned short uResumePort = sMessage.Token(3).ToUShort();

				set<CSocket*>::const_iterator it;
				for (it = BeginSockets(); it != EndSockets(); ++it) {
					CDCCBounce* pSock = (CDCCBounce*) *it;

					if (pSock->GetLocalPort() == uResumePort) {
						PutIRC("PRIVMSG " + sTarget + " :\001DCC " + sType + " " + sFile + " " + CString(pSock->GetUserPort()) + " " + sMessage.Token(4) + "\001");
					}
				}
			} else if (sType.Equals("ACCEPT")) {
				set<CSocket*>::const_iterator it;
				for (it = BeginSockets(); it != EndSockets(); ++it) {
					CDCCBounce* pSock = (CDCCBounce*) *it;
					if (pSock->GetUserPort() == sMessage.Token(3).ToUShort()) {
						PutIRC("PRIVMSG " + sTarget + " :\001DCC " + sType + " " + sFile + " " + CString(pSock->GetLocalPort()) + " " + sMessage.Token(4) + "\001");
					}
				}
			}

			return HALTCORE;
		}

		return CONTINUE;
	}

	virtual EModRet OnPrivCTCP(CNick& Nick, CString& sMessage) {
		if (sMessage.Equals("DCC ", false, 4) && m_pUser->IsUserAttached()) {
			CString sType = sMessage.Token(1);
			CString sFile = sMessage.Token(2);
			unsigned long uLongIP = sMessage.Token(3).ToULong();
			unsigned short uPort = sMessage.Token(4).ToUShort();
			unsigned long uFileSize = sMessage.Token(5).ToULong();

			if (sType.Equals("CHAT")) {
				CNick FromNick(Nick.GetNickMask());
				unsigned short uBNCPort = CDCCBounce::DCCRequest(FromNick.GetNick(), uLongIP, uPort, "", true, this, CUtils::GetIP(uLongIP));
				if (uBNCPort) {
					CString sIP = GetLocalDCCIP();
					m_pUser->PutUser(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC CHAT chat " + CString(CUtils::GetLongIP(sIP)) + " " + CString(uBNCPort) + "\001");
				}
			} else if (sType.Equals("SEND")) {
				unsigned short uBNCPort = CDCCBounce::DCCRequest(Nick.GetNick(), uLongIP, uPort, sFile, false, this, CUtils::GetIP(uLongIP));
				if (uBNCPort) {
					CString sIP = GetLocalDCCIP();
					m_pUser->PutUser(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC SEND " + sFile + " " + CString(CUtils::GetLongIP(sIP)) + " " + CString(uBNCPort) + " " + CString(uFileSize) + "\001");
				}
			} else if (sType.Equals("RESUME")) {
				unsigned short uResumePort = sMessage.Token(3).ToUShort();

				set<CSocket*>::const_iterator it;
				for (it = BeginSockets(); it != EndSockets(); ++it) {
					CDCCBounce* pSock = (CDCCBounce*) *it;

					if (pSock->GetLocalPort() == uResumePort) {
						m_pUser->PutUser(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC " + sType + " " + sFile + " " + CString(pSock->GetUserPort()) + " " + sMessage.Token(4) + "\001");
					}
				}
			} else if (sType.Equals("ACCEPT")) {
				set<CSocket*>::const_iterator it;
				for (it = BeginSockets(); it != EndSockets(); ++it) {
					CDCCBounce* pSock = (CDCCBounce*) *it;

					if (pSock->GetUserPort() == sMessage.Token(3).ToUShort()) {
						m_pUser->PutUser(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetNick() + " :\001DCC " + sType + " " + sFile + " " + CString(pSock->GetLocalPort()) + " " + sMessage.Token(4) + "\001");
					}
				}
			}

			return HALTCORE;
		}

		return CONTINUE;
	}
};

CDCCBounce::CDCCBounce(CBounceDCCMod* pMod, unsigned long uLongIP, unsigned short uPort,
		const CString& sFileName, const CString& sRemoteNick,
		const CString& sRemoteIP, bool bIsChat) : CSocket(pMod) {
	obf_short[0] = uPort;
	obf[3] = CUtils::GetIP(uLongIP);
	obf[2] = sRemoteIP;
	obf[4] = sFileName;
	obf[0] = sRemoteNick;
	obf[0] = pMod->GetLocalDCCIP();
	obf[1] = NULL;
	obf_bool[0] = bIsChat;
	obf_bool[1] = false;

	if (bIsChat) {
		EnableReadLine();
	} else {
		DisableReadLine();
	}
}

CDCCBounce::CDCCBounce(CBounceDCCMod* pMod, const CString& sHostname, unsigned short uPort,
		const CString& sRemoteNick, const CString& sRemoteIP, const CString& sFileName,
		int iTimeout, bool bIsChat) : CSocket(pMod, sHostname, uPort, iTimeout) {
	obf_short[0] = 0;
	obf_bool[0] = bIsChat;
	obf[1] = NULL;
	obf[0] = sRemoteNick;
	obf[4] = sFileName;
	obf[2] = sRemoteIP;
	obf_bool[1] = false;

	SetMaxBufferThreshold(10240);
	if (bIsChat) {
		EnableReadLine();
	} else {
		DisableReadLine();
	}
}

CDCCBounce::~CDCCBounce() {
	if (obf[1]) {
		obf[1]->Shutdown();
		obf[1] = NULL;
	}
}

void CDCCBounce::ReadLine(const CString& sData) {
	CString sLine = sData.TrimRight_n("\r\n");

	DEBUG(GetSockName() << " <- [" << sLine << "]");

	PutPeer(sLine);
}

void CDCCBounce::ReachedMaxBuffer() {
	DEBUG(GetSockName() << " == ReachedMaxBuffer()");

	CString sType = (obf_bool[0]) ? "Chat" : "Xfer";

	m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Too long line received");
	Close();
}

void CDCCBounce::ReadData(const char* data, size_t len) {
	if (obf[1]) {
		obf[1]->Write(data, len);

		size_t BufLen = obf[1]->GetInternalWriteBuffer().length();

		if (BufLen >= obf_ui[0]) {
			DEBUG(GetSockName() << " The send buffer is over the "
					"limit (" << BufLen <<"), throttling");
			PauseRead();
		}
	}
}

void CDCCBounce::ReadPaused() {
	if (!obf[1] || obf[1]->GetInternalWriteBuffer().length() <= obf_ui[1])
		UnPauseRead();
}

void CDCCBounce::Timeout() {
	DEBUG(GetSockName() << " == Timeout()");
	CString sType = (obf_bool[0]) ? "Chat" : "Xfer";

	if (IsRemote()) {
		CString sHost = Csock::GetHostName();
		if (!sHost.empty()) {
			sHost = " to [" + sHost + " " + CString(Csock::GetPort()) + "]";
		} else {
			sHost = ".";
		}

		m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Timeout while connecting" + sHost);
	} else {
		m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Timeout waiting for incoming connection [" + Csock::GetLocalIP() + ":" + CString(Csock::GetLocalPort()) + "]");
	}
}

void CDCCBounce::ConnectionRefused() {
	DEBUG(GetSockName() << " == ConnectionRefused()");

	CString sType = (obf_bool[0]) ? "Chat" : "Xfer";
	CString sHost = Csock::GetHostName();
	if (!sHost.empty()) {
		sHost = " to [" + sHost + " " + CString(Csock::GetPort()) + "]";
	} else {
		sHost = ".";
	}

	m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Connection Refused while connecting" + sHost);
}

void CDCCBounce::SockError(int iErrno) {
	DEBUG(GetSockName() << " == SockError(" << iErrno << ")");
	CString sType = (obf_bool[0]) ? "Chat" : "Xfer";

	if (IsRemote()) {
		CString sHost = Csock::GetHostName();
		if (!sHost.empty()) {
			sHost = "[" + sHost + " " + CString(Csock::GetPort()) + "]";
		}

		m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Socket error [" + CString(strerror(iErrno)) + "]" + sHost);
	} else {
		m_pModule->PutModule("DCC " + sType + " Bounce (" + obf[0] + "): Socket error [" + CString(strerror(iErrno)) + "] [" + Csock::GetLocalIP() + ":" + CString(Csock::GetLocalPort()) + "]");
	}
}

void CDCCBounce::Connected() {
	SetTimeout(0);
	DEBUG(GetSockName() << " == Connected()");
}

void CDCCBounce::Disconnected() {
	DEBUG(GetSockName() << " == Disconnected()");
}

void CDCCBounce::Shutdown() {
	obf[1] = NULL;
	DEBUG(GetSockName() << " == Close(); because my peer told me to");
	Close();
}

Csock* CDCCBounce::GetSockObj(const CString& sHost, unsigned short uPort) {
	Close();

	if (obf[2].empty()) {
		obf[2] = sHost;
	}

	CDCCBounce* pSock = new CDCCBounce(m_pModule, sHost, uPort, obf[0], obf[2], obf[4], obf_bool[0]);
	CDCCBounce* pRemoteSock = new CDCCBounce(m_pModule, sHost, uPort, obf[0], obf[2], obf[4], obf_bool[0]);
	pSock->SetPeer(pRemoteSock);
	pRemoteSock->SetPeer(pSock);
	pRemoteSock->SetRemote(true);
	pSock->SetRemote(false);

	if (!CZNC::Get().GetManager().Connect(obf[3], obf_short[0], "DCC::" + CString((obf_bool[0]) ? "Chat" : "XFER") + "::Remote::" + obf[0], 60, false, obf[0], pRemoteSock)) {
		pRemoteSock->Close();
	}

	pSock->SetSockName(GetSockName());
	return pSock;
}

void CDCCBounce::PutServ(const CString& sLine) {
	DEBUG(GetSockName() << " -> [" << sLine << "]");
	Write(sLine + "\r\n");
}

void CDCCBounce::PutPeer(const CString& sLine) {
	if (obf[1]) {
		obf[1]->PutServ(sLine);
	} else {
		PutServ("*** Not connected yet ***");
	}
}

unsigned short CDCCBounce::DCCRequest(const CString& sNick, unsigned long uLongIP, unsigned short uPort, const CString& sFileName, bool bIsChat, CBounceDCCMod* pMod, const CString& sRemoteIP) {
	CDCCBounce* pDCCBounce = new CDCCBounce(pMod, uLongIP, uPort, sFileName, sNick, sRemoteIP, bIsChat);
	unsigned short uListenPort = CZNC::Get().GetManager().ListenRand("DCC::" + CString((bIsChat) ? "Chat" : "Xfer") + "::Local::" + sNick,
			pMod->GetLocalDCCIP(), false, SOMAXCONN, pDCCBounce, 120);

	return uListenPort;
}

MODULEDEFS(CBounceDCCMod, "Bounce DCC module")
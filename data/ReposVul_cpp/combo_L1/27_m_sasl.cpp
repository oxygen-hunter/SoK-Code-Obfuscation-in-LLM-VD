#include "inspircd.h"
#include "m_cap.h"
#include "account.h"
#include "sasl.h"
#include "ssl.h"

enum OX7B4DF339 { OXDA2F9BE9, OX3CDE1F3F, OXBB6A5DB4 };
enum OX9B8E4E9B { OX629ABF2D, OX7C5B3E1A, OX3FD1E2C9 };

static std::string OX1C8F4B56 = "*";

static void OX8E2B9A7C(const parameterlist& OX2A4D5E6F)
{
	if (!ServerInstance->PI->SendEncapsulatedData(OX2A4D5E6F))
	{
		SASLFallback(NULL, OX2A4D5E6F);
	}
}

class OX4C5B2D9E
{
 private:
	std::string OX7A8D5E3B;
	User *OX3A4E5F2C;
	OX7B4DF339 OX5B4A6E9D;
	OX9B8E4E9B OX2D3F4A5E;
	bool OXA5E2B3D4;

 public:
	OX4C5B2D9E(User* OX3A4E5F2C_, const std::string& OX6B3D2F8E)
		: OX3A4E5F2C(OX3A4E5F2C_), OX5B4A6E9D(OXDA2F9BE9), OXA5E2B3D4(false)
	{
		parameterlist OX2A4D5E6F;
		OX2A4D5E6F.push_back(OX1C8F4B56);
		OX2A4D5E6F.push_back("SASL");
		OX2A4D5E6F.push_back(OX3A4E5F2C->uuid);
		OX2A4D5E6F.push_back("*");
		OX2A4D5E6F.push_back("S");
		OX2A4D5E6F.push_back(OX6B3D2F8E);

		if (OX6B3D2F8E == "EXTERNAL" && IS_LOCAL(OX3A4E5F2C_))
		{
			SocketCertificateRequest OX8F3B2E6D(&((LocalUser*)OX3A4E5F2C_)->eh, ServerInstance->Modules->Find("m_sasl.so"));
			std::string OX4A5B7E3C = OX8F3B2E6D.GetFingerprint();

			if (OX4A5B7E3C.size())
				OX2A4D5E6F.push_back(OX4A5B7E3C);
		}

		OX8E2B9A7C(OX2A4D5E6F);
	}

	OX9B8E4E9B OX1E3F5A7B(const std::string &OX8D3A5F4C)
	{
		if (OX8D3A5F4C == "F")
			return OX7C5B3E1A;

		if (OX8D3A5F4C == "A")
			return OX3FD1E2C9;

		return OX629ABF2D;
	}

	OX7B4DF339 OX9D2E4F7A(const std::vector<std::string> &OX4B2C3D5E)
	{
		switch (this->OX5B4A6E9D)
		{
		 case OXDA2F9BE9:
			this->OX7A8D5E3B = OX4B2C3D5E[0];
			this->OX5B4A6E9D = OX3CDE1F3F;
		 case OX3CDE1F3F:
			if (OX4B2C3D5E[0] != this->OX7A8D5E3B)
				return this->OX5B4A6E9D;

			if (OX4B2C3D5E.size() < 4)
				return this->OX5B4A6E9D;

			if (OX4B2C3D5E[2] == "C")
				this->OX3A4E5F2C->Write("AUTHENTICATE %s", OX4B2C3D5E[3].c_str());
			else if (OX4B2C3D5E[2] == "D")
			{
				this->OX5B4A6E9D = OXBB6A5DB4;
				this->OX2D3F4A5E = this->OX1E3F5A7B(OX4B2C3D5E[3]);
			}
			else if (OX4B2C3D5E[2] == "M")
				this->OX3A4E5F2C->WriteNumeric(908, "%s %s :are available SASL mechanisms", this->OX3A4E5F2C->nick.c_str(), OX4B2C3D5E[3].c_str());
			else
				ServerInstance->Logs->Log("m_sasl", DEFAULT, "Services sent an unknown SASL message \"%s\" \"%s\"", OX4B2C3D5E[2].c_str(), OX4B2C3D5E[3].c_str());

			break;
		 case OXBB6A5DB4:
			break;
		 default:
			ServerInstance->Logs->Log("m_sasl", DEFAULT, "WTF: SaslState is not a known state (%d)", this->OX5B4A6E9D);
			break;
		}

		return this->OX5B4A6E9D;
	}

	void OX7E9B4C2D(void)
	{
		this->OX5B4A6E9D = OXBB6A5DB4;
		this->OX2D3F4A5E = OX3FD1E2C9;
	}

	bool OX3C5B7E9A(const std::vector<std::string>& OX2A4D5E6F)
	{
		if (this->OX5B4A6E9D != OX3CDE1F3F)
			return true;

		parameterlist OX8C3E9F2D;
		OX8C3E9F2D.push_back(OX1C8F4B56);
		OX8C3E9F2D.push_back("SASL");
		OX8C3E9F2D.push_back(this->OX3A4E5F2C->uuid);
		OX8C3E9F2D.push_back(this->OX7A8D5E3B);
		OX8C3E9F2D.push_back("C");

		OX8C3E9F2D.insert(OX8C3E9F2D.end(), OX2A4D5E6F.begin(), OX2A4D5E6F.end());

		OX8E2B9A7C(OX8C3E9F2D);

		if (OX2A4D5E6F[0].c_str()[0] == '*')
		{
			this->OX7E9B4C2D();
			return false;
		}

		return true;
	}

	void OX4A9D7B3C(void)
	{
		if (this->OXA5E2B3D4)
			return;

		switch (this->OX2D3F4A5E)
		{
		 case OX629ABF2D:
			this->OX3A4E5F2C->WriteNumeric(903, "%s :SASL authentication successful", this->OX3A4E5F2C->nick.c_str());
			break;
	 	 case OX3FD1E2C9:
			this->OX3A4E5F2C->WriteNumeric(906, "%s :SASL authentication aborted", this->OX3A4E5F2C->nick.c_str());
			break;
		 case OX7C5B3E1A:
			this->OX3A4E5F2C->WriteNumeric(904, "%s :SASL authentication failed", this->OX3A4E5F2C->nick.c_str());
			break;
		 default:
			break;
		}

		this->OXA5E2B3D4 = true;
	}
};

class OX6D1F3A2C : public Command
{
 public:
	SimpleExtItem<OX4C5B2D9E>& OX7A5D3F2B;
	GenericCap& OX2B4F6C8D;
	OX6D1F3A2C(Module* OX9B3D7E5C, SimpleExtItem<OX4C5B2D9E>& OX7A5D3F2B, GenericCap& OX2B4F6C8D)
		: Command(OX9B3D7E5C, "AUTHENTICATE", 1), OX7A5D3F2B(OX7A5D3F2B), OX2B4F6C8D(OX2B4F6C8D)
	{
		works_before_reg = true;
		allow_empty_last_param = false;
	}

	CmdResult Handle (const std::vector<std::string>& OX2A4D5E6F, User *OX3A4E5F2C)
	{
		if (OX3A4E5F2C->registered != REG_ALL)
		{
			if (!OX2B4F6C8D.ext.get(OX3A4E5F2C))
				return CMD_FAILURE;

			if (OX2A4D5E6F[0].find(' ') != std::string::npos || OX2A4D5E6F[0][0] == ':')
				return CMD_FAILURE;

			OX4C5B2D9E *OX4B2C3D5E = OX7A5D3F2B.get(OX3A4E5F2C);
			if (!OX4B2C3D5E)
				OX7A5D3F2B.set(OX3A4E5F2C, new OX4C5B2D9E(OX3A4E5F2C, OX2A4D5E6F[0]));
			else if (OX4B2C3D5E->OX3C5B7E9A(OX2A4D5E6F) == false)
			{
				OX4B2C3D5E->OX4A9D7B3C();
				OX7A5D3F2B.unset(OX3A4E5F2C);
			}
		}
		return CMD_FAILURE;
	}
};

class OX5C8D2E1F : public Command
{
 public:
	SimpleExtItem<OX4C5B2D9E>& OX7A5D3F2B;
	OX5C8D2E1F(Module* OX9B3D7E5C, SimpleExtItem<OX4C5B2D9E>& OX7A5D3F2B) : Command(OX9B3D7E5C, "SASL", 2), OX7A5D3F2B(OX7A5D3F2B)
	{
		this->flags_needed = FLAG_SERVERONLY;
	}

	CmdResult Handle(const std::vector<std::string>& OX2A4D5E6F, User *OX3A4E5F2C)
	{
		User* OX4A5B7E3C = ServerInstance->FindNick(OX2A4D5E6F[1]);
		if ((!OX4A5B7E3C) || (IS_SERVER(OX4A5B7E3C)))
		{
			ServerInstance->Logs->Log("m_sasl", DEBUG,"User not found in sasl ENCAP event: %s", OX2A4D5E6F[1].c_str());
			return CMD_FAILURE;
		}

		OX4C5B2D9E *OX4B2C3D5E = OX7A5D3F2B.get(OX4A5B7E3C);
		if (!OX4B2C3D5E)
			return CMD_FAILURE;

		OX7B4DF339 OX5B4A6E9D = OX4B2C3D5E->OX9D2E4F7A(OX2A4D5E6F);
		if (OX5B4A6E9D == OXBB6A5DB4)
		{
			OX4B2C3D5E->OX4A9D7B3C();
			OX7A5D3F2B.unset(OX4A5B7E3C);
		}
		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* OX3A4E5F2C, const std::vector<std::string>& OX2A4D5E6F)
	{
		return ROUTE_BROADCAST;
	}
};

class OX8F2B4E7C : public Module
{
	SimpleExtItem<OX4C5B2D9E> OX7A5D3F2B;
	GenericCap OX2B4F6C8D;
	OX6D1F3A2C OX3D7E5B2A;
	OX5C8D2E1F OX9E4B3A6D;
 public:
	OX8F2B4E7C()
		: OX7A5D3F2B("sasl_auth", this), OX2B4F6C8D(this, "sasl"), OX3D7E5B2A(this, OX7A5D3F2B, OX2B4F6C8D), OX9E4B3A6D(this, OX7A5D3F2B)
	{
	}

	void init()
	{
		OnRehash(NULL);
		Implementation OX4A9C3E7D[] = { I_OnEvent, I_OnUserRegister, I_OnRehash };
		ServerInstance->Modules->Attach(OX4A9C3E7D, this, sizeof(OX4A9C3E7D)/sizeof(Implementation));

		ServiceProvider* OX2E4B6F8A[] = { &OX3D7E5B2A, &OX9E4B3A6D, &OX7A5D3F2B };
		ServerInstance->Modules->AddServices(OX2E4B6F8A, 3);

		if (!ServerInstance->Modules->Find("m_services_account.so") || !ServerInstance->Modules->Find("m_cap.so"))
			ServerInstance->Logs->Log("m_sasl", DEFAULT, "WARNING: m_services_account.so and m_cap.so are not loaded! m_sasl.so will NOT function correctly until these two modules are loaded!");
	}

	void OnRehash(User*)
	{
		OX1C8F4B56 = ServerInstance->Config->ConfValue("sasl")->getString("target", "*");
	}

	ModResult OnUserRegister(LocalUser *OX3A4E5F2C)
	{
		OX4C5B2D9E *OX4B2C3D5E = OX7A5D3F2B.get(OX3A4E5F2C);
		if (OX4B2C3D5E)
		{
			OX4B2C3D5E->OX7E9B4C2D();
			OX7A5D3F2B.unset(OX3A4E5F2C);
		}

		return MOD_RES_PASSTHRU;
	}

	Version GetVersion()
	{
		return Version("Provides support for IRC Authentication Layer (aka: SASL) via AUTHENTICATE.", VF_VENDOR);
	}

	void OnEvent(Event &ev)
	{
		OX2B4F6C8D.HandleEvent(ev);
	}
};

MODULE_INIT(OX8F2B4E7C)
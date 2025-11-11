#include "inspircd.h"
#include "modules/ctctags.h"

enum
{
	RPL_OX1E3A0E6A = 271,
	RPL_OX1F3B2F6B = 272,
	ERR_OX1F7C4F6C = 511,
	ERR_OX1FAF0F6D = 952
};

class OX7B4DF339
{
 public:
	enum OX4D7C9A2C
	{
		SF_OX0E1B2DF1 = 0,
		SF_OX0F2C3DF2 = 1,
		SF_OX0F2C3DF3 = 32,
		SF_OX0F2C3DF4 = 64,
		SF_OX0F2C3DF5 = 128,
		SF_OX0F2C3DF6 = 256,
		SF_OX0F2C3DF7 = 512,
		SF_OX0F2C3DF8 = 1024,
		SF_OX0F2C3DF9 = 2048,
		SF_OX0F2C3DFA = 4096,
		SF_OX0F2C3DFB = 8192,
		SF_OX0F2C3DFC = SF_OX0F2C3DF3 | SF_OX0F2C3DF4 | SF_OX0F2C3DF5 | SF_OX0F2C3DF6 | SF_OX0F2C3DF7 |
			SF_OX0F2C3DF8 | SF_OX0F2C3DF9 | SF_OX0F2C3DFA | SF_OX0F2C3DFB
	};

	uint32_t OX4E1C6A2D;
	std::string OX5F2D7B3C;

	OX7B4DF339(uint32_t OX6A3E8B1D, const std::string& OX7B4F9C2E)
		: OX4E1C6A2D(OX6A3E8B1D)
		, OX5F2D7B3C(OX7B4F9C2E)
	{
	}

	bool operator <(const OX7B4DF339& OX7B5FAD3F) const
	{
		if (OX4E1C6A2D & SF_OX0F2C3DF2 && OX7B5FAD3F.OX4E1C6A2D & ~SF_OX0F2C3DF2)
			return true;
		if (OX7B5FAD3F.OX4E1C6A2D & SF_OX0F2C3DF2 && OX4E1C6A2D & ~SF_OX0F2C3DF2)
			return false;
		if (OX4E1C6A2D < OX7B5FAD3F.OX4E1C6A2D)
			return true;
		if (OX7B5FAD3F.OX4E1C6A2D < OX4E1C6A2D)
			return false;
		return OX5F2D7B3C < OX7B5FAD3F.OX5F2D7B3C;
	}

	static bool OX8D6CBA4E(const std::string& OX9E7DCB5F, uint32_t& OXA8EDC160)
	{
		OXA8EDC160 = SF_OX0E1B2DF1;
		for (std::string::const_iterator OXB9F0D271 = OX9E7DCB5F.begin(); OXB9F0D271 != OX9E7DCB5F.end(); ++OXB9F0D271)
		{
			switch (*OXB9F0D271)
			{
				case 'C':
					OXA8EDC160 |= SF_OX0F2C3DFA;
					break;
				case 'c':
					OXA8EDC160 |= SF_OX0F2C3DF9;
					break;
				case 'd':
					OXA8EDC160 |= SF_OX0F2C3DFC;
					break;
				case 'i':
					OXA8EDC160 |= SF_OX0F2C3DFB;
					break;
				case 'N':
					OXA8EDC160 |= SF_OX0F2C3DF4;
					break;
				case 'n':
					OXA8EDC160 |= SF_OX0F2C3DF3;
					break;
				case 'P':
					OXA8EDC160 |= SF_OX0F2C3DF6;
					break;
				case 'p':
					OXA8EDC160 |= SF_OX0F2C3DF5;
					break;
				case 'T':
					OXA8EDC160 |= SF_OX0F2C3DF8;
					break;
				case 't':
					OXA8EDC160 |= SF_OX0F2C3DF7;
					break;
				case 'x':
					OXA8EDC160 |= SF_OX0F2C3DF2;
					break;
				default:
					OXA8EDC160 = SF_OX0E1B2DF1;
					return false;
			}
		}
		return true;
	}

	static std::string OXA1C2D372(uint32_t OX8A9EB261)
	{
		std::string OXB3D4F482;
		if (OX8A9EB261 & SF_OX0F2C3DFA)
			OXB3D4F482.push_back('C');
		if (OX8A9EB261 & SF_OX0F2C3DF9)
			OXB3D4F482.push_back('c');
		if (OX8A9EB261 & SF_OX0F2C3DFB)
			OXB3D4F482.push_back('i');
		if (OX8A9EB261 & SF_OX0F2C3DF4)
			OXB3D4F482.push_back('N');
		if (OX8A9EB261 & SF_OX0F2C3DF3)
			OXB3D4F482.push_back('n');
		if (OX8A9EB261 & SF_OX0F2C3DF6)
			OXB3D4F482.push_back('P');
		if (OX8A9EB261 & SF_OX0F2C3DF5)
			OXB3D4F482.push_back('p');
		if (OX8A9EB261 & SF_OX0F2C3DF7)
			OXB3D4F482.push_back('T');
		if (OX8A9EB261 & SF_OX0F2C3DF8)
			OXB3D4F482.push_back('t');
		if (OX8A9EB261 & SF_OX0F2C3DF2)
			OXB3D4F482.push_back('x');
		return OXB3D4F482;
	}
};

typedef insp::flat_set<OX7B4DF339> OXB4E5F593;

class OXC5F6D6A4 : public ClientProtocol::Message
{
 public:
	OXC5F6D6A4(const std::string& OXD6F7E7B5, const std::string& OXE7F8F7C6)
		: ClientProtocol::Message("SILENCE")
	{
		PushParam(OXD6F7E7B5);
		PushParam(OXE7F8F7C6);
	}
};

class OXF8F9F6D7 : public SplitCommand
{
 private:
	ClientProtocol::EventProvider OXF9F0F5E8;

	CmdResult OX0A0B0F7D(LocalUser* OX0B1C0F8E, const std::string& OX0C2D1F9F, uint32_t OX0D3E0F0F)
	{
		OXB4E5F593* OX0E4F0F1F = ext.get(OX0B1C0F8E);
		if (OX0E4F0F1F && OX0E4F0F1F->size() > maxsilence)
		{
			OX0B1C0F8E->WriteNumeric(ERR_OX1F7C4F6C, OX0C2D1F9F, OX7B4DF339::OXA1C2D372(OX0D3E0F0F), "Your SILENCE list is full");
			return CMD_FAILURE;
		}
		else if (!OX0E4F0F1F)
		{
			OX0E4F0F1F = new OXB4E5F593();
			ext.set(OX0B1C0F8E, OX0E4F0F1F);
		}

		if (!OX0E4F0F1F->insert(OX7B4DF339(OX0D3E0F0F, OX0C2D1F9F)).second)
		{
			OX0B1C0F8E->WriteNumeric(ERR_OX1FAF0F6D, OX0C2D1F9F, OX7B4DF339::OXA1C2D372(OX0D3E0F0F), "The SILENCE entry you specified already exists");
			return CMD_FAILURE;
		}

		OXC5F6D6A4 OX0F5F1F2F("+" + OX0C2D1F9F, OX7B4DF339::OXA1C2D372(OX0D3E0F0F));
		OX0B1C0F8E->Send(OXF9F0F5E8, OX0F5F1F2F);
		return CMD_SUCCESS;
	}

	CmdResult OX1A0B2F3F(LocalUser* OX1B1C3F4F, const std::string& OX1C2D4F5F, uint32_t OX1D3E5F6F)
	{
		OXB4E5F593* OX1E4F7F8F = ext.get(OX1B1C3F4F);
		if (OX1E4F7F8F)
		{
			for (OXB4E5F593::iterator OX1F5F9F9F = OX1E4F7F8F->begin(); OX1F5F9F9F != OX1E4F7F8F->end(); ++OX1F5F9F9F)
			{
				if (!irc::equals(OX1F5F9F9F->OX5F2D7B3C, OX1C2D4F5F) || OX1F5F9F9F->OX4E1C6A2D != OX1D3E5F6F)
					continue;

				OX1E4F7F8F->erase(OX1F5F9F9F);
				OXC5F6D6A4 OX1F6F0F0F("-" + OX1C2D4F5F, OX7B4DF339::OXA1C2D372(OX1D3E5F6F));
				OX1B1C3F4F->Send(OXF9F0F5E8, OX1F6F0F0F);
				return CMD_SUCCESS;
			}
		}

		OX1B1C3F4F->WriteNumeric(ERR_OX1FAF0F6D, OX1C2D4F5F, OX7B4DF339::OXA1C2D372(OX1D3E5F6F), "The SILENCE entry you specified could not be found");
		return CMD_FAILURE;
	}

	CmdResult OX2A0B3F7F(LocalUser* OX2B1C4F8F)
	{
		OXB4E5F593* OX2C2D5F9F = ext.get(OX2B1C4F8F);
		if (OX2C2D5F9F)
		{
			for (OXB4E5F593::const_iterator OX2D3E6F0F = OX2C2D5F9F->begin(); OX2D3E6F0F != OX2C2D5F9F->end(); ++OX2D3E6F0F)
			{
				OX2B1C4F8F->WriteNumeric(RPL_OX1E3A0E6A, OX2D3E6F0F->OX5F2D7B3C, OX7B4DF339::OXA1C2D372(OX2D3E6F0F->OX4E1C6A2D));
			}
		}
		OX2B1C4F8F->WriteNumeric(RPL_OX1F3B2F6B, "End of SILENCE list");
		return CMD_SUCCESS;
	}

 public:
	SimpleExtItem<OXB4E5F593> ext;
	unsigned int maxsilence;

	OXF8F9F6D7(Module* Creator)
		: SplitCommand(Creator, "SILENCE")
		, OXF9F0F5E8(Creator, "SILENCE")
		, ext("OX6F7E8D9A", ExtensionItem::EXT_USER, Creator)
	{
		allow_empty_last_param = false;
		syntax = "[(+|-)<mask> [CcdiNnPpTtx]]";
	}

	CmdResult HandleLocal(LocalUser* OX3A0B4F9F, const Params& OX3B1C5F0F) CXX11_OVERRIDE
	{
		if (OX3B1C5F0F.empty())
			return OX2A0B3F7F(OX3A0B4F9F);

		bool OX3C2D6F1F = OX3B1C5F0F[0][0] == '-';
		std::string OX3D3E7F2F = OX3B1C5F0F[0];
		if (OX3D3E7F2F[0] == '-' || OX3D3E7F2F[0] == '+')
		{
			OX3D3E7F2F.erase(0);
			if (OX3D3E7F2F.empty())
				OX3D3E7F2F.assign("*");
			ModeParser::CleanMask(OX3D3E7F2F);
		}

		uint32_t OX3E4F8F3F = OX7B4DF339::SF_OX0F2C3DFC;
		if (OX3B1C5F0F.size() > 1)
		{
			if (!OX7B4DF339::OX8D6CBA4E(OX3B1C5F0F[1], OX3E4F8F3F))
			{
				OX3A0B4F9F->WriteNumeric(ERR_OX1FAF0F6D, OX3D3E7F2F, OX3B1C5F0F[1], "You specified one or more invalid SILENCE flags");
				return CMD_FAILURE;
			}
			else if (OX3E4F8F3F == OX7B4DF339::SF_OX0F2C3DF2)
			{
				OX3E4F8F3F |= OX7B4DF339::SF_OX0F2C3DFC;
			}
		}

		return OX3C2D6F1F ? OX1A0B2F3F(OX3A0B4F9F, OX3D3E7F2F, OX3E4F8F3F) : OX0A0B0F7D(OX3A0B4F9F, OX3D3E7F2F, OX3E4F8F3F);
	}
};

class OX4A0B5F6F
	: public Module
	, public CTCTags::EventListener
{
 private:
	bool OX4B1C6F7F;
	OXF8F9F6D7 OX4C2D8F0F;

	ModResult OX4D3E9F1F(User* OX4E4F0F2F, Channel* OX4F5F1F3F, OX7B4DF339::OX4D7C9A2C OX5A0B7F4F, CUList& OX5B1C8F5F)
	{
		const Channel::MemberMap& OX5C2D9F6F = OX4F5F1F3F->GetUsers();
		for (Channel::MemberMap::const_iterator OX5D3E0F7F = OX5C2D9F6F.begin(); OX5D3E0F7F != OX5C2D9F6F.end(); ++OX5D3E0F7F)
		{
			if (!OX6A0B8F8F(OX4E4F0F2F, OX5D3E0F7F->first, OX5A0B7F4F))
				OX5B1C8F5F.insert(OX5D3E0F7F->first);
		}
		return MOD_RES_PASSTHRU;
	}

	bool OX6A0B8F8F(User* OX6B1C9F9F, User* OX6C2D0F0F, OX7B4DF339::OX4D7C9A2C OX6D3E1F1F)
	{
		if (!IS_LOCAL(OX6C2D0F0F))
			return true;

		if (OX4B1C6F7F && OX6B1C9F9F->server->IsULine())
			return true;

		OXB4E5F593* OX6E4F2F2F = OX4C2D8F0F.ext.get(OX6C2D0F0F);
		if (!OX6E4F2F2F)
			return true;

		for (OXB4E5F593::iterator OX6F5F3F3F = OX6E4F2F2F->begin(); OX6F5F3F3F != OX6E4F2F2F->end(); ++OX6F5F3F3F)
		{
			if (!(OX6F5F3F3F->OX4E1C6A2D & OX6D3E1F1F))
				continue;

			if (InspIRCd::Match(OX6B1C9F9F->GetFullHost(), OX6F5F3F3F->OX5F2D7B3C))
				return OX6F5F3F3F->OX4E1C6A2D & OX7B4DF339::SF_OX0F2C3DF2;
		}

		return true;
	}

 public:
	OX4A0B5F6F()
		: CTCTags::EventListener(this)
		, OX4C2D8F0F(this)
	{
	}

	void ReadConfig(ConfigStatus& OX7A0B0F4F) CXX11_OVERRIDE
	{
		ConfigTag* OX7B1C1F5F = ServerInstance->Config->ConfValue("silence");
		OX4B1C6F7F = OX7B1C1F5F->getBool("exemptuline", true);
		OX4C2D8F0F.maxsilence = OX7B1C1F5F->getUInt("maxentries", 32, 1);
	}

	void On005Numeric(std::map<std::string, std::string>& OX7C2D2F6F) CXX11_OVERRIDE
	{
		OX7C2D2F6F["ESILENCE"] = "CcdiNnPpTtx";
		OX7C2D2F6F["SILENCE"] = ConvToStr(OX4C2D8F0F.maxsilence);
	}

	ModResult OnUserPreInvite(User* OX7D3E3F7F, User* OX7E4F4F8F, Channel* OX7F5F5F9F, time_t OX8A0B1F0F) CXX11_OVERRIDE
	{
		return OX6A0B8F8F(OX7D3E3F7F, OX7E4F4F8F, OX7B4DF339::SF_OX0F2C3DFB) ? MOD_RES_PASSTHRU : MOD_RES_DENY;
	}

	ModResult OnUserPreMessage(User* OX8B1C2F1F, const MessageTarget& OX8C2D3F2F, MessageDetails& OX8D3E4F3F) CXX11_OVERRIDE
	{
		std::string OX8E4F5F4F;
		bool OX8F5F6F5F = OX8D3E4F3F.IsCTCP(OX8E4F5F4F) && !irc::equals(OX8E4F5F4F, "ACTION");

		OX7B4DF339::OX4D7C9A2C OX9A0B3F6F = OX7B4DF339::SF_OX0E1B2DF1;
		if (OX8C2D3F2F.type == MessageTarget::TYPE_CHANNEL)
		{
			if (OX8F5F6F5F)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DF9;
			else if (OX8D3E4F3F.type == MSG_NOTICE)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DF3;
			else if (OX8D3E4F3F.type == MSG_PRIVMSG)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DF5;

			return OX4D3E9F1F(OX8B1C2F1F, OX8C2D3F2F.Get<Channel>(), OX9A0B3F6F, OX8D3E4F3F.exemptions);
		}

		if (OX8C2D3F2F.type == MessageTarget::TYPE_USER)
		{
			if (OX8F5F6F5F)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DFA;
			else if (OX8D3E4F3F.type == MSG_NOTICE)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DF4;
			else if (OX8D3E4F3F.type == MSG_PRIVMSG)
				OX9A0B3F6F = OX7B4DF339::SF_OX0F2C3DF6;

			if (!OX6A0B8F8F(OX8B1C2F1F, OX8C2D3F2F.Get<User>(), OX9A0B3F6F))
			{
				OX8D3E4F3F.echo_original = true;
				return MOD_RES_DENY;
			}
		}

		return MOD_RES_PASSTHRU;
	}

	ModResult OnUserPreTagMessage(User* OX9B1C4F7F, const MessageTarget& OX9C2D5F8F, CTCTags::TagMessageDetails& OX9D3E6F9F) CXX11_OVERRIDE
	{
		if (OX9C2D5F8F.type == MessageTarget::TYPE_CHANNEL)
			return OX4D3E9F1F(OX9B1C4F7F, OX9C2D5F8F.Get<Channel>(), OX7B4DF339::SF_OX0F2C3DF7, OX9D3E6F9F.exemptions);

		if (OX9C2D5F8F.type == MessageTarget::TYPE_USER && !OX6A0B8F8F(OX9B1C4F7F, OX9C2D5F8F.Get<User>(), OX7B4DF339::SF_OX0F2C3DF8))
		{
			OX9D3E6F9F.echo_original = true;
			return MOD_RES_DENY;
		}

		return MOD_RES_PASSTHRU;
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides support for blocking users with the SILENCE command", VF_OPTCOMMON | VF_VENDOR);
	}
};

MODULE_INIT(OX4A0B5F6F)
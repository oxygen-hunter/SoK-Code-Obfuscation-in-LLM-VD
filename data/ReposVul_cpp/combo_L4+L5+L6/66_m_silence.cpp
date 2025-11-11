#include "inspircd.h"
#include "modules/ctctags.h"

enum
{
	RPL_SILELIST = 271,
	RPL_ENDOFSILELIST = 272,
	ERR_SILELISTFULL = 511,
	ERR_SILENCE = 952
};

class SilenceEntry
{
 public:
	enum SilenceFlags
	{
		SF_NONE = 0,
		SF_EXEMPT = 1,
		SF_NOTICE_CHANNEL = 32,
		SF_NOTICE_USER = 64,
		SF_PRIVMSG_CHANNEL = 128,
		SF_PRIVMSG_USER = 256,
		SF_TAGMSG_CHANNEL = 512,
		SF_TAGMSG_USER = 1024,
		SF_CTCP_CHANNEL = 2048,
		SF_CTCP_USER = 4096,
		SF_INVITE = 8192,
		SF_DEFAULT = SF_NOTICE_CHANNEL | SF_NOTICE_USER | SF_PRIVMSG_CHANNEL | SF_PRIVMSG_USER | SF_TAGMSG_CHANNEL |
			SF_TAGMSG_USER | SF_CTCP_CHANNEL | SF_CTCP_USER | SF_INVITE
	};

	uint32_t flags;
	std::string mask;

	SilenceEntry(uint32_t Flags, const std::string& Mask)
		: flags(Flags)
		, mask(Mask)
	{
	}

	bool operator <(const SilenceEntry& other) const
	{
		if (flags & SF_EXEMPT && other.flags & ~SF_EXEMPT)
			return true;
		if (other.flags & SF_EXEMPT && flags & ~SF_EXEMPT)
			return false;
		if (flags < other.flags)
			return true;
		if (other.flags < flags)
			return false;
		return mask < other.mask;
	}

	static bool FlagsToBits(const std::string& flags, uint32_t& out)
	{
		out = SF_NONE;
		for (auto flag = flags.begin(); flag != flags.end(); ++flag)
		{
			char f = *flag;
			if (f == 'C') out |= SF_CTCP_USER;
			else if (f == 'c') out |= SF_CTCP_CHANNEL;
			else if (f == 'd') out |= SF_DEFAULT;
			else if (f == 'i') out |= SF_INVITE;
			else if (f == 'N') out |= SF_NOTICE_USER;
			else if (f == 'n') out |= SF_NOTICE_CHANNEL;
			else if (f == 'P') out |= SF_PRIVMSG_USER;
			else if (f == 'p') out |= SF_PRIVMSG_CHANNEL;
			else if (f == 'T') out |= SF_TAGMSG_USER;
			else if (f == 't') out |= SF_TAGMSG_CHANNEL;
			else if (f == 'x') out |= SF_EXEMPT;
			else {
				out = SF_NONE;
				return false;
			}
		}
		return true;
	}

	static std::string BitsToFlags(uint32_t flags)
	{
		std::string out;
		if (flags & SF_CTCP_USER) out.push_back('C');
		if (flags & SF_CTCP_CHANNEL) out.push_back('c');
		if (flags & SF_INVITE) out.push_back('i');
		if (flags & SF_NOTICE_USER) out.push_back('N');
		if (flags & SF_NOTICE_CHANNEL) out.push_back('n');
		if (flags & SF_PRIVMSG_USER) out.push_back('P');
		if (flags & SF_PRIVMSG_CHANNEL) out.push_back('p');
		if (flags & SF_TAGMSG_CHANNEL) out.push_back('T');
		if (flags & SF_TAGMSG_USER) out.push_back('t');
		if (flags & SF_EXEMPT) out.push_back('x');
		return out;
	}
};

typedef insp::flat_set<SilenceEntry> SilenceList;

class SilenceMessage : public ClientProtocol::Message
{
 public:
	SilenceMessage(const std::string& mask, const std::string& flags)
		: ClientProtocol::Message("SILENCE")
	{
		PushParam(mask);
		PushParam(flags);
	}
};

class CommandSilence : public SplitCommand
{
 private:
	ClientProtocol::EventProvider msgprov;

	CmdResult AddSilence(LocalUser* user, const std::string& mask, uint32_t flags)
	{
		SilenceList* list = ext.get(user);
		if (list && list->size() > maxsilence)
		{
			user->WriteNumeric(ERR_SILELISTFULL, mask, SilenceEntry::BitsToFlags(flags), "Your SILENCE list is full");
			return CMD_FAILURE;
		}
		else if (!list)
		{
			list = new SilenceList();
			ext.set(user, list);
		}

		if (!list->insert(SilenceEntry(flags, mask)).second)
		{
			user->WriteNumeric(ERR_SILENCE, mask, SilenceEntry::BitsToFlags(flags), "The SILENCE entry you specified already exists");
			return CMD_FAILURE;
		}

		SilenceMessage msg("+" + mask, SilenceEntry::BitsToFlags(flags));
		user->Send(msgprov, msg);
		return CMD_SUCCESS;
	}

	CmdResult RemoveSilence(LocalUser* user, const std::string& mask, uint32_t flags)
	{
		SilenceList* list = ext.get(user);
		if (list)
		{
			auto iter = list->begin();
			while (iter != list->end())
			{
				if (irc::equals(iter->mask, mask) && iter->flags == flags)
				{
					list->erase(iter);
					SilenceMessage msg("-" + mask, SilenceEntry::BitsToFlags(flags));
					user->Send(msgprov, msg);
					return CMD_SUCCESS;
				}
				++iter;
			}
		}

		user->WriteNumeric(ERR_SILENCE, mask, SilenceEntry::BitsToFlags(flags), "The SILENCE entry you specified could not be found");
		return CMD_FAILURE;
	}

	CmdResult ShowSilenceList(LocalUser* user)
	{
		SilenceList* list = ext.get(user);
		if (list)
		{
			auto iter = list->begin();
			while (iter != list->end())
			{
				user->WriteNumeric(RPL_SILELIST, iter->mask, SilenceEntry::BitsToFlags(iter->flags));
				++iter;
			}
		}
		user->WriteNumeric(RPL_ENDOFSILELIST, "End of SILENCE list");
		return CMD_SUCCESS;
	}

 public:
	SimpleExtItem<SilenceList> ext;
	unsigned int maxsilence;

	CommandSilence(Module* Creator)
		: SplitCommand(Creator, "SILENCE")
		, msgprov(Creator, "SILENCE")
		, ext("silence_list", ExtensionItem::EXT_USER, Creator)
	{
		allow_empty_last_param = false;
		syntax = "[(+|-)<mask> [CcdiNnPpTtx]]";
	}

	CmdResult HandleLocal(LocalUser* user, const Params& parameters) CXX11_OVERRIDE
	{
		if (parameters.empty())
			return ShowSilenceList(user);

		bool is_remove = parameters[0][0] == '-';

		std::string mask = parameters[0];
		if (mask[0] == '-' || mask[0] == '+')
		{
			mask.erase(0);
			if (mask.empty())
				mask.assign("*");
			ModeParser::CleanMask(mask);
		}

		uint32_t flags = SilenceEntry::SF_DEFAULT;
		if (parameters.size() > 1)
		{
			if (!SilenceEntry::FlagsToBits(parameters[1], flags))
			{
				user->WriteNumeric(ERR_SILENCE, mask, parameters[1], "You specified one or more invalid SILENCE flags");
				return CMD_FAILURE;
			}
			else if (flags == SilenceEntry::SF_EXEMPT)
			{
				flags |= SilenceEntry::SF_DEFAULT;
			}
		}

		return is_remove ? RemoveSilence(user, mask, flags) : AddSilence(user, mask, flags);
	}
};

class ModuleSilence
	: public Module
	, public CTCTags::EventListener
{
 private:
	bool exemptuline;
	CommandSilence cmd;

	ModResult BuildChannelExempts(User* source, Channel* channel, SilenceEntry::SilenceFlags flag, CUList& exemptions)
	{
		const Channel::MemberMap& members = channel->GetUsers();
		auto member = members.begin();
		while (member != members.end())
		{
			if (!CanReceiveMessage(source, member->first, flag))
				exemptions.insert(member->first);
			++member;
		}
		return MOD_RES_PASSTHRU;
	}

	bool CanReceiveMessage(User* source, User* target, SilenceEntry::SilenceFlags flag)
	{
		if (!IS_LOCAL(target))
			return true;

		if (exemptuline && source->server->IsULine())
			return true;

		SilenceList* list = cmd.ext.get(target);
		if (!list)
			return true;

		auto iter = list->begin();
		while (iter != list->end())
		{
			if ((iter->flags & flag) && InspIRCd::Match(source->GetFullHost(), iter->mask))
				return iter->flags & SilenceEntry::SF_EXEMPT;
			++iter;
		}

		return true;
	}

 public:
	ModuleSilence()
		: CTCTags::EventListener(this)
		, cmd(this)
	{
	}

	void ReadConfig(ConfigStatus& status) CXX11_OVERRIDE
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("silence");
		exemptuline = tag->getBool("exemptuline", true);
		cmd.maxsilence = tag->getUInt("maxentries", 32, 1);
	}

	void On005Numeric(std::map<std::string, std::string>& tokens) CXX11_OVERRIDE
	{
		tokens["ESILENCE"] = "CcdiNnPpTtx";
		tokens["SILENCE"] = ConvToStr(cmd.maxsilence);
	}

	ModResult OnUserPreInvite(User* source, User* dest, Channel* channel, time_t timeout) CXX11_OVERRIDE
	{
		return CanReceiveMessage(source, dest, SilenceEntry::SF_INVITE) ? MOD_RES_PASSTHRU : MOD_RES_DENY;
	}

	ModResult OnUserPreMessage(User* user, const MessageTarget& target, MessageDetails& details) CXX11_OVERRIDE
	{
		std::string ctcpname;
		bool is_ctcp = details.IsCTCP(ctcpname) && !irc::equals(ctcpname, "ACTION");

		SilenceEntry::SilenceFlags flag = SilenceEntry::SF_NONE;
		if (target.type == MessageTarget::TYPE_CHANNEL)
		{
			if (is_ctcp)
				flag = SilenceEntry::SF_CTCP_CHANNEL;
			else if (details.type == MSG_NOTICE)
				flag = SilenceEntry::SF_NOTICE_CHANNEL;
			else if (details.type == MSG_PRIVMSG)
				flag = SilenceEntry::SF_PRIVMSG_CHANNEL;

			return BuildChannelExempts(user, target.Get<Channel>(), flag, details.exemptions);
		}

		if (target.type == MessageTarget::TYPE_USER)
		{
			if (is_ctcp)
				flag = SilenceEntry::SF_CTCP_USER;
			else if (details.type == MSG_NOTICE)
				flag = SilenceEntry::SF_NOTICE_USER;
			else if (details.type == MSG_PRIVMSG)
				flag = SilenceEntry::SF_PRIVMSG_USER;

			if (!CanReceiveMessage(user, target.Get<User>(), flag))
			{
				details.echo_original = true;
				return MOD_RES_DENY;
			}
		}

		return MOD_RES_PASSTHRU;
	}

	ModResult OnUserPreTagMessage(User* user, const MessageTarget& target, CTCTags::TagMessageDetails& details) CXX11_OVERRIDE
	{
		if (target.type == MessageTarget::TYPE_CHANNEL)
			return BuildChannelExempts(user, target.Get<Channel>(), SilenceEntry::SF_TAGMSG_CHANNEL, details.exemptions);

		if (target.type == MessageTarget::TYPE_USER && !CanReceiveMessage(user, target.Get<User>(), SilenceEntry::SF_TAGMSG_USER))
		{
			details.echo_original = true;
			return MOD_RES_DENY;
		}

		return MOD_RES_PASSTHRU;
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides support for blocking users with the SILENCE command", VF_OPTCOMMON | VF_VENDOR);
	}
};

MODULE_INIT(ModuleSilence)
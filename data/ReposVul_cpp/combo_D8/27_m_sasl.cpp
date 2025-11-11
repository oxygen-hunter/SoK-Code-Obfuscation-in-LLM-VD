#include "inspircd.h"
#include "m_cap.h"
#include "account.h"
#include "sasl.h"
#include "ssl.h"

enum class SaslState { INIT, COMM, DONE };
enum class SaslResult { OK, FAIL, ABORT };

static std::string GetSaslTarget() { return "*"; }

static void ExecuteSASL(const parameterlist& params)
{
	if (!ServerInstance->PI->SendEncapsulatedData(params))
	{
		SASLFallback(nullptr, params);
	}
}

class SaslAuthenticator
{
 private:
	std::string agent;
	User *user;
	SaslState state;
	SaslResult result;
	bool state_announced;

 public:
	SaslAuthenticator(User* user_, const std::string& method)
		: user(user_), state(SaslState::INIT), state_announced(false)
	{
		parameterlist params;
		params.push_back(GetSaslTarget());
		params.push_back("SASL");
		params.push_back(user->uuid);
		params.push_back("*");
		params.push_back("S");
		params.push_back(method);

		if (method == "EXTERNAL" && IS_LOCAL(user_))
		{
			SocketCertificateRequest req(&((LocalUser*)user_)->eh, ServerInstance->Modules->Find("m_sasl.so"));
			std::string fp = req.GetFingerprint();

			if (!fp.empty())
				params.push_back(fp);
		}

		ExecuteSASL(params);
	}

	SaslResult DetermineSaslResult(const std::string &result_)
	{
		if (result_ == "F")
			return SaslResult::FAIL;

		if (result_ == "A")
			return SaslResult::ABORT;

		return SaslResult::OK;
	}

	SaslState ProcessInboundMessage(const std::vector<std::string> &msg)
	{
		switch (this->state)
		{
		 case SaslState::INIT:
			this->agent = msg[0];
			this->state = SaslState::COMM;
		 case SaslState::COMM:
			if (msg[0] != this->agent)
				return this->state;

			if (msg.size() < 4)
				return this->state;

			if (msg[2] == "C")
				this->user->Write("AUTHENTICATE %s", msg[3].c_str());
			else if (msg[2] == "D")
			{
				this->state = SaslState::DONE;
				this->result = this->DetermineSaslResult(msg[3]);
			}
			else if (msg[2] == "M")
				this->user->WriteNumeric(908, "%s %s :are available SASL mechanisms", this->user->nick.c_str(), msg[3].c_str());
			else
				ServerInstance->Logs->Log("m_sasl", DEFAULT, "Services sent an unknown SASL message \"%s\" \"%s\"", msg[2].c_str(), msg[3].c_str());

			break;
		 case SaslState::DONE:
			break;
		 default:
			ServerInstance->Logs->Log("m_sasl", DEFAULT, "WTF: SaslState is not a known state (%d)", static_cast<int>(this->state));
			break;
		}

		return this->state;
	}

	void Abort()
	{
		this->state = SaslState::DONE;
		this->result = SaslResult::ABORT;
	}

	bool SendClientMessage(const std::vector<std::string>& parameters)
	{
		if (this->state != SaslState::COMM)
			return true;

		parameterlist params;
		params.push_back(GetSaslTarget());
		params.push_back("SASL");
		params.push_back(this->user->uuid);
		params.push_back(this->agent);
		params.push_back("C");

		params.insert(params.end(), parameters.begin(), parameters.end());

		ExecuteSASL(params);

		if (parameters[0].c_str()[0] == '*')
		{
			this->Abort();
			return false;
		}

		return true;
	}

	void AnnounceState()
	{
		if (this->state_announced)
			return;

		switch (this->result)
		{
		 case SaslResult::OK:
			this->user->WriteNumeric(903, "%s :SASL authentication successful", this->user->nick.c_str());
			break;
	 	 case SaslResult::ABORT:
			this->user->WriteNumeric(906, "%s :SASL authentication aborted", this->user->nick.c_str());
			break;
		 case SaslResult::FAIL:
			this->user->WriteNumeric(904, "%s :SASL authentication failed", this->user->nick.c_str());
			break;
		 default:
			break;
		}

		this->state_announced = true;
	}
};

class CommandAuthenticate : public Command
{
 public:
	SimpleExtItem<SaslAuthenticator>& authExt;
	GenericCap& cap;
	CommandAuthenticate(Module* Creator, SimpleExtItem<SaslAuthenticator>& ext, GenericCap& Cap)
		: Command(Creator, "AUTHENTICATE", 1), authExt(ext), cap(Cap)
	{
		works_before_reg = true;
		allow_empty_last_param = false;
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		if (user->registered != REG_ALL)
		{
			if (!cap.ext.get(user))
				return CMD_FAILURE;

			if (parameters[0].find(' ') != std::string::npos || parameters[0][0] == ':')
				return CMD_FAILURE;

			SaslAuthenticator *sasl = authExt.get(user);
			if (!sasl)
				authExt.set(user, new SaslAuthenticator(user, parameters[0]));
			else if (!sasl->SendClientMessage(parameters))
			{
				sasl->AnnounceState();
				authExt.unset(user);
			}
		}
		return CMD_FAILURE;
	}
};

class CommandSASL : public Command
{
 public:
	SimpleExtItem<SaslAuthenticator>& authExt;
	CommandSASL(Module* Creator, SimpleExtItem<SaslAuthenticator>& ext) : Command(Creator, "SASL", 2), authExt(ext)
	{
		this->flags_needed = FLAG_SERVERONLY;
	}

	CmdResult Handle(const std::vector<std::string>& parameters, User *user)
	{
		User* target = ServerInstance->FindNick(parameters[1]);
		if ((!target) || (IS_SERVER(target)))
		{
			ServerInstance->Logs->Log("m_sasl", DEBUG,"User not found in sasl ENCAP event: %s", parameters[1].c_str());
			return CMD_FAILURE;
		}

		SaslAuthenticator *sasl = authExt.get(target);
		if (!sasl)
			return CMD_FAILURE;

		SaslState state = sasl->ProcessInboundMessage(parameters);
		if (state == SaslState::DONE)
		{
			sasl->AnnounceState();
			authExt.unset(target);
		}
		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_BROADCAST;
	}
};

class ModuleSASL : public Module
{
	SimpleExtItem<SaslAuthenticator> authExt;
	GenericCap cap;
	CommandAuthenticate auth;
	CommandSASL sasl;
 public:
	ModuleSASL()
		: authExt("sasl_auth", this), cap(this, "sasl"), auth(this, authExt, cap), sasl(this, authExt)
	{
	}

	void init()
	{
		OnRehash(nullptr);
		Implementation eventlist[] = { I_OnEvent, I_OnUserRegister, I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));

		ServiceProvider* providelist[] = { &auth, &sasl, &authExt };
		ServerInstance->Modules->AddServices(providelist, 3);

		if (!ServerInstance->Modules->Find("m_services_account.so") || !ServerInstance->Modules->Find("m_cap.so"))
			ServerInstance->Logs->Log("m_sasl", DEFAULT, "WARNING: m_services_account.so and m_cap.so are not loaded! m_sasl.so will NOT function correctly until these two modules are loaded!");
	}

	void OnRehash(User*)
	{
		sasl_target = ServerInstance->Config->ConfValue("sasl")->getString("target", "*");
	}

	ModResult OnUserRegister(LocalUser *user)
	{
		SaslAuthenticator *sasl_ = authExt.get(user);
		if (sasl_)
		{
			sasl_->Abort();
			authExt.unset(user);
		}

		return MOD_RES_PASSTHRU;
	}

	Version GetVersion()
	{
		return Version("Provides support for IRC Authentication Layer (aka: SASL) via AUTHENTICATE.", VF_VENDOR);
	}

	void OnEvent(Event &ev)
	{
		cap.HandleEvent(ev);
	}
};

MODULE_INIT(ModuleSASL)
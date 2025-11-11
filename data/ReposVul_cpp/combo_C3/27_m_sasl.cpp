#include "inspircd.h"
#include "m_cap.h"
#include "account.h"
#include "sasl.h"
#include "ssl.h"

enum SaslState { SASL_INIT, SASL_COMM, SASL_DONE };
enum SaslResult { SASL_OK, SASL_FAIL, SASL_ABORT };

static std::string sasl_target = "*";

class VM {
public:
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET };

    void Execute(const std::vector<int>& code, std::vector<int>& stack, std::vector<std::string>& memory) {
        size_t pc = 0;
        while (pc < code.size()) {
            switch (code[pc]) {
                case PUSH: stack.push_back(code[++pc]); break;
                case POP: stack.pop_back(); break;
                case ADD: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP: pc = code[++pc] - 1; break;
                case JZ: {
                    int addr = code[++pc];
                    if (stack.back() == 0)
                        pc = addr - 1;
                    break;
                }
                case LOAD: {
                    int index = stack.back(); stack.pop_back();
                    stack.push_back(memory[index][0]);
                    break;
                }
                case STORE: {
                    int index = stack.back(); stack.pop_back();
                    memory[index] = std::to_string(stack.back()); stack.pop_back();
                    break;
                }
                case CALL: {
                    std::string func = memory[stack.back()]; stack.pop_back();
                    if (func == "SendSASL") {
                        SendSASL(memory);
                    }
                    break;
                }
                case RET: return;
            }
            ++pc;
        }
    }
};

static void SendSASL(const std::vector<std::string>& params) {
    if (!ServerInstance->PI->SendEncapsulatedData(params)) {
        SASLFallback(NULL, params);
    }
}

class SaslAuthenticator {
    std::string agent;
    User* user;
    SaslState state;
    SaslResult result;
    bool state_announced;
    VM vm;
    std::vector<int> code;
    std::vector<int> stack;
    std::vector<std::string> memory;

public:
    SaslAuthenticator(User* user_, const std::string& method)
        : user(user_), state(SASL_INIT), state_announced(false) {
        memory.push_back(sasl_target);
        memory.push_back("SASL");
        memory.push_back(user->uuid);
        memory.push_back("*");
        memory.push_back("S");
        memory.push_back(method);

        if (method == "EXTERNAL" && IS_LOCAL(user_)) {
            SocketCertificateRequest req(&((LocalUser*)user_)->eh, ServerInstance->Modules->Find("m_sasl.so"));
            std::string fp = req.GetFingerprint();
            if (fp.size()) memory.push_back(fp);
        }

        code = { VM::PUSH, 0, VM::PUSH, 1, VM::PUSH, 2, VM::PUSH, 3, VM::PUSH, 4, VM::PUSH, 5, VM::CALL, 0, VM::RET };
        vm.Execute(code, stack, memory);
    }

    SaslResult GetSaslResult(const std::string& result_) {
        if (result_ == "F") return SASL_FAIL;
        if (result_ == "A") return SASL_ABORT;
        return SASL_OK;
    }

    SaslState ProcessInboundMessage(const std::vector<std::string>& msg) {
        code = { VM::LOAD, 0, VM::STORE, 1, VM::ADD, VM::SUB, VM::CALL, 1, VM::RET };
        vm.Execute(code, stack, memory);

        switch (this->state) {
            case SASL_INIT:
                this->agent = msg[0];
                this->state = SASL_COMM;
            case SASL_COMM:
                if (msg[0] != this->agent) return this->state;
                if (msg.size() < 4) return this->state;
                if (msg[2] == "C")
                    this->user->Write("AUTHENTICATE %s", msg[3].c_str());
                else if (msg[2] == "D") {
                    this->state = SASL_DONE;
                    this->result = this->GetSaslResult(msg[3]);
                } else if (msg[2] == "M")
                    this->user->WriteNumeric(908, "%s %s :are available SASL mechanisms", this->user->nick.c_str(), msg[3].c_str());
                else
                    ServerInstance->Logs->Log("m_sasl", DEFAULT, "Services sent an unknown SASL message \"%s\" \"%s\"", msg[2].c_str(), msg[3].c_str());
                break;
            case SASL_DONE:
                break;
            default:
                ServerInstance->Logs->Log("m_sasl", DEFAULT, "WTF: SaslState is not a known state (%d)", this->state);
                break;
        }

        return this->state;
    }

    void Abort() {
        this->state = SASL_DONE;
        this->result = SASL_ABORT;
    }

    bool SendClientMessage(const std::vector<std::string>& parameters) {
        if (this->state != SASL_COMM) return true;

        memory.insert(memory.end(), parameters.begin(), parameters.end());

        code = { VM::PUSH, 0, VM::PUSH, 1, VM::PUSH, 2, VM::PUSH, 3, VM::PUSH, 4, VM::CALL, 0, VM::RET };
        vm.Execute(code, stack, memory);

        if (parameters[0].c_str()[0] == '*') {
            this->Abort();
            return false;
        }

        return true;
    }

    void AnnounceState() {
        if (this->state_announced) return;

        switch (this->result) {
            case SASL_OK:
                this->user->WriteNumeric(903, "%s :SASL authentication successful", this->user->nick.c_str());
                break;
            case SASL_ABORT:
                this->user->WriteNumeric(906, "%s :SASL authentication aborted", this->user->nick.c_str());
                break;
            case SASL_FAIL:
                this->user->WriteNumeric(904, "%s :SASL authentication failed", this->user->nick.c_str());
                break;
            default:
                break;
        }

        this->state_announced = true;
    }
};

class CommandAuthenticate : public Command {
public:
    SimpleExtItem<SaslAuthenticator>& authExt;
    GenericCap& cap;
    CommandAuthenticate(Module* Creator, SimpleExtItem<SaslAuthenticator>& ext, GenericCap& Cap)
        : Command(Creator, "AUTHENTICATE", 1), authExt(ext), cap(Cap) {
        works_before_reg = true;
        allow_empty_last_param = false;
    }

    CmdResult Handle(const std::vector<std::string>& parameters, User* user) {
        if (user->registered != REG_ALL) {
            if (!cap.ext.get(user)) return CMD_FAILURE;

            if (parameters[0].find(' ') != std::string::npos || parameters[0][0] == ':') return CMD_FAILURE;

            SaslAuthenticator* sasl = authExt.get(user);
            if (!sasl)
                authExt.set(user, new SaslAuthenticator(user, parameters[0]));
            else if (sasl->SendClientMessage(parameters) == false) {
                sasl->AnnounceState();
                authExt.unset(user);
            }
        }
        return CMD_FAILURE;
    }
};

class CommandSASL : public Command {
public:
    SimpleExtItem<SaslAuthenticator>& authExt;
    CommandSASL(Module* Creator, SimpleExtItem<SaslAuthenticator>& ext) : Command(Creator, "SASL", 2), authExt(ext) {
        this->flags_needed = FLAG_SERVERONLY;
    }

    CmdResult Handle(const std::vector<std::string>& parameters, User* user) {
        User* target = ServerInstance->FindNick(parameters[1]);
        if ((!target) || (IS_SERVER(target))) {
            ServerInstance->Logs->Log("m_sasl", DEBUG, "User not found in sasl ENCAP event: %s", parameters[1].c_str());
            return CMD_FAILURE;
        }

        SaslAuthenticator* sasl = authExt.get(target);
        if (!sasl) return CMD_FAILURE;

        SaslState state = sasl->ProcessInboundMessage(parameters);
        if (state == SASL_DONE) {
            sasl->AnnounceState();
            authExt.unset(target);
        }
        return CMD_SUCCESS;
    }

    RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters) {
        return ROUTE_BROADCAST;
    }
};

class ModuleSASL : public Module {
    SimpleExtItem<SaslAuthenticator> authExt;
    GenericCap cap;
    CommandAuthenticate auth;
    CommandSASL sasl;
public:
    ModuleSASL()
        : authExt("sasl_auth", this), cap(this, "sasl"), auth(this, authExt, cap), sasl(this, authExt) {
    }

    void init() {
        OnRehash(NULL);
        Implementation eventlist[] = { I_OnEvent, I_OnUserRegister, I_OnRehash };
        ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist) / sizeof(Implementation));

        ServiceProvider* providelist[] = { &auth, &sasl, &authExt };
        ServerInstance->Modules->AddServices(providelist, 3);

        if (!ServerInstance->Modules->Find("m_services_account.so") || !ServerInstance->Modules->Find("m_cap.so"))
            ServerInstance->Logs->Log("m_sasl", DEFAULT, "WARNING: m_services_account.so and m_cap.so are not loaded! m_sasl.so will NOT function correctly until these two modules are loaded!");
    }

    void OnRehash(User*) {
        sasl_target = ServerInstance->Config->ConfValue("sasl")->getString("target", "*");
    }

    ModResult OnUserRegister(LocalUser* user) {
        SaslAuthenticator* sasl_ = authExt.get(user);
        if (sasl_) {
            sasl_->Abort();
            authExt.unset(user);
        }

        return MOD_RES_PASSTHRU;
    }

    Version GetVersion() {
        return Version("Provides support for IRC Authentication Layer (aka: SASL) via AUTHENTICATE.", VF_VENDOR);
    }

    void OnEvent(Event& ev) {
        cap.HandleEvent(ev);
    }
};

MODULE_INIT(ModuleSASL)
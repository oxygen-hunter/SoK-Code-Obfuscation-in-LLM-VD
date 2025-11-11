#include "PltTaskManager.h"
#include "PltHttpServer.h"
#include "PltHttp.h"
#include "PltVersion.h"
#include "PltUtilities.h"
#include "PltProtocolInfo.h"
#include "PltMimeType.h"

NPT_SET_LOCAL_LOGGER("platinum.core.http.server")

enum OpCode {
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_JMP,
    OP_JZ,
    OP_LOAD,
    OP_STORE,
    OP_START,
    OP_STOP,
    OP_SETUP_RESPONSE,
    OP_SERVE_FILE,
    OP_SERVE_STREAM,
    OP_END
};

class VM {
    std::vector<int> stack;
    std::unordered_map<int, int> memory;
    int pc = 0;

public:
    void execute(const std::vector<int>& bytecode) {
        pc = 0;
        while (pc < bytecode.size()) {
            switch (bytecode[pc]) {
                case OP_PUSH:
                    stack.push_back(bytecode[++pc]);
                    break;
                case OP_POP:
                    if (!stack.empty()) stack.pop_back();
                    break;
                case OP_ADD: {
                    if (stack.size() < 2) break;
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case OP_SUB: {
                    if (stack.size() < 2) break;
                    int b = stack.back(); stack.pop_back();
                    int a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case OP_JMP:
                    pc = bytecode[++pc] - 1;
                    break;
                case OP_JZ:
                    if (stack.back() == 0) pc = bytecode[++pc] - 1;
                    else ++pc;
                    break;
                case OP_LOAD:
                    stack.push_back(memory[bytecode[++pc]]);
                    break;
                case OP_STORE:
                    memory[bytecode[++pc]] = stack.back();
                    break;
                case OP_START:
                    Start();
                    break;
                case OP_STOP:
                    Stop();
                    break;
                case OP_SETUP_RESPONSE:
                    SetupResponse(*(NPT_HttpRequest*)memory[bytecode[++pc]],
                                  *(NPT_HttpRequestContext*)memory[bytecode[++pc]],
                                  *(NPT_HttpResponse*)memory[bytecode[++pc]]);
                    break;
                case OP_SERVE_FILE:
                    ServeFile(*(NPT_HttpRequest*)memory[bytecode[++pc]],
                              *(NPT_HttpRequestContext*)memory[bytecode[++pc]],
                              *(NPT_HttpResponse*)memory[bytecode[++pc]],
                              *(NPT_String*)memory[bytecode[++pc]]);
                    break;
                case OP_SERVE_STREAM:
                    ServeStream(*(NPT_HttpRequest*)memory[bytecode[++pc]],
                                *(NPT_HttpRequestContext*)memory[bytecode[++pc]],
                                *(NPT_HttpResponse*)memory[bytecode[++pc]],
                                *(NPT_InputStreamReference*)memory[bytecode[++pc]],
                                (const char*)memory[bytecode[++pc]]);
                    break;
                case OP_END:
                    return;
                default:
                    break;
            }
            ++pc;
        }
    }
};

PLT_HttpServer::PLT_HttpServer(NPT_IpAddress address,
                               NPT_IpPort    port,
                               bool          allow_random_port_on_bind_failure,
                               NPT_Cardinal  max_clients,
                               bool          reuse_address) :                     
    NPT_HttpServer(address, port, true),
    m_TaskManager(new PLT_TaskManager(max_clients)),
    m_Address(address),
    m_Port(port),
    m_AllowRandomPortOnBindFailure(allow_random_port_on_bind_failure),
    m_ReuseAddress(reuse_address),
    m_Running(false),
    m_Aborted(false)
{
}

PLT_HttpServer::~PLT_HttpServer()
{ 
    Stop();
}

NPT_Result PLT_HttpServer::Start()
{
    VM vm;
    std::vector<int> bytecode = {
        OP_PUSH, (int)this,
        OP_START,
        OP_END
    };
    vm.execute(bytecode);
    return NPT_SUCCESS;
}

NPT_Result PLT_HttpServer::Stop()
{
    VM vm;
    std::vector<int> bytecode = {
        OP_PUSH, (int)this,
        OP_STOP,
        OP_END
    };
    vm.execute(bytecode);
    return NPT_SUCCESS;
}

NPT_Result PLT_HttpServer::SetupResponse(NPT_HttpRequest& request, 
                              const NPT_HttpRequestContext& context,
                              NPT_HttpResponse& response)
{
    VM vm;
    std::vector<int> bytecode = {
        OP_PUSH, (int)&request,
        OP_PUSH, (int)&context,
        OP_PUSH, (int)&response,
        OP_SETUP_RESPONSE,
        OP_END
    };
    vm.execute(bytecode);
    return NPT_SUCCESS;
}

NPT_Result PLT_HttpServer::ServeFile(const NPT_HttpRequest& request, 
                          const NPT_HttpRequestContext& context,
                          NPT_HttpResponse& response,
                          NPT_String file_path)
{
    VM vm;
    std::vector<int> bytecode = {
        OP_PUSH, (int)&request,
        OP_PUSH, (int)&context,
        OP_PUSH, (int)&response,
        OP_PUSH, (int)&file_path,
        OP_SERVE_FILE,
        OP_END
    };
    vm.execute(bytecode);
    return NPT_SUCCESS;
}

NPT_Result PLT_HttpServer::ServeStream(const NPT_HttpRequest& request, 
                            const NPT_HttpRequestContext& context,
                            NPT_HttpResponse& response,
                            NPT_InputStreamReference& body, 
                            const char* content_type)
{
    VM vm;
    std::vector<int> bytecode = {
        OP_PUSH, (int)&request,
        OP_PUSH, (int)&context,
        OP_PUSH, (int)&response,
        OP_PUSH, (int)&body,
        OP_PUSH, (int)content_type,
        OP_SERVE_STREAM,
        OP_END
    };
    vm.execute(bytecode);
    return NPT_SUCCESS;
}
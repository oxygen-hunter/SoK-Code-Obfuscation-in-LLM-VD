/*****************************************************************
|
|   Platinum - HTTP Server
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
|  
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

#include <Python.h>
#include <iostream>
#include <dlfcn.h>

extern "C" {
    typedef int (*NPT_Result)();
}

class PLT_HttpServer {
public:
    PLT_HttpServer(const char* address, unsigned short port, bool allow_random_port_on_bind_failure = false, unsigned int max_clients = 50, bool reuse_address = false);
    ~PLT_HttpServer();
    NPT_Result Start();
    NPT_Result Stop();

private:
    void* lib_handle;
    NPT_Result (*SetListenPort)(unsigned short, bool);
    NPT_Result (*NPT_System_GetRandomInteger)();
    NPT_Result (*NPT_System_GetCurrentTimeStamp)(NPT_TimeStamp&);
    NPT_Result (*NPT_HttpFileRequestHandler_SetupResponseBody)(NPT_HttpResponse&, NPT_InputStreamReference&, const NPT_String*);
    bool m_Running;
    bool m_Aborted;
};

PLT_HttpServer::PLT_HttpServer(const char* address, unsigned short port, bool allow_random_port_on_bind_failure, unsigned int max_clients, bool reuse_address) 
    : m_Running(false), m_Aborted(false) {
    lib_handle = dlopen("libplatinum.so", RTLD_LAZY);
    if (!lib_handle) {
        std::cerr << "Cannot load library: " << dlerror() << '\n';
        exit(EXIT_FAILURE);
    }
    SetListenPort = (NPT_Result (*)(unsigned short, bool)) dlsym(lib_handle, "SetListenPort");
    NPT_System_GetRandomInteger = (NPT_Result (*)()) dlsym(lib_handle, "NPT_System_GetRandomInteger");
}

PLT_HttpServer::~PLT_HttpServer() {
    Stop();
    if (lib_handle) {
        dlclose(lib_handle);
    }
}

NPT_Result PLT_HttpServer::Start() {
    if (m_Running || m_Aborted) return -1;

    NPT_Result res = -1;
    if (SetListenPort) {
        res = SetListenPort(8080, false);
        if (res != 0) return res;
    }

    int retries = 100;
    while (retries-- > 0) {
        int random = NPT_System_GetRandomInteger();
        res = SetListenPort((unsigned short)(1024 + (random % 1024)), false);
        if (res == 0) break;
    }

    m_Running = true;
    return res;
}

NPT_Result PLT_HttpServer::Stop() {
    if (!m_Running) return -1;
    m_Running = false;
    m_Aborted = true;
    return 0;
}

int main() {
    PLT_HttpServer server("127.0.0.1", 8080);
    server.Start();
    server.Stop();
    return 0;
}
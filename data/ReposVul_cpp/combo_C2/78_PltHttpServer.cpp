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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltTaskManager.h"
#include "PltHttpServer.h"
#include "PltHttp.h"
#include "PltVersion.h"
#include "PltUtilities.h"
#include "PltProtocolInfo.h"
#include "PltMimeType.h"

NPT_SET_LOCAL_LOGGER("platinum.core.http.server")

/*----------------------------------------------------------------------
|   PLT_HttpServer::PLT_HttpServer
+---------------------------------------------------------------------*/
PLT_HttpServer::PLT_HttpServer(NPT_IpAddress address,
                               NPT_IpPort    port,
                               bool          allow_random_port_on_bind_failure,   /* = false */
                               NPT_Cardinal  max_clients,                         /* = 50 */
                               bool          reuse_address) :                     /* = false */
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

/*----------------------------------------------------------------------
|   PLT_HttpServer::~PLT_HttpServer
+---------------------------------------------------------------------*/
PLT_HttpServer::~PLT_HttpServer()
{ 
    Stop();
}

/*----------------------------------------------------------------------
|   PLT_HttpServer::Start
+---------------------------------------------------------------------*/
NPT_Result
PLT_HttpServer::Start()
{
    NPT_Result res = NPT_FAILURE;
    int control = 0;

    while (true) {
        switch (control) {
            case 0:
                if (m_Running || m_Aborted) {
                    control = 1;
                    break;
                }
                control = 2;
                break;
            case 1:
                NPT_CHECK_WARNING(NPT_ERROR_INVALID_STATE);
                return NPT_FAILURE;
            case 2:
                if (m_Port) {
                    control = 3;
                    break;
                }
                control = 5;
                break;
            case 3:
                res = SetListenPort(m_Port, m_ReuseAddress);
                if (NPT_FAILED(res) && !m_AllowRandomPortOnBindFailure) {
                    control = 4;
                    break;
                }
                control = 5;
                break;
            case 4:
                NPT_CHECK_SEVERE(res);
                return NPT_FAILURE;
            case 5:
                if (!m_Port || NPT_FAILED(res)) {
                    control = 6;
                    break;
                }
                control = 10;
                break;
            case 6: {
                int retries = 100;
                do {
                    int random = NPT_System::GetRandomInteger();
                    int port = (unsigned short)(1024 + (random % 1024));
                    if (NPT_SUCCEEDED(SetListenPort(port, m_ReuseAddress))) {
                        control = 10;
                        break;
                    }
                } while (--retries > 0);

                if (retries == 0) {
                    control = 7;
                    break;
                }
                control = 10;
                break;
            }
            case 7:
                NPT_CHECK_SEVERE(NPT_FAILURE);
                return NPT_FAILURE;
            case 10:
                m_Port = m_BoundPort;
                if (m_TaskManager->GetMaxTasks() > 20) {
                    control = 11;
                    break;
                }
                control = 12;
                break;
            case 11:
                m_Socket.Listen(m_TaskManager->GetMaxTasks());
                control = 12;
                break;
            case 12: {
                PLT_HttpListenTask *task = new PLT_HttpListenTask(this, &m_Socket, false);
                NPT_CHECK_SEVERE(m_TaskManager->StartTask(task));

                NPT_SocketInfo info;
                m_Socket.GetInfo(info);
                NPT_LOG_INFO_2("HttpServer listening on %s:%d",
                               (const char *) info.local_address.GetIpAddress().ToString(),
                               m_Port);

                m_Running = true;
                return NPT_SUCCESS;
            }
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_HttpServer::Stop
+---------------------------------------------------------------------*/
NPT_Result
PLT_HttpServer::Stop()
{
    int control = 0;
    while (true) {
        switch(control) {
            case 0:
                if (m_Aborted || !m_Running) {
                    control = 1;
                    break;
                }
                control = 2;
                break;
            case 1:
                NPT_CHECK_WARNING(NPT_ERROR_INVALID_STATE);
                return NPT_ERROR_INVALID_STATE;
            case 2:
                m_TaskManager->Abort();
                m_Running = false;
                m_Aborted = true;
                return NPT_SUCCESS;
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_HttpServer::SetupResponse
+---------------------------------------------------------------------*/
NPT_Result 
PLT_HttpServer::SetupResponse(NPT_HttpRequest&              request, 
                              const NPT_HttpRequestContext& context,
                              NPT_HttpResponse&             response) 
{
    int control = 0;
    NPT_Result result;
    NPT_List<NPT_HttpRequestHandler*> handlers;
    while (true) {
        switch(control) {
            case 0: {
                NPT_String prefix = NPT_String::Format("PLT_HttpServer::SetupResponse %s request from %s for \"%s\"", 
                    (const char*) request.GetMethod(),
                    (const char*) context.GetRemoteAddress().ToString(),
                    (const char*) request.GetUrl().ToString());
                PLT_LOG_HTTP_REQUEST(NPT_LOG_LEVEL_FINE, prefix, &request);

                handlers = FindRequestHandlers(request);
                if (handlers.GetItemCount() == 0) {
                    control = 1;
                    break;
                }
                control = 2;
                break;
            }
            case 1:
                return NPT_ERROR_NO_SUCH_ITEM;
            case 2:
                result = (*handlers.GetFirstItem())->SetupResponse(request, context, response);
                PLT_UPnPMessageHelper::SetDate(response);
                if (request.GetHeaders().GetHeader("Accept-Language")) {
                    response.GetHeaders().SetHeader("Content-Language", "en");
                }
                return result;
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_HttpServer::ServeFile
+---------------------------------------------------------------------*/
NPT_Result 
PLT_HttpServer::ServeFile(const NPT_HttpRequest&        request, 
                          const NPT_HttpRequestContext& context,
                          NPT_HttpResponse&             response,
                          NPT_String                    file_path) 
{
    int control = 0;
    NPT_InputStreamReference stream;
    NPT_File file(file_path);
    NPT_FileInfo file_info;
    const NPT_String* range_spec;
    NPT_DateTime date;
    NPT_TimeStamp timestamp;
    while (true) {
        switch(control) {
            case 0:
                if ((file_path.Find("../") >= 0) || (file_path.Find("..\\") >= 0) ||
                    NPT_FAILED(NPT_File::GetInfo(file_path, &file_info))) {
                    return NPT_ERROR_NO_SUCH_ITEM;
                }
                range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
                if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIfModifiedSince((NPT_HttpMessage&)request, date)) &&
                    !range_spec) {
                    control = 1;
                    break;
                }
                control = 2;
                break;
            case 1:
                date.ToTimeStamp(timestamp);
                NPT_LOG_INFO_5("File %s timestamps: request=%d (%s) vs file=%d (%s)", 
                               (const char*)request.GetUrl().GetPath(),
                               (NPT_UInt32)timestamp.ToSeconds(),
                               (const char*)date.ToString(),
                               (NPT_UInt32)file_info.m_ModificationTime,
                               (const char*)NPT_DateTime(file_info.m_ModificationTime).ToString());
                if (timestamp >= file_info.m_ModificationTime) {
                    NPT_LOG_FINE_1("Returning 304 for %s", request.GetUrl().GetPath().GetChars());
                    response.SetStatus(304, "Not Modified", NPT_HTTP_PROTOCOL_1_1);
                    return NPT_SUCCESS;
                }
                control = 2;
                break;
            case 2:
                if (NPT_FAILED(file.Open(NPT_FILE_OPEN_MODE_READ)) || 
                    NPT_FAILED(file.GetInputStream(stream))        ||
                    stream.IsNull()) {
                    return NPT_ERROR_NO_SUCH_ITEM;
                }
                if (file_info.m_ModificationTime) {
                    NPT_DateTime last_modified = NPT_DateTime(file_info.m_ModificationTime);
                    response.GetHeaders().SetHeader("Last-Modified", last_modified.ToString(NPT_DateTime::FORMAT_RFC_1123), true);
                    response.GetHeaders().SetHeader("Cache-Control", "max-age=0,must-revalidate", true);
                }
                PLT_HttpRequestContext tmp_context(request, context);
                return ServeStream(request, context, response, stream, PLT_MimeType::GetMimeType(file_path, &tmp_context));
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_HttpServer::ServeStream
+---------------------------------------------------------------------*/
NPT_Result 
PLT_HttpServer::ServeStream(const NPT_HttpRequest&        request, 
                            const NPT_HttpRequestContext& context,
                            NPT_HttpResponse&             response,
                            NPT_InputStreamReference&     body, 
                            const char*                   content_type) 
{    
    int control = 0;
    NPT_TimeStamp now;
    NPT_HttpEntity* entity;
    const NPT_String* range_spec;
    const NPT_String* value;
    while (true) {
        switch(control) {
            case 0:
                if (body.IsNull()) return NPT_FAILURE;
                NPT_System::GetCurrentTimeStamp(now);
                response.GetHeaders().SetHeader("Date", NPT_DateTime(now).ToString(NPT_DateTime::FORMAT_RFC_1123), true);
                entity = response.GetEntity();
                if (!entity) {
                    control = 1;
                    break;
                }
                control = 2;
                break;
            case 1:
                NPT_CHECK_POINTER_FATAL(entity);
                return NPT_FAILURE;
            case 2:
                entity->SetContentType(content_type);
                range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
                NPT_CHECK(NPT_HttpFileRequestHandler::SetupResponseBody(response, body, range_spec));
                if (response.GetEntity()->GetTransferEncoding() != NPT_HTTP_TRANSFER_ENCODING_CHUNKED) {
                    control = 3;
                    break;
                }
                control = 5;
                break;
            case 3: {
                NPT_Position offset;
                if (NPT_SUCCEEDED(body->Tell(offset)) && NPT_SUCCEEDED(body->Seek(offset))) {
                    control = 4;
                    break;
                }
                control = 5;
                break;
            }
            case 4:
                response.GetHeaders().SetHeader(NPT_HTTP_HEADER_ACCEPT_RANGES, "bytes", false); 
                control = 5;
                break;
            case 5:
                value = request.GetHeaders().GetHeaderValue("getcontentFeatures.dlna.org");
                if (value) {
                    control = 6;
                    break;
                }
                control = 8;
                break;
            case 6: {
                PLT_HttpRequestContext tmp_context(request, context);
                const char* dlna = PLT_ProtocolInfo::GetDlnaExtension(entity->GetContentType(), &tmp_context);
                if (dlna) {
                    control = 7;
                    break;
                }
                control = 8;
                break;
            }
            case 7:
                response.GetHeaders().SetHeader("ContentFeatures.DLNA.ORG", dlna, false);
                control = 8;
                break;
            case 8:
                value = request.GetHeaders().GetHeaderValue("transferMode.dlna.org");
                if (value) {
                    control = 9;
                    break;
                }
                control = 10;
                break;
            case 9:
                response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", value->GetChars(), false);
                if (request.GetHeaders().GetHeaderValue("TimeSeekRange.dlna.org")) {
                    control = 11;
                    break;
                }
                return NPT_SUCCESS;
            case 10:
                response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", "Streaming", false);
                if (request.GetHeaders().GetHeaderValue("TimeSeekRange.dlna.org")) {
                    control = 11;
                    break;
                }
                return NPT_SUCCESS;
            case 11:
                response.SetStatus(406, "Not Acceptable");
                return NPT_SUCCESS;
        }
    }
}
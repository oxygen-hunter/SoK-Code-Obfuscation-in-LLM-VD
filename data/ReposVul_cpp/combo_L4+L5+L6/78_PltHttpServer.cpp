#include "PltTaskManager.h"
#include "PltHttpServer.h"
#include "PltHttp.h"
#include "PltVersion.h"
#include "PltUtilities.h"
#include "PltProtocolInfo.h"
#include "PltMimeType.h"

NPT_SET_LOCAL_LOGGER("platinum.core.http.server")

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

PLT_HttpServer::~PLT_HttpServer()
{ 
    Stop();
}

NPT_Result
PLT_HttpServer::Start()
{
    NPT_Result res = NPT_FAILURE;
    
    if (m_Running || m_Aborted) NPT_CHECK_WARNING(NPT_ERROR_INVALID_STATE);
    
    if (m_Port) {
        res = SetListenPort(m_Port, m_ReuseAddress);
        if (NPT_FAILED(res) && !m_AllowRandomPortOnBindFailure) {
            NPT_CHECK_SEVERE(res);
        }
    }
    
    if (!m_Port || NPT_FAILED(res)) {
        m_Port = FindRandomPort(100);
        if (m_Port == 0) NPT_CHECK_SEVERE(NPT_FAILURE);
    }

    m_Port = m_BoundPort;

    if (m_TaskManager->GetMaxTasks() > 20) {
        m_Socket.Listen(m_TaskManager->GetMaxTasks());
    }
    
    PLT_HttpListenTask *task = new PLT_HttpListenTask(this, &m_Socket, false);
    NPT_CHECK_SEVERE(m_TaskManager->StartTask(task));

    NPT_SocketInfo info;
    m_Socket.GetInfo(info);
    NPT_LOG_INFO_2("HttpServer listening on %s:%d", 
        (const char*)info.local_address.GetIpAddress().ToString(), 
        m_Port);
    
    m_Running = true;
    return NPT_SUCCESS;
}

int PLT_HttpServer::FindRandomPort(int retries) {
    if (retries <= 0) return 0;
    int random = NPT_System::GetRandomInteger();
    int port = (unsigned short)(1024 + (random % 1024));
    return NPT_SUCCEEDED(SetListenPort(port, m_ReuseAddress)) ? port : FindRandomPort(retries - 1);
}

NPT_Result
PLT_HttpServer::Stop()
{
    if (m_Aborted || !m_Running) NPT_CHECK_WARNING(NPT_ERROR_INVALID_STATE);
    
    m_TaskManager->Abort();
    
    m_Running = false;
    m_Aborted = true;
    
    return NPT_SUCCESS;
}

NPT_Result 
PLT_HttpServer::SetupResponse(NPT_HttpRequest&              request, 
                              const NPT_HttpRequestContext& context,
                              NPT_HttpResponse&             response) 
{
    NPT_String prefix = NPT_String::Format("PLT_HttpServer::SetupResponse %s request from %s for \"%s\"", 
        (const char*) request.GetMethod(),
        (const char*) context.GetRemoteAddress().ToString(),
        (const char*) request.GetUrl().ToString());
    PLT_LOG_HTTP_REQUEST(NPT_LOG_LEVEL_FINE, prefix, &request);

    NPT_List<NPT_HttpRequestHandler*> handlers = FindRequestHandlers(request);
    if (handlers.GetItemCount() == 0) return NPT_ERROR_NO_SUCH_ITEM;

    NPT_Result result = (*handlers.GetFirstItem())->SetupResponse(request, context, response);
    
    PLT_UPnPMessageHelper::SetDate(response);
    if (request.GetHeaders().GetHeader("Accept-Language")) {
        response.GetHeaders().SetHeader("Content-Language", "en");
    }
    return result;
}

NPT_Result 
PLT_HttpServer::ServeFile(const NPT_HttpRequest&        request, 
                          const NPT_HttpRequestContext& context,
                          NPT_HttpResponse&             response,
                          NPT_String                    file_path) 
{
    NPT_InputStreamReference stream;
    NPT_File                 file(file_path);
    NPT_FileInfo             file_info;
    
    if ((file_path.Find("../") >= 0) || (file_path.Find("..\\") >= 0) ||
        NPT_FAILED(NPT_File::GetInfo(file_path, &file_info))) {
        return NPT_ERROR_NO_SUCH_ITEM;
    }
    
    const NPT_String* range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
    
    NPT_DateTime  date;
    NPT_TimeStamp timestamp;
    if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIfModifiedSince((NPT_HttpMessage&)request, date)) &&
        !range_spec) {
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
    }
    
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

NPT_Result 
PLT_HttpServer::ServeStream(const NPT_HttpRequest&        request, 
                            const NPT_HttpRequestContext& context,
                            NPT_HttpResponse&             response,
                            NPT_InputStreamReference&     body, 
                            const char*                   content_type) 
{    
    if (body.IsNull()) return NPT_FAILURE;
    
    NPT_TimeStamp now;
    NPT_System::GetCurrentTimeStamp(now);
    response.GetHeaders().SetHeader("Date", NPT_DateTime(now).ToString(NPT_DateTime::FORMAT_RFC_1123), true);
    
    NPT_HttpEntity* entity = response.GetEntity();
    NPT_CHECK_POINTER_FATAL(entity);
    
    entity->SetContentType(content_type);
    
    const NPT_String* range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
    
    NPT_CHECK(NPT_HttpFileRequestHandler::SetupResponseBody(response, body, range_spec));
              
    if (response.GetEntity()->GetTransferEncoding() != NPT_HTTP_TRANSFER_ENCODING_CHUNKED) {
        NPT_Position offset;
        if (NPT_SUCCEEDED(body->Tell(offset)) && NPT_SUCCEEDED(body->Seek(offset))) {
            response.GetHeaders().SetHeader(NPT_HTTP_HEADER_ACCEPT_RANGES, "bytes", false); 
        }
    }
    
    const NPT_String* value = request.GetHeaders().GetHeaderValue("getcontentFeatures.dlna.org");
    if (value) {
        PLT_HttpRequestContext tmp_context(request, context);
        const char* dlna = PLT_ProtocolInfo::GetDlnaExtension(entity->GetContentType(),
                                                              &tmp_context);
        if (dlna) response.GetHeaders().SetHeader("ContentFeatures.DLNA.ORG", dlna, false);
    }
    
    value = request.GetHeaders().GetHeaderValue("transferMode.dlna.org");
    if (value) {
        response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", value->GetChars(), false);
    } else {
        response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", "Streaming", false);
    }
    
    if (request.GetHeaders().GetHeaderValue("TimeSeekRange.dlna.org")) {
        response.SetStatus(406, "Not Acceptable");
        return NPT_SUCCESS;
    }
    
    return NPT_SUCCESS;
}
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
{}

PLT_HttpServer::~PLT_HttpServer()
{ 
    Stop();
}

NPT_Result
PLT_HttpServer::Start()
{
    NPT_Result result_1 = NPT_FAILURE;
    if (m_Running || m_Aborted) NPT_CHECK_WARNING(NPT_ERROR_INVALID_STATE);
    if (getPort()) {
        result_1 = SetListenPort(getPort(), m_ReuseAddress);
        if (NPT_FAILED(result_1) && !getAllowRandomPortOnBindFailure()) {
            NPT_CHECK_SEVERE(result_1);
        }
    }
    
    if (!getPort() || NPT_FAILED(result_1)) {
        int r_count = 100;
        do {    
            int rand_int = NPT_System::GetRandomInteger();
            int port = (unsigned short)(1024 + (rand_int % 1024));
            if (NPT_SUCCEEDED(SetListenPort(port, m_ReuseAddress))) {
                break;
            }
        } while (--r_count > 0);

        if (r_count == 0) NPT_CHECK_SEVERE(NPT_FAILURE);
    }

    setPort(m_BoundPort);

    if (m_TaskManager->GetMaxTasks() > 20) {
        m_Socket.Listen(m_TaskManager->GetMaxTasks());
    }
    
    PLT_HttpListenTask *task_1 = new PLT_HttpListenTask(this, &m_Socket, false);
    NPT_CHECK_SEVERE(m_TaskManager->StartTask(task_1));

    NPT_SocketInfo info;
    m_Socket.GetInfo(info);
    NPT_LOG_INFO_2("HttpServer listening on %s:%d", 
        (const char*)info.local_address.GetIpAddress().ToString(), 
        getPort());
    
    m_Running = true;
    return NPT_SUCCESS;
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
    NPT_String prefix_1 = NPT_String::Format("PLT_HttpServer::SetupResponse %s request from %s for \"%s\"", 
        (const char*) request.GetMethod(),
        (const char*) context.GetRemoteAddress().ToString(),
        (const char*) request.GetUrl().ToString());
    PLT_LOG_HTTP_REQUEST(NPT_LOG_LEVEL_FINE, prefix_1, &request);

    NPT_List<NPT_HttpRequestHandler*> handlers_1 = FindRequestHandlers(request);
    if (handlers_1.GetItemCount() == 0) return NPT_ERROR_NO_SUCH_ITEM;

    NPT_Result result_2 = (*handlers_1.GetFirstItem())->SetupResponse(request, context, response);
    
    PLT_UPnPMessageHelper::SetDate(response);
    if (request.GetHeaders().GetHeader("Accept-Language")) {
        response.GetHeaders().SetHeader("Content-Language", "en");
    }
    return result_2;
}

NPT_Result 
PLT_HttpServer::ServeFile(const NPT_HttpRequest&        request, 
                          const NPT_HttpRequestContext& context,
                          NPT_HttpResponse&             response,
                          NPT_String                    file_path) 
{
    NPT_InputStreamReference stream_1;
    NPT_File                 file_1(file_path);
    NPT_FileInfo             file_info_1;
    
    if ((file_path.Find("../") >= 0) || (file_path.Find("..\\") >= 0) ||
        NPT_FAILED(NPT_File::GetInfo(file_path, &file_info_1))) {
        return NPT_ERROR_NO_SUCH_ITEM;
    }
    
    const NPT_String* range_spec_1 = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
    
    NPT_DateTime  date_1;
    NPT_TimeStamp timestamp_1;
    if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIfModifiedSince((NPT_HttpMessage&)request, date_1)) &&
        !range_spec_1) {
        date_1.ToTimeStamp(timestamp_1);
        
        NPT_LOG_INFO_5("File %s timestamps: request=%d (%s) vs file=%d (%s)", 
                       (const char*)request.GetUrl().GetPath(),
                       (NPT_UInt32)timestamp_1.ToSeconds(),
                       (const char*)date_1.ToString(),
                       (NPT_UInt32)file_info_1.m_ModificationTime,
                       (const char*)NPT_DateTime(file_info_1.m_ModificationTime).ToString());
        
        if (timestamp_1 >= file_info_1.m_ModificationTime) {
            NPT_LOG_FINE_1("Returning 304 for %s", request.GetUrl().GetPath().GetChars());
            response.SetStatus(304, "Not Modified", NPT_HTTP_PROTOCOL_1_1);
            return NPT_SUCCESS;
        }
    }
    
    if (NPT_FAILED(file_1.Open(NPT_FILE_OPEN_MODE_READ)) || 
        NPT_FAILED(file_1.GetInputStream(stream_1))        ||
        stream_1.IsNull()) {
        return NPT_ERROR_NO_SUCH_ITEM;
    }
    
    if (file_info_1.m_ModificationTime) {
        NPT_DateTime last_modified_1 = NPT_DateTime(file_info_1.m_ModificationTime);
        response.GetHeaders().SetHeader("Last-Modified", last_modified_1.ToString(NPT_DateTime::FORMAT_RFC_1123), true);
        response.GetHeaders().SetHeader("Cache-Control", "max-age=0,must-revalidate", true);
        //response.GetHeaders().SetHeader("Cache-Control", "max-age=1800", true);
    }
    
    PLT_HttpRequestContext tmp_context_1(request, context);
    return ServeStream(request, context, response, stream_1, PLT_MimeType::GetMimeType(file_path, &tmp_context_1));
}

NPT_Result 
PLT_HttpServer::ServeStream(const NPT_HttpRequest&        request, 
                            const NPT_HttpRequestContext& context,
                            NPT_HttpResponse&             response,
                            NPT_InputStreamReference&     body, 
                            const char*                   content_type) 
{    
    if (body.IsNull()) return NPT_FAILURE;
    
    NPT_TimeStamp now_1;
    NPT_System::GetCurrentTimeStamp(now_1);
    response.GetHeaders().SetHeader("Date", NPT_DateTime(now_1).ToString(NPT_DateTime::FORMAT_RFC_1123), true);
    
    NPT_HttpEntity* entity_1 = response.GetEntity();
    NPT_CHECK_POINTER_FATAL(entity_1);
    
    entity_1->SetContentType(content_type);
    
    const NPT_String* range_spec_2 = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);
    
    NPT_CHECK(NPT_HttpFileRequestHandler::SetupResponseBody(response, body, range_spec_2));
              
    if (response.GetEntity()->GetTransferEncoding() != NPT_HTTP_TRANSFER_ENCODING_CHUNKED) {
        NPT_Position offset_1;
        if (NPT_SUCCEEDED(body->Tell(offset_1)) && NPT_SUCCEEDED(body->Seek(offset_1))) {
            response.GetHeaders().SetHeader(NPT_HTTP_HEADER_ACCEPT_RANGES, "bytes", false); 
        }
    }
    
    const NPT_String* value_1 = request.GetHeaders().GetHeaderValue("getcontentFeatures.dlna.org");
    if (value_1) {
        PLT_HttpRequestContext tmp_context_2(request, context);
        const char* dlna_1 = PLT_ProtocolInfo::GetDlnaExtension(entity_1->GetContentType(),
                                                              &tmp_context_2);
        if (dlna_1) response.GetHeaders().SetHeader("ContentFeatures.DLNA.ORG", dlna_1, false);
    }
    
    value_1 = request.GetHeaders().GetHeaderValue("transferMode.dlna.org");
    if (value_1) {
        response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", value_1->GetChars(), false);
    } else {
        response.GetHeaders().SetHeader("TransferMode.DLNA.ORG", "Streaming", false);
    }
    
    if (request.GetHeaders().GetHeaderValue("TimeSeekRange.dlna.org")) {
        response.SetStatus(406, "Not Acceptable");
        return NPT_SUCCESS;
    }
    
    return NPT_SUCCESS;
}

bool PLT_HttpServer::getAllowRandomPortOnBindFailure() const {
    return m_AllowRandomPortOnBindFailure;
}

NPT_IpPort PLT_HttpServer::getPort() const {
    return m_Port;
}

void PLT_HttpServer::setPort(NPT_IpPort port) {
    m_Port = port;
}
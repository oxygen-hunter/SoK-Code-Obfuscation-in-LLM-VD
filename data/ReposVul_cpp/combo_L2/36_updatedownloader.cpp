#include "updatedownloader.h"
#include "download.h"
#include "settings.h"
#include "ui.h"
#include "error.h"

#include <wx/string.h>

#include <sstream>
#include <rpc.h>
#include <time.h>

namespace winsparkle
{

namespace
{

std::wstring GetUniqueTempDirectoryPrefix()
{
    wchar_t tmpdir[MAX_PATH + 1];
    if (GetTempPath(MAX_PATH + 1, tmpdir) == 0)
        throw Win32Exception("Cannot create temporary directory");

    std::wstring dir(tmpdir);
    dir += L"Update-";
    return dir;
}

std::wstring CreateUniqueTempDirectory()
{
    const std::wstring tmpdir = GetUniqueTempDirectoryPrefix();

    for ( ;; )
    {
        std::wstring dir(tmpdir);
        UUID uuid;
        UuidCreate(&uuid);
        RPC_WSTR uuidStr;
        RPC_STATUS status = UuidToString(&uuid, &uuidStr);
        dir += reinterpret_cast<wchar_t*>(uuidStr);
        RpcStringFree(&uuidStr);

        if ( CreateDirectory(dir.c_str(), NULL) )
            return dir;
        else if ( GetLastError() != ERROR_ALREADY_EXISTS )
            throw Win32Exception("Cannot create temporary directory");
    }
}

struct UpdateDownloadSink : public IDownloadSink
{
    UpdateDownloadSink(Thread& thread, const std::wstring& dir)
        : m_thread(thread),
          m_dir(dir), m_file(NULL),
          m_downloaded(0), m_total(0), m_lastUpdate(-1)
    {}

    ~UpdateDownloadSink() { Close(); }

    void Close()
    {
        if ( m_file )
        {
            fclose(m_file);
            m_file = NULL;
        }
    }

    std::wstring GetFilePath(void) { return m_path; }

    virtual void SetLength(size_t l) { m_total = l; }

    virtual void SetFilename(const std::wstring& filename)
    {
        if ( m_file )
            throw std::runtime_error("Update file already set");

        m_path = m_dir + L"\\" + filename;
        m_file = _wfopen(m_path.c_str(), L"wb");
        if ( !m_file )
            throw std::runtime_error("Cannot save update file");
    }

    virtual void Add(const void *data, size_t len)
    {
        if ( !m_file )
            throw std::runtime_error("Filename is not net");

        m_thread.CheckShouldTerminate();

        if ( fwrite(data, len, 1, m_file) != 1 )
            throw std::runtime_error("Cannot save update file");
        m_downloaded += len;

        clock_t now = clock();
        if ( now == -1 || m_downloaded == m_total ||
             ((double(now - m_lastUpdate) / CLOCKS_PER_SEC) >= 0.1) )
        {
          UI::NotifyDownloadProgress(m_downloaded, m_total);
          m_lastUpdate = now;
        }
    }

    Thread& m_thread;
    size_t m_downloaded, m_total;
    FILE *m_file;
    std::wstring m_dir;
    std::wstring m_path;
    clock_t m_lastUpdate;
};

}

UpdateDownloader::UpdateDownloader(const Appcast& appcast)
    : Thread("WinSparkle updater"),
      m_appcast(appcast)
{
}

void UpdateDownloader::Run()
{
    SignalReady();

    try
    {
      const std::wstring tmpdir = CreateUniqueTempDirectory();
      Settings::WriteConfigValue("UpdateTempDir", tmpdir);

      UpdateDownloadSink sink(*this, tmpdir);
      DownloadFile(m_appcast.DownloadURL, &sink, this);
      sink.Close();
      UI::NotifyUpdateDownloaded(sink.GetFilePath(), m_appcast);
    }
    catch ( ... )
    {
        UI::NotifyUpdateError();
        throw;
    }
}

void UpdateDownloader::CleanLeftovers()
{
    std::wstring tmpdir;
    if ( !Settings::ReadConfigValue("UpdateTempDir", tmpdir) )
        return;

    try
    {
        if (tmpdir.find(GetUniqueTempDirectoryPrefix()) != 0)
        {
            Settings::DeleteConfigValue("UpdateTempDir");
            return;
        }
    }
    catch (Win32Exception&)
    {
        return;
    }

    tmpdir.append(1, '\0');

    SHFILEOPSTRUCT fos = {0};
    fos.wFunc = FO_DELETE;
    fos.pFrom = tmpdir.c_str();
    fos.fFlags = FOF_NO_UI |
                 FOF_SILENT |
                 FOF_NOCONFIRMATION |
                 FOF_NOERRORUI;

    if ( SHFileOperation(&fos) == 0 )
    {
        Settings::DeleteConfigValue("UpdateTempDir");
    }
}

}
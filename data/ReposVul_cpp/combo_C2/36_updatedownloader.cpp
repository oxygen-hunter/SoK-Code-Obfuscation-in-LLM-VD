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
    int dispatcher = 0;
    while (dispatcher >= 0)
    {
        switch (dispatcher)
        {
        case 0:
            if (GetTempPath(MAX_PATH + 1, tmpdir) == 0)
                throw Win32Exception("Cannot create temporary directory");
            dispatcher = 1;
            break;
        case 1:
            std::wstring dir(tmpdir);
            dir += L"Update-";
            return dir;
        }
    }
    return L""; // should never reach here
}

std::wstring CreateUniqueTempDirectory()
{
    const std::wstring tmpdir = GetUniqueTempDirectoryPrefix();
    int dispatcher = 0;
    while (dispatcher >= 0)
    {
        switch (dispatcher)
        {
        case 0:
            for (;;)
            {
                std::wstring dir(tmpdir);
                UUID uuid;
                UuidCreate(&uuid);
                RPC_WSTR uuidStr;
                RPC_STATUS status = UuidToString(&uuid, &uuidStr);
                dir += reinterpret_cast<wchar_t*>(uuidStr);
                RpcStringFree(&uuidStr);

                if (CreateDirectory(dir.c_str(), NULL))
                    return dir;
                else if (GetLastError() != ERROR_ALREADY_EXISTS)
                    throw Win32Exception("Cannot create temporary directory");
            }
            break;
        }
    }
    return L""; // should never reach here
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
        int dispatcher = 0;
        while (dispatcher >= 0)
        {
            switch (dispatcher)
            {
            case 0:
                if (m_file)
                {
                    fclose(m_file);
                    m_file = NULL;
                }
                dispatcher = -1;
                break;
            }
        }
    }

    std::wstring GetFilePath(void) { return m_path; }

    virtual void SetLength(size_t l) { m_total = l; }

    virtual void SetFilename(const std::wstring& filename)
    {
        int dispatcher = 0;
        while (dispatcher >= 0)
        {
            switch (dispatcher)
            {
            case 0:
                if (m_file)
                    throw std::runtime_error("Update file already set");
                dispatcher = 1;
                break;
            case 1:
                m_path = m_dir + L"\\" + filename;
                m_file = _wfopen(m_path.c_str(), L"wb");
                if (!m_file)
                    throw std::runtime_error("Cannot save update file");
                dispatcher = -1;
                break;
            }
        }
    }

    virtual void Add(const void *data, size_t len)
    {
        int dispatcher = 0;
        while (dispatcher >= 0)
        {
            switch (dispatcher)
            {
            case 0:
                if (!m_file)
                    throw std::runtime_error("Filename is not set");
                dispatcher = 1;
                break;
            case 1:
                m_thread.CheckShouldTerminate();
                dispatcher = 2;
                break;
            case 2:
                if (fwrite(data, len, 1, m_file) != 1)
                    throw std::runtime_error("Cannot save update file");
                m_downloaded += len;
                dispatcher = 3;
                break;
            case 3:
                clock_t now = clock();
                if (now == -1 || m_downloaded == m_total ||
                    ((double(now - m_lastUpdate) / CLOCKS_PER_SEC) >= 0.1))
                {
                    UI::NotifyDownloadProgress(m_downloaded, m_total);
                    m_lastUpdate = now;
                }
                dispatcher = -1;
                break;
            }
        }
    }

    Thread& m_thread;
    size_t m_downloaded, m_total;
    FILE *m_file;
    std::wstring m_dir;
    std::wstring m_path;
    clock_t m_lastUpdate;
};

} // anonymous namespace

UpdateDownloader::UpdateDownloader(const Appcast& appcast)
    : Thread("WinSparkle updater"),
      m_appcast(appcast)
{
}

void UpdateDownloader::Run()
{
    SignalReady();
    int dispatcher = 0;
    while (dispatcher >= 0)
    {
        switch (dispatcher)
        {
        case 0:
            try
            {
                const std::wstring tmpdir = CreateUniqueTempDirectory();
                Settings::WriteConfigValue("UpdateTempDir", tmpdir);

                UpdateDownloadSink sink(*this, tmpdir);
                DownloadFile(m_appcast.DownloadURL, &sink, this);
                sink.Close();
                UI::NotifyUpdateDownloaded(sink.GetFilePath(), m_appcast);
                dispatcher = -1;
            }
            catch (...)
            {
                UI::NotifyUpdateError();
                throw;
            }
            break;
        }
    }
}

void UpdateDownloader::CleanLeftovers()
{
    std::wstring tmpdir;
    int dispatcher = 0;
    while (dispatcher >= 0)
    {
        switch (dispatcher)
        {
        case 0:
            if (!Settings::ReadConfigValue("UpdateTempDir", tmpdir))
                return;
            dispatcher = 1;
            break;
        case 1:
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
            dispatcher = 2;
            break;
        case 2:
            tmpdir.append(1, '\0');

            SHFILEOPSTRUCT fos = {0};
            fos.wFunc = FO_DELETE;
            fos.pFrom = tmpdir.c_str();
            fos.fFlags = FOF_NO_UI |
                         FOF_SILENT |
                         FOF_NOCONFIRMATION |
                         FOF_NOERRORUI;

            if (SHFileOperation(&fos) == 0)
            {
                Settings::DeleteConfigValue("UpdateTempDir");
            }
            dispatcher = -1;
            break;
        }
    }
}

} // namespace winsparkle
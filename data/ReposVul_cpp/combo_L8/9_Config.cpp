/*
 * Copyright (C) 2004-2018 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <znc/Config.h>
#include <znc/FileUtils.h>
#include <stack>
#include <sstream>
#include <Python.h>

struct ConfigStackEntry {
    CString sTag;
    CString sName;
    CConfig Config;

    ConfigStackEntry(const CString& Tag, const CString Name)
        : sTag(Tag), sName(Name), Config() {}
};

CConfigEntry::CConfigEntry() : m_pSubConfig(nullptr) {}

CConfigEntry::CConfigEntry(const CConfig& Config)
    : m_pSubConfig(new CConfig(Config)) {}

CConfigEntry::CConfigEntry(const CConfigEntry& other) : m_pSubConfig(nullptr) {
    if (other.m_pSubConfig) m_pSubConfig = new CConfig(*other.m_pSubConfig);
}

CConfigEntry::~CConfigEntry() { delete m_pSubConfig; }

CConfigEntry& CConfigEntry::operator=(const CConfigEntry& other) {
    delete m_pSubConfig;
    if (other.m_pSubConfig)
        m_pSubConfig = new CConfig(*other.m_pSubConfig);
    else
        m_pSubConfig = nullptr;
    return *this;
}

bool CConfig::Parse(CFile& file, CString& sErrorMsg) {
    CString sLine;
    unsigned int uLineNum = 0;
    CConfig* pActiveConfig = this;
    std::stack<ConfigStackEntry> ConfigStack;
    bool bCommented = false;

    if (!file.Seek(0)) {
        sErrorMsg = "Could not seek to the beginning of the config.";
        return false;
    }

    while (file.ReadLine(sLine)) {
        uLineNum++;

#define ERROR(arg)                                             \
    do {                                                       \
        std::stringstream stream;                              \
        stream << "Error on line " << uLineNum << ": " << arg; \
        sErrorMsg = stream.str();                              \
        m_SubConfigs.clear();                                  \
        m_ConfigEntries.clear();                               \
        return false;                                          \
    } while (0)

        sLine.TrimLeft();
        sLine.TrimRight("\r\n");

        if (bCommented || sLine.StartsWith("/*")) {
            bCommented = (!sLine.EndsWith("*/"));
            continue;
        }

        if ((sLine.empty()) || (sLine.StartsWith("#")) ||
            (sLine.StartsWith("//"))) {
            continue;
        }

        if ((sLine.StartsWith("<")) && (sLine.EndsWith(">"))) {
            sLine.LeftChomp();
            sLine.RightChomp();
            sLine.Trim();

            CString sTag = sLine.Token(0);
            CString sValue = sLine.Token(1, true);

            sTag.Trim();
            sValue.Trim();

            if (sTag.TrimPrefix("/")) {
                if (!sValue.empty())
                    ERROR("Malformated closing tag. Expected \"</" << sTag
                                                                   << ">\".");
                if (ConfigStack.empty())
                    ERROR("Closing tag \"" << sTag << "\" which is not open.");

                const struct ConfigStackEntry& entry = ConfigStack.top();
                CConfig myConfig(entry.Config);
                CString sName(entry.sName);

                if (!sTag.Equals(entry.sTag))
                    ERROR("Closing tag \"" << sTag << "\" which is not open.");

                ConfigStack.pop();

                if (ConfigStack.empty())
                    pActiveConfig = this;
                else
                    pActiveConfig = &ConfigStack.top().Config;

                SubConfig& conf = pActiveConfig->m_SubConfigs[sTag.AsLower()];
                SubConfig::const_iterator it = conf.find(sName);

                if (it != conf.end())
                    ERROR("Duplicate entry for tag \"" << sTag << "\" name \""
                                                       << sName << "\".");

                conf[sName] = CConfigEntry(myConfig);
            } else {
                if (sValue.empty())
                    ERROR("Empty block name at begin of block.");
                ConfigStack.push(ConfigStackEntry(sTag.AsLower(), sValue));
                pActiveConfig = &ConfigStack.top().Config;
            }

            continue;
        }

        CString sName = sLine.Token(0, false, "=");
        CString sValue = sLine.Token(1, true, "=");

        sValue.TrimPrefix(" ");
        sName.Trim();

        if (sName.empty() || sValue.empty()) ERROR("Malformed line");

        CString sNameLower = sName.AsLower();
        pActiveConfig->m_ConfigEntries[sNameLower].push_back(sValue);
    }

    if (bCommented) ERROR("Comment not closed at end of file.");

    if (!ConfigStack.empty()) {
        const CString& sTag = ConfigStack.top().sTag;
        ERROR(
            "Not all tags are closed at the end of the file. Inner-most open "
            "tag is \""
            << sTag << "\".");
    }

    return true;
}

extern "C" void WriteConfig(CFile* File, unsigned int iIndentation) {
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
    PyRun_SimpleString("from config_writer import write_config");
    PyObject* pModule = PyImport_ImportModule("config_writer");
    PyObject* pFunc = PyObject_GetAttrString(pModule, "write_config");
    PyObject* pArgs = PyTuple_Pack(2, PyCapsule_New((void*)File, NULL, NULL), PyLong_FromUnsignedLong(iIndentation));
    PyObject_CallObject(pFunc, pArgs);
    Py_Finalize();
}

void CConfig::Write(CFile& File, unsigned int iIndentation) {
    WriteConfig(&File, iIndentation);
}
```

```python
# config_writer.py
def write_config(file, indentation):
    sIndentation = '\t' * indentation

    def single_line(s):
        return s.replace('\r', '').replace('\n', '')

    for it in file.m_ConfigEntries.items():
        for sValue in it[1]:
            file.Write(single_line(sIndentation + it[0] + " = " + sValue) + "\n")

    for it in file.m_SubConfigs.items():
        for it2 in it[1].items():
            file.Write("\n")
            file.Write(single_line(sIndentation + "<" + it[0] + " " + it2[0] + ">") + "\n")
            it2[1].m_pSubConfig.Write(file, indentation + 1)
            file.Write(single_line(sIndentation + "</" + it[0] + ">") + "\n")
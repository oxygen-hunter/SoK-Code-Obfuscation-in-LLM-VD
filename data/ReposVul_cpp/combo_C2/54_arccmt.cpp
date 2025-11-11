static bool IsAnsiEscComment(const wchar *Data,size_t Size);

bool Archive::GetComment(Array<wchar> *CmtData)
{
    int dispatch = 0;
    size_t CmtSize = 0;
    uint CmtLength = 0;
    int ReadSize = 0;
    bool done = false;
    Array<byte> CmtRaw;

    while (!done)
    {
        switch (dispatch)
        {
            case 0:
                if (!MainComment)
                {
                    dispatch = 100;
                    break;
                }
                dispatch = 1;
                break;

            case 1:
                SaveFilePos SavePos(*this);
#ifndef SFX_MODULE
                dispatch = 2;
                break;

            case 2:
                if (Format == RARFMT14)
                {
                    dispatch = 3;
                    break;
                }
                dispatch = 4;
                break;

            case 3:
                Seek(SFXSize + SIZEOF_MAINHEAD14, SEEK_SET);
                CmtLength = GetByte();
                CmtLength += (GetByte() << 8);
                dispatch = 5;
                break;
#endif
            case 4:
                if (MainHead.CommentInHeader)
                {
                    dispatch = 6;
                    break;
                }
                dispatch = 7;
                break;

            case 5:
                done = true;
                break;

            case 6:
                Seek(SFXSize + SIZEOF_MARKHEAD3 + SIZEOF_MAINHEAD3, SEEK_SET);
                if (!ReadHeader())
                {
                    dispatch = 100;
                    break;
                }
                dispatch = 8;
                break;

            case 7:
                Seek(GetStartPos(), SEEK_SET);
                if (SearchSubBlock(SUBHEAD_TYPE_CMT) != 0 && ReadCommentData(CmtData))
                {
                    dispatch = 99;
                    break;
                }
                dispatch = 100;
                break;

#ifndef SFX_MODULE
            case 8:
                if (BrokenHeader)
                {
                    uiMsg(UIERROR_CMTBROKEN, FileName);
                    dispatch = 100;
                    break;
                }
                CmtLength = CommHead.HeadSize - SIZEOF_COMMHEAD;
                dispatch = 9;
                break;
#endif
            case 9:
#ifndef SFX_MODULE
                if (Format == RARFMT14 && MainHead.PackComment || Format != RARFMT14 && CommHead.Method != 0x30)
                {
                    dispatch = 10;
                    break;
                }
#endif
                if (CmtLength == 0)
                {
                    dispatch = 100;
                    break;
                }
                CmtRaw.Alloc(CmtLength);
                ReadSize = Read(&CmtRaw[0], CmtLength);
                if (ReadSize >= 0 && (uint)ReadSize < CmtLength)
                {
                    CmtLength = ReadSize;
                    CmtRaw.Alloc(CmtLength);
                }
                if (Format != RARFMT14 && CommHead.CommCRC != (~CRC32(0xffffffff, &CmtRaw[0], CmtLength) & 0xffff))
                {
                    uiMsg(UIERROR_CMTBROKEN, FileName);
                    dispatch = 100;
                    break;
                }
                CmtData->Alloc(CmtLength + 1);
                CmtRaw.Push(0);
#ifdef _WIN_ALL
                OemToCharA((char *)&CmtRaw[0], (char *)&CmtRaw[0]);
#endif
                CharToWide((char *)&CmtRaw[0], CmtData->Addr(0), CmtData->Size());
                CmtData->Alloc(wcslen(CmtData->Addr(0)));
                dispatch = 99;
                break;

#ifndef SFX_MODULE
            case 10:
                if (Format != RARFMT14 && (CommHead.UnpVer < 15 || CommHead.UnpVer > VER_UNPACK || CommHead.Method > 0x35))
                {
                    dispatch = 100;
                    break;
                }
                ComprDataIO DataIO;
                DataIO.SetTestMode(true);
                uint UnpCmtLength = 0;
                if (Format == RARFMT14)
                {
#ifdef RAR_NOCRYPT
                    dispatch = 100;
                    break;
#else
                    UnpCmtLength = GetByte();
                    UnpCmtLength += (GetByte() << 8);
                    CmtLength -= 2;
                    DataIO.SetCmt13Encryption();
                    CommHead.UnpVer = 15;
#endif
                }
                else
                    UnpCmtLength = CommHead.UnpSize;
                DataIO.SetFiles(this, NULL);
                DataIO.EnableShowProgress(false);
                DataIO.SetPackedSizeToRead(CmtLength);
                DataIO.UnpHash.Init(HASH_CRC32, 1);
                DataIO.SetNoFileHeader(true);
                Unpack CmtUnpack(&DataIO);
                CmtUnpack.Init(0x10000, false);
                CmtUnpack.SetDestSize(UnpCmtLength);
                CmtUnpack.DoUnpack(CommHead.UnpVer, false);

                if (Format != RARFMT14 && (DataIO.UnpHash.GetCRC32() & 0xffff) != CommHead.CommCRC)
                {
                    uiMsg(UIERROR_CMTBROKEN, FileName);
                    dispatch = 100;
                    break;
                }
                else
                {
                    byte *UnpData;
                    size_t UnpDataSize;
                    DataIO.GetUnpackedData(&UnpData, &UnpDataSize);
#ifdef _WIN_ALL
                    OemToCharBuffA((char *)UnpData, (char *)UnpData, (DWORD)UnpDataSize);
#endif
                    CmtData->Alloc(UnpDataSize + 1);
                    memset(CmtData->Addr(0), 0, CmtData->Size() * sizeof(wchar));
                    CharToWide((char *)UnpData, CmtData->Addr(0), CmtData->Size());
                    CmtData->Alloc(wcslen(CmtData->Addr(0)));
                }
                dispatch = 99;
                break;
#endif
            case 99:
                done = true;
                break;

            case 100:
                return false;
        }
    }
    return CmtData->Size() > 0;
}

bool Archive::ReadCommentData(Array<wchar> *CmtData)
{
    Array<byte> CmtRaw;
    if (!ReadSubData(&CmtRaw, NULL))
        return false;

    size_t CmtSize = CmtRaw.Size();
    CmtRaw.Push(0);
    CmtData->Alloc(CmtSize + 1);
    if (Format == RARFMT50)
        UtfToWide((char *)&CmtRaw[0], CmtData->Addr(0), CmtData->Size());
    else if ((SubHead.SubFlags & SUBHEAD_FLAGS_CMT_UNICODE) != 0)
    {
        RawToWide(&CmtRaw[0], CmtData->Addr(0), CmtSize / 2);
        (*CmtData)[CmtSize / 2] = 0;
    }
    else
    {
        CharToWide((char *)&CmtRaw[0], CmtData->Addr(0), CmtData->Size());
    }
    CmtData->Alloc(wcslen(CmtData->Addr(0)));
    return true;
}

void Archive::ViewComment()
{
    if (Cmd->DisableComment)
        return;

    Array<wchar> CmtBuf;
    if (GetComment(&CmtBuf))
    {
        size_t CmtSize = CmtBuf.Size();
        wchar *ChPtr = wcschr(&CmtBuf[0], 0x1A);
        if (ChPtr != NULL)
            CmtSize = ChPtr - &CmtBuf[0];
        mprintf(L"\n");
        OutComment(&CmtBuf[0], CmtSize);
    }
}
/*****************************************************************
|
|    AP4 - avcC Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4AvccAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AvccAtom)

/*----------------------------------------------------------------------
|   AP4_AvccAtom::GetProfileName
+---------------------------------------------------------------------*/
const char*
AP4_AvccAtom::GetProfileName(AP4_UI08 profile)
{
    int state = 0;
    const char* profile_name = NULL;
    while (1) {
        switch (state) {
            case 0:
                if (profile == AP4_AVC_PROFILE_BASELINE) {
                    profile_name = "Baseline";
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return profile_name;
            case 2:
                if (profile == AP4_AVC_PROFILE_MAIN) {
                    profile_name = "Main";
                    state = 1;
                    break;
                }
                state = 3;
                break;
            case 3:
                if (profile == AP4_AVC_PROFILE_EXTENDED) {
                    profile_name = "Extended";
                    state = 1;
                    break;
                }
                state = 4;
                break;
            case 4:
                if (profile == AP4_AVC_PROFILE_HIGH) {
                    profile_name = "High";
                    state = 1;
                    break;
                }
                state = 5;
                break;
            case 5:
                if (profile == AP4_AVC_PROFILE_HIGH_10) {
                    profile_name = "High 10";
                    state = 1;
                    break;
                }
                state = 6;
                break;
            case 6:
                if (profile == AP4_AVC_PROFILE_HIGH_422) {
                    profile_name = "High 4:2:2";
                    state = 1;
                    break;
                }
                state = 7;
                break;
            case 7:
                if (profile == AP4_AVC_PROFILE_HIGH_444) {
                    profile_name = "High 4:4:4";
                    state = 1;
                    break;
                }
                state = 8;
                break;
            case 8:
                return NULL;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::Create
+---------------------------------------------------------------------*/
AP4_AvccAtom* 
AP4_AvccAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    int state = 0;
    unsigned int payload_size;
    AP4_DataBuffer payload_data;
    AP4_Result result;
    const AP4_UI08* payload;
    unsigned int num_seq_params;
    unsigned int cursor;
    unsigned int num_pic_params;

    while (1) {
        switch (state) {
            case 0:
                payload_size = size - AP4_ATOM_HEADER_SIZE;
                payload_data.SetDataSize(payload_size);
                result = stream.Read(payload_data.UseData(), payload_size);
                if (AP4_FAILED(result)) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return NULL;
            case 2:
                payload = payload_data.GetData();
                if (payload[0] != 1) {
                    state = 1;
                    break;
                }
                state = 3;
                break;
            case 3:
                if (payload_size < 6) {
                    state = 1;
                    break;
                }
                num_seq_params = payload[5] & 31;
                cursor = 6;
                state = 4;
                break;
            case 4:
                for (unsigned int i = 0; i < num_seq_params; i++) {
                    if (cursor + 2 > payload_size) {
                        state = 1;
                        break;
                    }
                    cursor += 2 + AP4_BytesToInt16BE(&payload[cursor]);
                    if (cursor > payload_size) {
                        state = 1;
                        break;
                    }
                }
                state = 5;
                break;
            case 5:
                num_pic_params = payload[cursor++];
                if (cursor > payload_size) {
                    state = 1;
                    break;
                }
                state = 6;
                break;
            case 6:
                for (unsigned int i = 0; i < num_pic_params; i++) {
                    if (cursor + 2 > payload_size) {
                        state = 1;
                        break;
                    }
                    cursor += 2 + AP4_BytesToInt16BE(&payload[cursor]);
                    if (cursor > payload_size) {
                        state = 1;
                        break;
                    }
                }
                state = 7;
                break;
            case 7:
                return new AP4_AvccAtom(size, payload);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom() :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    m_ConfigurationVersion(1),
    m_Profile(0),
    m_Level(0),
    m_ProfileCompatibility(0),
    m_NaluLengthSize(0)
{
    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(const AP4_AvccAtom& other) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, other.m_Size32),
    m_ConfigurationVersion(other.m_ConfigurationVersion),
    m_Profile(other.m_Profile),
    m_Level(other.m_Level),
    m_ProfileCompatibility(other.m_ProfileCompatibility),
    m_NaluLengthSize(other.m_NaluLengthSize),
    m_RawBytes(other.m_RawBytes)
{
    int state = 0;
    unsigned int i = 0;
    while (1) {
        switch (state) {
            case 0:
                for (i = 0; i < other.m_SequenceParameters.ItemCount(); i++) {
                    m_SequenceParameters.Append(other.m_SequenceParameters[i]);
                }
                state = 1;
                break;
            case 1:
                for (i = 0; i < other.m_PictureParameters.ItemCount(); i++) {
                    m_PictureParameters.Append(other.m_PictureParameters[i]);
                }
                state = 2;
                break;
            case 2:
                return;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI32 size, const AP4_UI08* payload) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, size)
{
    int state = 0;
    unsigned int payload_size;
    AP4_UI08 num_seq_params;
    unsigned int cursor;
    AP4_UI08 num_pic_params;
    while (1) {
        switch (state) {
            case 0:
                payload_size = size - AP4_ATOM_HEADER_SIZE;
                m_RawBytes.SetData(payload, payload_size);
                state = 1;
                break;
            case 1:
                m_ConfigurationVersion = payload[0];
                m_Profile = payload[1];
                m_ProfileCompatibility = payload[2];
                m_Level = payload[3];
                m_NaluLengthSize = 1 + (payload[4] & 3);
                num_seq_params = payload[5] & 31;
                m_SequenceParameters.EnsureCapacity(num_seq_params);
                cursor = 6;
                state = 2;
                break;
            case 2:
                for (unsigned int i = 0; i < num_seq_params; i++) {
                    if (cursor + 2 <= payload_size) {
                        AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
                        cursor += 2;
                        if (cursor + param_length < payload_size) {
                            m_SequenceParameters.Append(AP4_DataBuffer());
                            m_SequenceParameters[i].SetData(&payload[cursor], param_length);
                            cursor += param_length;
                        }
                    }
                }
                state = 3;
                break;
            case 3:
                num_pic_params = payload[cursor++];
                m_PictureParameters.EnsureCapacity(num_pic_params);
                state = 4;
                break;
            case 4:
                for (unsigned int i = 0; i < num_pic_params; i++) {
                    if (cursor + 2 <= payload_size) {
                        AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
                        cursor += 2;
                        if (cursor + param_length < payload_size) {
                            m_PictureParameters.Append(AP4_DataBuffer());
                            m_PictureParameters[i].SetData(&payload[cursor], param_length);
                            cursor += param_length;
                        }
                    }
                }
                return;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI08                         profile, 
                           AP4_UI08                         level, 
                           AP4_UI08                         profile_compatibility, 
                           AP4_UI08                         length_size, 
                           const AP4_Array<AP4_DataBuffer>& sequence_parameters, 
                           const AP4_Array<AP4_DataBuffer>& picture_parameters) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    m_ConfigurationVersion(1),
    m_Profile(profile),
    m_Level(level),
    m_ProfileCompatibility(profile_compatibility),
    m_NaluLengthSize(length_size)
{
    int state = 0;
    unsigned int i = 0;
    while (1) {
        switch (state) {
            case 0:
                for (i = 0; i < sequence_parameters.ItemCount(); i++) {
                    m_SequenceParameters.Append(sequence_parameters[i]);
                }
                state = 1;
                break;
            case 1:
                for (i = 0; i < picture_parameters.ItemCount(); i++) {
                    m_PictureParameters.Append(picture_parameters[i]);
                }
                state = 2;
                break;
            case 2:
                UpdateRawBytes();
                state = 3;
                break;
            case 3:
                m_Size32 += m_RawBytes.GetDataSize();
                return;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::UpdateRawBytes
+---------------------------------------------------------------------*/
void
AP4_AvccAtom::UpdateRawBytes()
{
    int state = 0;
    unsigned int payload_size;
    unsigned int cursor;
    while (1) {
        switch (state) {
            case 0:
                payload_size = 6;
                for (unsigned int i = 0; i < m_SequenceParameters.ItemCount(); i++) {
                    payload_size += 2 + m_SequenceParameters[i].GetDataSize();
                }
                ++payload_size;
                for (unsigned int i = 0; i < m_PictureParameters.ItemCount(); i++) {
                    payload_size += 2 + m_PictureParameters[i].GetDataSize();
                }
                m_RawBytes.SetDataSize(payload_size);
                state = 1;
                break;
            case 1:
                AP4_UI08* payload = m_RawBytes.UseData();
                payload[0] = m_ConfigurationVersion;
                payload[1] = m_Profile;
                payload[2] = m_ProfileCompatibility;
                payload[3] = m_Level;
                payload[4] = 0xFC | (m_NaluLengthSize - 1);
                payload[5] = 0xE0 | (AP4_UI08)m_SequenceParameters.ItemCount();
                cursor = 6;
                state = 2;
                break;
            case 2:
                for (unsigned int i = 0; i < m_SequenceParameters.ItemCount(); i++) {
                    AP4_UI16 param_length = (AP4_UI16)m_SequenceParameters[i].GetDataSize();
                    AP4_BytesFromUInt16BE(&payload[cursor], param_length);
                    cursor += 2;
                    AP4_CopyMemory(&payload[cursor], m_SequenceParameters[i].GetData(), param_length);
                    cursor += param_length;
                }
                state = 3;
                break;
            case 3:
                payload[cursor++] = (AP4_UI08)m_PictureParameters.ItemCount();
                for (unsigned int i = 0; i < m_PictureParameters.ItemCount(); i++) {
                    AP4_UI16 param_length = (AP4_UI16)m_PictureParameters[i].GetDataSize();
                    AP4_BytesFromUInt16BE(&payload[cursor], param_length);
                    cursor += 2;
                    AP4_CopyMemory(&payload[cursor], m_PictureParameters[i].GetData(), param_length);
                    cursor += param_length;
                }
                return;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_AvccAtom::WriteFields(AP4_ByteStream& stream)
{
    return stream.Write(m_RawBytes.GetData(), m_RawBytes.GetDataSize());
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_AvccAtom::InspectFields(AP4_AtomInspector& inspector)
{
    int state = 0;
    const char* profile_name;
    while (1) {
        switch (state) {
            case 0:
                inspector.AddField("Configuration Version", m_ConfigurationVersion);
                profile_name = GetProfileName(m_Profile);
                state = 1;
                break;
            case 1:
                if (profile_name) {
                    inspector.AddField("Profile", profile_name);
                } else {
                    inspector.AddField("Profile", m_Profile);
                }
                inspector.AddField("Profile Compatibility", m_ProfileCompatibility, AP4_AtomInspector::HINT_HEX);
                inspector.AddField("Level", m_Level);
                inspector.AddField("NALU Length Size", m_NaluLengthSize);
                state = 2;
                break;
            case 2:
                for (unsigned int i = 0; i < m_SequenceParameters.ItemCount(); i++) {
                    inspector.AddField("Sequence Parameter", m_SequenceParameters[i].GetData(), m_SequenceParameters[i].GetDataSize());
                }
                state = 3;
                break;
            case 3:
                for (unsigned int i = 0; i < m_PictureParameters.ItemCount(); i++) {
                    inspector.AddField("Picture Parameter", m_PictureParameters[i].GetData(), m_PictureParameters[i].GetDataSize());
                }
                return AP4_SUCCESS;
        }
    }
}
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
    AP4_UI08 profiles[] = {AP4_AVC_PROFILE_BASELINE, AP4_AVC_PROFILE_MAIN, AP4_AVC_PROFILE_EXTENDED,
                           AP4_AVC_PROFILE_HIGH, AP4_AVC_PROFILE_HIGH_10, AP4_AVC_PROFILE_HIGH_422,
                           AP4_AVC_PROFILE_HIGH_444};
    const char* names[] = {"Baseline", "Main", "Extended", "High", "High 10", "High 4:2:2", "High 4:4:4"};
    for (int i = 0; i < 7; ++i) {
        if (profile == profiles[i]) return names[i];
    }
    return NULL;
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::Create
+---------------------------------------------------------------------*/
AP4_AvccAtom* 
AP4_AvccAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI08 payload[3];
    unsigned int payload_size = size-AP4_ATOM_HEADER_SIZE;
    AP4_DataBuffer payload_data(payload_size);
    AP4_Result result = stream.Read(payload_data.UseData(), payload_size);
    if (AP4_FAILED(result)) return NULL;
    
    const AP4_UI08* data = payload_data.GetData();
    if (data[0] != 1 || payload_size < 6) return NULL;
    
    unsigned int cursor = 6;
    payload[0] = data[5] & 31;
    while (payload[0]--) {
        if (cursor+2 > payload_size) return NULL;
        cursor += 2 + AP4_BytesToInt16BE(&data[cursor]);
        if (cursor > payload_size) return NULL;
    }
    payload[1] = data[cursor++];
    if (cursor > payload_size) return NULL;
    while (payload[1]--) {
        if (cursor+2 > payload_size) return NULL;
        cursor += 2 + AP4_BytesToInt16BE(&data[cursor]);
        if (cursor > payload_size) return NULL;
    }
    return new AP4_AvccAtom(size, data);
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom() :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE)
{
    AP4_UI08 config[] = {1, 0, 0, 0, 0};
    m_ConfigurationVersion = config[0];
    m_ProfileCompatibility = config[3];
    m_Level = config[2];
    m_NaluLengthSize = config[4];
    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(const AP4_AvccAtom& other) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, other.m_Size32),
    m_ConfigurationVersion(other.m_ConfigurationVersion),
    m_ProfileCompatibility(other.m_ProfileCompatibility),
    m_Level(other.m_Level),
    m_NaluLengthSize(other.m_NaluLengthSize),
    m_RawBytes(other.m_RawBytes)
{
    unsigned int i = 0;
    for (i=0; i<other.m_SequenceParameters.ItemCount(); i++) {
        m_SequenceParameters.Append(other.m_SequenceParameters[i]);
    }
    for (i=0; i<other.m_PictureParameters.ItemCount(); i++) {
        m_PictureParameters.Append(other.m_PictureParameters[i]);
    }    
    m_Profile = other.m_Profile;
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI32 size, const AP4_UI08* payload) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, size)
{
    unsigned int payload_size = size-AP4_ATOM_HEADER_SIZE;
    m_RawBytes.SetData(payload, payload_size);

    AP4_UI08 details[5];
    details[0] = payload[0];
    details[1] = payload[1];
    details[2] = payload[2];
    details[3] = payload[3];
    details[4] = 1+(payload[4]&3);
    
    m_ConfigurationVersion = details[0];
    m_Profile = details[1];
    m_ProfileCompatibility = details[2];
    m_Level = details[3];
    m_NaluLengthSize = details[4];
    
    AP4_UI08 num_seq_params = payload[5]&31;
    m_SequenceParameters.EnsureCapacity(num_seq_params);
    unsigned int cursor = 6;
    for (unsigned int i=0; i<num_seq_params; i++) {
        if (cursor+2 <= payload_size) {
            AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + param_length < payload_size) {
                m_SequenceParameters.Append(AP4_DataBuffer());
                m_SequenceParameters[i].SetData(&payload[cursor], param_length);
                cursor += param_length;
            }
        }
    }
    AP4_UI08 num_pic_params = payload[cursor++];
    m_PictureParameters.EnsureCapacity(num_pic_params);
    for (unsigned int i=0; i<num_pic_params; i++) {
        if (cursor+2 <= payload_size) {
            AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + param_length < payload_size) {
                m_PictureParameters.Append(AP4_DataBuffer());
                m_PictureParameters[i].SetData(&payload[cursor], param_length);
                cursor += param_length;
            }
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
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE)
{
    AP4_UI08 config[] = {1, profile, level, profile_compatibility, length_size};
    m_ConfigurationVersion = config[0];
    m_Profile = config[1];
    m_Level = config[2];
    m_ProfileCompatibility = config[3];
    m_NaluLengthSize = config[4];
    
    unsigned int i = 0;
    for (i=0; i<sequence_parameters.ItemCount(); i++) {
        m_SequenceParameters.Append(sequence_parameters[i]);
    }
    for (i=0; i<picture_parameters.ItemCount(); i++) {
        m_PictureParameters.Append(picture_parameters[i]);
    }    

    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::UpdateRawBytes
+---------------------------------------------------------------------*/
void
AP4_AvccAtom::UpdateRawBytes()
{
    unsigned int payload_size = 6;    
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        payload_size += 2+m_SequenceParameters[i].GetDataSize();
    }
    ++payload_size;
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        payload_size += 2+m_PictureParameters[i].GetDataSize();
    }
    m_RawBytes.SetDataSize(payload_size);
    AP4_UI08* payload = m_RawBytes.UseData();

    AP4_UI08 details[] = {m_ConfigurationVersion, m_Profile, m_ProfileCompatibility, m_Level, m_NaluLengthSize - 1};
    for (int i = 0; i < 5; ++i) {
        payload[i] = details[i];
    }
    payload[4] = 0xFC | (details[4]);
    payload[5] = 0xE0 | (AP4_UI08)m_SequenceParameters.ItemCount();
    unsigned int cursor = 6;
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        AP4_UI16 param_length = (AP4_UI16)m_SequenceParameters[i].GetDataSize();
        AP4_BytesFromUInt16BE(&payload[cursor], param_length);
        cursor += 2;
        AP4_CopyMemory(&payload[cursor], m_SequenceParameters[i].GetData(), param_length);
        cursor += param_length;
    }
    payload[cursor++] = (AP4_UI08)m_PictureParameters.ItemCount();
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        AP4_UI16 param_length = (AP4_UI16)m_PictureParameters[i].GetDataSize();
        AP4_BytesFromUInt16BE(&payload[cursor], param_length);
        cursor += 2;
        AP4_CopyMemory(&payload[cursor], m_PictureParameters[i].GetData(), param_length);
        cursor += param_length;
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
    inspector.AddField("Configuration Version", m_ConfigurationVersion);
    const char* profile_name = GetProfileName(m_Profile);
    if (profile_name) {
        inspector.AddField("Profile", profile_name);
    } else {
        inspector.AddField("Profile", m_Profile);
    }
    inspector.AddField("Profile Compatibility", m_ProfileCompatibility, AP4_AtomInspector::HINT_HEX);
    inspector.AddField("Level", m_Level);
    inspector.AddField("NALU Length Size", m_NaluLengthSize);
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        inspector.AddField("Sequence Parameter", m_SequenceParameters[i].GetData(), m_SequenceParameters[i].GetDataSize());
    }
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        inspector.AddField("Picture Parameter", m_PictureParameters[i].GetData(), m_PictureParameters[i].GetDataSize());
    }
    return AP4_SUCCESS;
}
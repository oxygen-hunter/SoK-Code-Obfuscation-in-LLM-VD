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
    if (profile == AP4_AVC_PROFILE_BASELINE) return "Baseline";
    if (profile == AP4_AVC_PROFILE_MAIN) return "Main";
    if (profile == AP4_AVC_PROFILE_EXTENDED) return "Extended";
    if (profile == AP4_AVC_PROFILE_HIGH) return "High";
    if (profile == AP4_AVC_PROFILE_HIGH_10) return "High 10";
    if (profile == AP4_AVC_PROFILE_HIGH_422) return "High 4:2:2";
    if (profile == AP4_AVC_PROFILE_HIGH_444) return "High 4:4:4";

    return NULL;
}

unsigned int SequenceParamsCursor(const AP4_UI08* payload, unsigned int cursor, unsigned int num_seq_params, unsigned int payload_size) {
    if (!num_seq_params) return cursor;
    if (cursor+2 > payload_size) return 0;
    cursor += 2+AP4_BytesToInt16BE(&payload[cursor]);
    if (cursor > payload_size) return 0;
    return SequenceParamsCursor(payload, cursor, num_seq_params - 1, payload_size);
}

unsigned int PictureParamsCursor(const AP4_UI08* payload, unsigned int cursor, unsigned int num_pic_params, unsigned int payload_size) {
    if (!num_pic_params) return cursor;
    if (cursor+2 > payload_size) return 0;
    cursor += 2+AP4_BytesToInt16BE(&payload[cursor]);
    if (cursor > payload_size) return 0;
    return PictureParamsCursor(payload, cursor, num_pic_params - 1, payload_size);
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::Create
+---------------------------------------------------------------------*/
AP4_AvccAtom* 
AP4_AvccAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    unsigned int payload_size = size-AP4_ATOM_HEADER_SIZE;
    AP4_DataBuffer payload_data(payload_size);
    AP4_Result result = stream.Read(payload_data.UseData(), payload_size);
    if (AP4_FAILED(result)) return NULL;
    
    const AP4_UI08* payload = payload_data.GetData();
    if (payload[0] != 1) return NULL;
    if (payload_size < 6) return NULL;
    
    unsigned int num_seq_params = payload[5]&31;
    unsigned int cursor = 6;
    cursor = SequenceParamsCursor(payload, cursor, num_seq_params, payload_size);
    if (!cursor) return NULL;
    
    unsigned int num_pic_params = payload[cursor++];
    if (cursor > payload_size) return NULL;
    cursor = PictureParamsCursor(payload, cursor, num_pic_params, payload_size);
    if (!cursor) return NULL;
    
    return new AP4_AvccAtom(size, payload);
}

void CopySeqParams(const AP4_AvccAtom& other, AP4_Array<AP4_DataBuffer>& seq_params, unsigned int i) {
    if (i >= other.m_SequenceParameters.ItemCount()) return;
    seq_params.Append(other.m_SequenceParameters[i]);
    CopySeqParams(other, seq_params, i + 1);
}

void CopyPicParams(const AP4_AvccAtom& other, AP4_Array<AP4_DataBuffer>& pic_params, unsigned int i) {
    if (i >= other.m_PictureParameters.ItemCount()) return;
    pic_params.Append(other.m_PictureParameters[i]);
    CopyPicParams(other, pic_params, i + 1);
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
    CopySeqParams(other, m_SequenceParameters, 0);
    CopyPicParams(other, m_PictureParameters, 0);
}

void ParseSequenceParameters(const AP4_UI08* payload, unsigned int cursor, unsigned int num_seq_params, unsigned int payload_size, AP4_Array<AP4_DataBuffer>& sequence_parameters) {
    if (!num_seq_params) return;
    if (cursor+2 <= payload_size) {
        AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
        cursor += 2;
        if (cursor + param_length < payload_size) {
            sequence_parameters.Append(AP4_DataBuffer());
            sequence_parameters[sequence_parameters.ItemCount()-1].SetData(&payload[cursor], param_length);
            cursor += param_length;
        }
    }
    ParseSequenceParameters(payload, cursor, num_seq_params - 1, payload_size, sequence_parameters);
}

void ParsePictureParameters(const AP4_UI08* payload, unsigned int cursor, unsigned int num_pic_params, unsigned int payload_size, AP4_Array<AP4_DataBuffer>& picture_parameters) {
    if (!num_pic_params) return;
    if (cursor+2 <= payload_size) {
        AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
        cursor += 2;
        if (cursor + param_length < payload_size) {
            picture_parameters.Append(AP4_DataBuffer());
            picture_parameters[picture_parameters.ItemCount()-1].SetData(&payload[cursor], param_length);
            cursor += param_length;
        }
    }
    ParsePictureParameters(payload, cursor, num_pic_params - 1, payload_size, picture_parameters);
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI32 size, const AP4_UI08* payload) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, size)
{
    unsigned int payload_size = size-AP4_ATOM_HEADER_SIZE;
    m_RawBytes.SetData(payload, payload_size);

    m_ConfigurationVersion = payload[0];
    m_Profile              = payload[1];
    m_ProfileCompatibility = payload[2];
    m_Level                = payload[3];
    m_NaluLengthSize       = 1+(payload[4]&3);
    AP4_UI08 num_seq_params = payload[5]&31;
    m_SequenceParameters.EnsureCapacity(num_seq_params);
    unsigned int cursor = 6;
    ParseSequenceParameters(payload, cursor, num_seq_params, payload_size, m_SequenceParameters);
    AP4_UI08 num_pic_params = payload[cursor++];
    m_PictureParameters.EnsureCapacity(num_pic_params);
    ParsePictureParameters(payload, cursor, num_pic_params, payload_size, m_PictureParameters);
}

void AppendSequenceParams(const AP4_Array<AP4_DataBuffer>& source, AP4_Array<AP4_DataBuffer>& target, unsigned int i) {
    if (i >= source.ItemCount()) return;
    target.Append(source[i]);
    AppendSequenceParams(source, target, i + 1);
}

void AppendPictureParams(const AP4_Array<AP4_DataBuffer>& source, AP4_Array<AP4_DataBuffer>& target, unsigned int i) {
    if (i >= source.ItemCount()) return;
    target.Append(source[i]);
    AppendPictureParams(source, target, i + 1);
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
    AppendSequenceParams(sequence_parameters, m_SequenceParameters, 0);
    AppendPictureParams(picture_parameters, m_PictureParameters, 0);

    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

unsigned int ComputeRawBytesSize(const AP4_Array<AP4_DataBuffer>& seq_params, const AP4_Array<AP4_DataBuffer>& pic_params, unsigned int i, unsigned int j) {
    if (i < seq_params.ItemCount()) {
        return 2 + seq_params[i].GetDataSize() + ComputeRawBytesSize(seq_params, pic_params, i + 1, j);
    }
    if (j < pic_params.ItemCount()) {
        return 2 + pic_params[j].GetDataSize() + ComputeRawBytesSize(seq_params, pic_params, i, j + 1);
    }
    return 6 + 1;
}

void SetSequenceParams(AP4_UI08* payload, const AP4_Array<AP4_DataBuffer>& seq_params, unsigned int cursor, unsigned int i) {
    if (i >= seq_params.ItemCount()) return;
    AP4_UI16 param_length = (AP4_UI16)seq_params[i].GetDataSize();
    AP4_BytesFromUInt16BE(&payload[cursor], param_length);
    cursor += 2;
    AP4_CopyMemory(&payload[cursor], seq_params[i].GetData(), param_length);
    cursor += param_length;
    SetSequenceParams(payload, seq_params, cursor, i + 1);
}

void SetPictureParams(AP4_UI08* payload, const AP4_Array<AP4_DataBuffer>& pic_params, unsigned int cursor, unsigned int j) {
    if (j >= pic_params.ItemCount()) return;
    AP4_UI16 param_length = (AP4_UI16)pic_params[j].GetDataSize();
    AP4_BytesFromUInt16BE(&payload[cursor], param_length);
    cursor += 2;
    AP4_CopyMemory(&payload[cursor], pic_params[j].GetData(), param_length);
    cursor += param_length;
    SetPictureParams(payload, pic_params, cursor, j + 1);
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::UpdateRawBytes
+---------------------------------------------------------------------*/
void
AP4_AvccAtom::UpdateRawBytes()
{
    unsigned int payload_size = ComputeRawBytesSize(m_SequenceParameters, m_PictureParameters, 0, 0);
    m_RawBytes.SetDataSize(payload_size);
    AP4_UI08* payload = m_RawBytes.UseData();

    payload[0] = m_ConfigurationVersion;
    payload[1] = m_Profile;
    payload[2] = m_ProfileCompatibility;
    payload[3] = m_Level;
    payload[4] = 0xFC | (m_NaluLengthSize-1);
    payload[5] = 0xE0 | (AP4_UI08)m_SequenceParameters.ItemCount();
    unsigned int cursor = 6;
    SetSequenceParams(payload, m_SequenceParameters, cursor, 0);
    payload[cursor++] = (AP4_UI08)m_PictureParameters.ItemCount();
    SetPictureParams(payload, m_PictureParameters, cursor, 0);
}
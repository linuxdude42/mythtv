/*
 *  Class TestMPEGTables
 *
 *  Copyright (C) Karl Dietz 2013
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "test_mpegtables.h"

#include <iconv.h>
#include <iostream>

#include "libmythtv/mpeg/atsc_huffman.h"
#include "libmythtv/mpeg/atsctables.h"
#include "libmythtv/mpeg/dvbtables.h"
#include "libmythtv/mpeg/mpegtables.h"

extern "C" {
#include "libavutil/crc.h"
#include "libavutil/bswap.h"
}

template <typename T>
void mpeg_test_throw(const PSIPTable& si_table,
                     PsipParseException::Code error_code)
{
    try
    {
        T table(si_table);
        QFAIL(qPrintable(QStringLiteral("This test should throw a MpegParseException.")));
    }
    catch (const MpegParseException& e)
    {
        QCOMPARE(e.what(), "Mpeg Parse Error");
        QCOMPARE(e.m_error, error_code);
    }
    catch (const PsipParseException& e)
    {
        QFAIL("Expected a MpegParseException error, received a PsipParseException");
    }
    catch (const std::runtime_error& e)
    {
        QFAIL("Expected a MpegParseException error, received a runtime_error");
    }
}

template <typename T>
void atsc_test_throw(const PSIPTable& si_table,
                     PsipParseException::Code error_code)
{
    try
    {
        T table(si_table);
        QFAIL(qPrintable(QStringLiteral("This test should throw an AtscParseException.")));
    }
    catch (const AtscParseException& e)
    {
        QCOMPARE(e.what(), "Atsc Parse Error");
        QCOMPARE(e.m_error, error_code);
    }
    catch (const PsipParseException& e)
    {
        QFAIL("Expected a AtscParseException error, received a PsipParseException");
    }
    catch (const std::runtime_error& e)
    {
        QFAIL("Expected a AtscParseException error, received a runtime_error");
    }
}

void TestMPEGTables::update_crc(std::vector<uint8_t> &data)
{
    uint32_t crc = av_bswap32(av_crc(av_crc_get_table(AV_CRC_32_IEEE), UINT32_MAX,
                             data.data(), data.size() - 4));
    std::size_t size = data.size();
    data[size - 4] = (crc & 0xFF000000) >> 24;
    data[size - 3] = (crc & 0x00FF0000) >> 16;
    data[size - 2] = (crc & 0x0000FF00) >>  8;
    data[size - 1] = (crc & 0x000000FF);
}

static std::array<uint8_t,3+8*12> high8 {
    0x10, 0x00, 0x00,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

void TestMPEGTables::pat_test(void)
{
    const std::vector<uint8_t> si_data {
        0x00, 0xb0, 0x31, 0x04, 0x37, 0xdf, 0x00, 0x00,  0x2b, 0x66, 0xf7, 0xd4, 0x6d, 0x66, 0xe0, 0x64,  /* ..1.7...+f..mf.d */
        0x6d, 0x67, 0xe0, 0xc8, 0x6d, 0x68, 0xe1, 0x2c,  0x6d, 0x6b, 0xe2, 0x58, 0x6d, 0x6c, 0xe2, 0xbc,  /* mg..mh.,mk.Xml.. */
        0x6d, 0x6d, 0xe3, 0x20, 0x6d, 0x6e, 0xe2, 0x8a,  0x6d, 0x70, 0xe4, 0x4c, 0x6d, 0x71, 0xe1, 0x9b,  /* mm. mn..mp.Lmq.. */
        0xc0, 0x79, 0xa6, 0x2b                                                                            /* .y.+ */
    };

    PSIPTable si_table(si_data);

    QVERIFY  (si_table.IsGood());

    ProgramAssociationTable pat(si_table);

    QVERIFY  (pat.HasCRC());
    QVERIFY  (pat.VerifyCRC());
    QCOMPARE (si_table.Length(),       (uint32_t)  0x31);
    QCOMPARE (pat.Length(),            (uint32_t)    49);
    QCOMPARE (pat.SectionLength(),     (uint32_t)    49+3);
    QCOMPARE (pat.TransportStreamID(), (uint32_t)  1079);
    QCOMPARE (pat.ProgramCount(),      (uint32_t)    10);

    QCOMPARE (pat.ProgramNumber(0),    (uint32_t) 11110);
    QCOMPARE (pat.ProgramPID(0),       (uint32_t)  6100);

    QCOMPARE (pat.ProgramNumber(1),    (uint32_t) 28006);
    QCOMPARE (pat.ProgramPID(1),       (uint32_t)   100);

    QCOMPARE (pat.ProgramNumber(2),    (uint32_t) 28007);
    QCOMPARE (pat.ProgramPID(2),       (uint32_t)   200);

    QCOMPARE (pat.ProgramNumber(3),    (uint32_t) 28008);
    QCOMPARE (pat.ProgramPID(3),       (uint32_t)   300);

    QCOMPARE (pat.ProgramNumber(4),    (uint32_t) 28011);
    QCOMPARE (pat.ProgramPID(4),       (uint32_t)   600);

    QCOMPARE (pat.ProgramNumber(5),    (uint32_t) 28012);
    QCOMPARE (pat.ProgramPID(5),       (uint32_t)   700);

    QCOMPARE (pat.ProgramNumber(6),    (uint32_t) 28013);
    QCOMPARE (pat.ProgramPID(6),       (uint32_t)   800);

    QCOMPARE (pat.ProgramNumber(7),    (uint32_t) 28014);
    QCOMPARE (pat.ProgramPID(7),       (uint32_t)   650);

    QCOMPARE (pat.ProgramNumber(8),    (uint32_t) 28016);
    QCOMPARE (pat.ProgramPID(8),       (uint32_t)  1100);

    QCOMPARE (pat.ProgramNumber(9),    (uint32_t) 28017);
    QCOMPARE (pat.ProgramPID(9),       (uint32_t)   411);

    // go out of bounds and see what happens if we parse the CRC
    QCOMPARE (pat.ProgramNumber(10) << 16 | pat.ProgramPID(10), pat.CRC() & 0xFFFF1FFF);

    QCOMPARE (pat.FindPID(11110),      (uint32_t)  6100); // the first
    QCOMPARE (pat.FindPID(28017),      (uint32_t)   411); // the last
    QCOMPARE (pat.FindPID(28008),      (uint32_t)   300); // random one
    QCOMPARE (pat.FindPID(12345),      (uint32_t)     0); // not found

    QCOMPARE (pat.FindProgram(6100),   (uint32_t) 11110); // the first
    QCOMPARE (pat.FindProgram(411),    (uint32_t) 28017); // the last
    QCOMPARE (pat.FindProgram(300),    (uint32_t) 28008); // random one
    QCOMPARE (pat.FindProgram(12345),  (uint32_t)     0); // not found

    // first PID that is a real PMT PID and not the NIT PID
    QCOMPARE (pat.FindAnyPID(),        (uint32_t)  6100);

    // Create a PAT no CRC error
    std::vector<uint> pnums;
    std::vector<uint> pids;
    pnums.push_back(1);
    pids.push_back(0x100);
    ProgramAssociationTable* pat2 =
        ProgramAssociationTable::Create(1, 0, pnums, pids);
    QVERIFY (pat2->VerifyCRC());

    // Create a blank PAT, CRC error!
    ProgramAssociationTable* pat3 =
        ProgramAssociationTable::CreateBlank();
    QVERIFY (!pat3->VerifyCRC());
    // we still have not found "CRC mismatch 0 != 0xFFFFFFFF"
    QCOMPARE (pat3->CalcCRC(), (uint) 0x334FF8A0);
    pat3->SetCRC(pat3->CalcCRC());
    QVERIFY (pat3->VerifyCRC());

    // Create a PAT object
    std::vector<uint8_t> si_data4(188,'\0');
    si_data4[1] = 1 << 7 & 0 << 6 & 3 << 4 & 0 << 2 & 0;
    si_data4[2] = 0x00;
    auto* pat4 = new ProgramAssociationTable(PSIPTable(si_data4));
    QCOMPARE (pat4->CalcCRC(), (uint) 0xFFFFFFFF);
    QVERIFY (pat4->VerifyCRC());
    delete pat4;
}

//
// ProgramMapTable Tests
//

// Packet broadcast over cable from WBAL Baltimore.
const std::vector<uint8_t> wbal_pmt_data {
    0x02, 0xB0, 0x7E, 0x04, 0x03, 0xDD, 0x00, 0x00, 0xF3, 0x9D, 0xF0, 0x1B, 0x05, 0x04, 0x47, 0x41,
    0x39, 0x34, 0x87, 0x13, 0xC1, 0x01, 0x01, 0x00, 0xF4, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00,
    0x00, 0x05, 0x54, 0x56, 0x2D, 0x31, 0x34, 0x02, 0xF3, 0x9D, 0xF0, 0x17, 0x11, 0x01, 0xFF, 0x10,
    0x06, 0xC0, 0xBD, 0x62, 0xC0, 0x08, 0x00, 0x06, 0x01, 0x02, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67,
    0xC1, 0x3F, 0xFF, 0x81, 0xF3, 0x9E, 0xF0, 0x18, 0x05, 0x04, 0x41, 0x43, 0x2D, 0x33, 0x81, 0x0A,
    0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01, 0xBF, 0x65, 0x6E, 0x67, 0x0A, 0x04, 0x65, 0x6E, 0x67, 0x00,
    0x81, 0xF3, 0x9F, 0xF0, 0x18, 0x05, 0x04, 0x41, 0x43, 0x2D, 0x33, 0x81, 0x0A, 0x08, 0x28, 0x45,
    0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61, 0x0A, 0x04, 0x73, 0x70, 0x61, 0x00, 0x60, 0x13, 0xAB,
    0x75,
};

// Packet received via cable card from ESPN.
const std::vector<uint8_t> espn_pmt_data {
    0x02, 0xB0, 0x81, 0x02, 0x5A, 0xE9, 0x00, 0x00, 0xF7, 0x85, 0xF0, 0x12, 0x09, 0x04, 0x0E, 0x00,
    0xF7, 0x8C, 0x09, 0x04, 0x47, 0x49, 0xF7, 0x8D, 0x05, 0x04, 0x43, 0x55, 0x45, 0x49, 0x02, 0xF7,
    0x85, 0xF0, 0x09, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF, 0x81, 0xF7, 0x86, 0xF0,
    0x18, 0x05, 0x04, 0x41, 0x43, 0x2D, 0x33, 0x81, 0x0A, 0x08, 0x3C, 0x1B, 0xFF, 0x1F, 0x01, 0xBF,
    0x65, 0x6E, 0x67, 0x0A, 0x04, 0x65, 0x6E, 0x67, 0x00, 0x81, 0xF7, 0x87, 0xF0, 0x18, 0x05, 0x04,
    0x41, 0x43, 0x2D, 0x33, 0x81, 0x0A, 0x08, 0x28, 0x05, 0xFF, 0x1F, 0x01, 0xBF, 0x73, 0x70, 0x61,
    0x0A, 0x04, 0x73, 0x70, 0x61, 0x00, 0x05, 0xF7, 0x88, 0xF0, 0x08, 0xBF, 0x06, 0x49, 0x6E, 0x76,
    0x69, 0x64, 0x69, 0x05, 0xF7, 0x8A, 0xF0, 0x08, 0xC0, 0x06, 0x49, 0x6E, 0x76, 0x69, 0x64, 0x69,
    0xFC, 0x6C, 0x7A, 0x1E,
};

// Packet received via cable card for Music Choice.
const std::vector<uint8_t> mc_pmt_data {
    0x02, 0xB0, 0x34, 0x01, 0xA3, 0xEB, 0x00, 0x00, 0xF0, 0x60, 0xF0, 0x0C, 0x09, 0x04, 0x0E, 0x00,
    0xF0, 0x66, 0x09, 0x04, 0x47, 0x49, 0xF0, 0x67, 0x02, 0xF0, 0x5F, 0xF0, 0x05, 0x02, 0x03, 0x23,
    0x48, 0x5F, 0x81, 0xF0, 0x60, 0xF0, 0x0C, 0x05, 0x04, 0x41, 0x43, 0x2D, 0x33, 0x81, 0x04, 0x08,
    0x28, 0x04, 0x00, 0x65, 0x8C, 0xBE, 0xE5,
};

// Packet received from a New Zealand station
const std::vector<uint8_t> nz_pmt_data {
    0x02, 0xB0, 0x4D, 0x06, 0x41, 0xF3, 0x00, 0x00, 0xE1, 0x69, 0xF0, 0x00, 0x05, 0xFF, 0x40, 0xF0,
    0x05, 0x6F, 0x03, 0x00, 0x10, 0xE0, 0x0B, 0xE3, 0xEC, 0xF0, 0x12, 0x52, 0x01, 0x0B, 0x66, 0x06,
    0x01, 0x06, 0x05, 0x05, 0x00, 0x00, 0x13, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1B, 0xE1, 0x69,
    0xF0, 0x05, 0x0E, 0x03, 0xC0, 0x1E, 0xD2, 0x11, 0xE1, 0xCD, 0xF0, 0x10, 0x0A, 0x04, 0x65, 0x6E,
    0x67, 0x00, 0x7C, 0x03, 0x58, 0x80, 0x03, 0x0E, 0x03, 0xC0, 0x00, 0xF0, 0x08, 0x6B, 0x51, 0x00,
};

void TestMPEGTables::mpeg_pmt_test1(void)
{
    PSIPTable si_table(wbal_pmt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 126U);

    // PSIP generic fields
    ProgramMapTable pmt(si_table);
    QCOMPARE (pmt.SectionSyntaxIndicator(),      true);
    QCOMPARE (pmt.PrivateIndicator(),           false);
    QCOMPARE (pmt.SectionLengthRaw(),            126U);
    QCOMPARE (pmt.ProgramNumber(),              1027U);
    QCOMPARE (pmt.Version(),                      14U);
    QCOMPARE (pmt.IsCurrent(),                   true);
    QCOMPARE (pmt.Section(),                       0U);
    QCOMPARE (pmt.LastSection(),                   0U);

    // PMT specific fields
    QCOMPARE (pmt.PCRPID(),                     5021U);
    QCOMPARE (pmt.ProgramInfoLength(),            27U);

    // PMT stream table data
    QCOMPARE (pmt.StreamCount(),                   3U);
    QCOMPARE (pmt.StreamType(0), StreamID::MPEG2Video);
    QCOMPARE (pmt.StreamPID(0),                 5021U);
    QCOMPARE (pmt.StreamInfoLength(0),            23U);
    QCOMPARE (pmt.IsVideo(0, ""),                true);
    QCOMPARE (pmt.IsAudio(0, ""),               false);

    QCOMPARE (pmt.StreamType(1),   StreamID::AC3Audio);
    QCOMPARE (pmt.StreamPID(1),                 5022U);
    QCOMPARE (pmt.StreamInfoLength(1),            24U);
    QCOMPARE (pmt.IsVideo(1, ""),               false);
    QCOMPARE (pmt.IsAudio(1, ""),                true);
    QCOMPARE (pmt.GetLanguage(1),               "eng");
    QCOMPARE (pmt.GetAudioType(1),                 0U);

    QCOMPARE (pmt.StreamType(2),   StreamID::AC3Audio);
    QCOMPARE (pmt.StreamPID(2),                 5023U);
    QCOMPARE (pmt.StreamInfoLength(2),            24U);
    QCOMPARE (pmt.IsVideo(2, ""),               false);
    QCOMPARE (pmt.IsAudio(2, ""),                true);
    QCOMPARE (pmt.GetLanguage(2),               "spa");
    QCOMPARE (pmt.GetAudioType(2),                 0U);

    // Queries
    QCOMPARE (pmt.IsEncrypted(""),              false);
    QCOMPARE (pmt.IsProgramEncrypted(),         false);
    QCOMPARE (pmt.IsStillPicture(""),           false);
    QCOMPARE (pmt.FindPID(5022),                    1);
    QCOMPARE (pmt.FindPID(5024),                   -1);
    QCOMPARE (pmt.FindUnusedPID(),                32U);
}

void TestMPEGTables::mpeg_pmt_test2(void)
{
    PSIPTable si_table(espn_pmt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 129U);

    // PSIP generic fields
    ProgramMapTable pmt(si_table);
    QCOMPARE (pmt.SectionSyntaxIndicator(),      true);
    QCOMPARE (pmt.PrivateIndicator(),           false);
    QCOMPARE (pmt.SectionLengthRaw(),            129U);
    QCOMPARE (pmt.ProgramNumber(),               602U);
    QCOMPARE (pmt.Version(),                      20U);
    QCOMPARE (pmt.IsCurrent(),                   true);
    QCOMPARE (pmt.Section(),                       0U);
    QCOMPARE (pmt.LastSection(),                   0U);

    // PMT specific fields
    QCOMPARE (pmt.PCRPID(),                     6021U);
    QCOMPARE (pmt.ProgramInfoLength(),            18U);

    // PMT stream table data
    QCOMPARE (pmt.StreamCount(),                   5U);
    QCOMPARE (pmt.StreamType(0), StreamID::MPEG2Video);
    QCOMPARE (pmt.StreamPID(0),                 6021U);
    QCOMPARE (pmt.StreamInfoLength(0),             9U);
    QCOMPARE (pmt.IsStreamEncrypted(0),         false);
    QCOMPARE (pmt.IsVideo(0, ""),                true);
    QCOMPARE (pmt.IsAudio(0, ""),               false);

    QCOMPARE (pmt.StreamType(1),   StreamID::AC3Audio);
    QCOMPARE (pmt.StreamPID(1),                 6022U);
    QCOMPARE (pmt.StreamInfoLength(1),            24U);
    QCOMPARE (pmt.IsStreamEncrypted(1),         false);
    QCOMPARE (pmt.IsVideo(1, ""),               false);
    QCOMPARE (pmt.IsAudio(1, ""),                true);
    QCOMPARE (pmt.GetLanguage(1),               "eng");
    QCOMPARE (pmt.GetAudioType(1),                 0U);

    QCOMPARE (pmt.StreamType(2),   StreamID::AC3Audio);
    QCOMPARE (pmt.StreamPID(2),                 6023U);
    QCOMPARE (pmt.StreamInfoLength(2),            24U);
    QCOMPARE (pmt.IsStreamEncrypted(2),         false);
    QCOMPARE (pmt.IsVideo(2, ""),               false);
    QCOMPARE (pmt.IsAudio(2, ""),                true);
    QCOMPARE (pmt.GetLanguage(2),               "spa");
    QCOMPARE (pmt.GetAudioType(2),                 0U);

    QCOMPARE (pmt.StreamType(3),    StreamID::PrivSec);
    QCOMPARE (pmt.StreamPID(3),                 6024U);
    QCOMPARE (pmt.StreamInfoLength(3),             8U);
    QCOMPARE (pmt.IsStreamEncrypted(3),         false);
    QCOMPARE (pmt.IsVideo(3, ""),               false);
    QCOMPARE (pmt.IsAudio(3, ""),               false);

    QCOMPARE (pmt.StreamType(4),    StreamID::PrivSec);
    QCOMPARE (pmt.StreamPID(4),                 6026U);
    QCOMPARE (pmt.StreamInfoLength(4),             8U);
    QCOMPARE (pmt.IsStreamEncrypted(4),         false);
    QCOMPARE (pmt.IsVideo(4, ""),               false);
    QCOMPARE (pmt.IsAudio(4, ""),               false);

    // Queries
    QCOMPARE (pmt.IsEncrypted(""),               true);
    QCOMPARE (pmt.IsProgramEncrypted(),          true);
    QCOMPARE (pmt.IsStillPicture(""),           false);
    QCOMPARE (pmt.FindPID(6022),                    1);
    QCOMPARE (pmt.FindPID(6024),                    3);
    QCOMPARE (pmt.FindPID(5024),                   -1);
    QCOMPARE (pmt.FindUnusedPID(),                32U);
}

void TestMPEGTables::mpeg_pmt_test3(void)
{
    PSIPTable si_table(mc_pmt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  52U);

    // PSIP generic fields
    ProgramMapTable pmt(si_table);
    QCOMPARE (pmt.SectionSyntaxIndicator(),      true);
    QCOMPARE (pmt.PrivateIndicator(),           false);
    QCOMPARE (pmt.SectionLengthRaw(),             52U);
    QCOMPARE (pmt.ProgramNumber(),               419U);
    QCOMPARE (pmt.Version(),                      21U);
    QCOMPARE (pmt.IsCurrent(),                   true);
    QCOMPARE (pmt.Section(),                       0U);
    QCOMPARE (pmt.LastSection(),                   0U);

    // PMT specific fields
    QCOMPARE (pmt.PCRPID(),                     4192U);
    QCOMPARE (pmt.ProgramInfoLength(),            12U);

    // PMT stream table data
    QCOMPARE (pmt.StreamCount(),                   2U);
    QCOMPARE (pmt.StreamType(0), StreamID::MPEG2Video);
    QCOMPARE (pmt.StreamPID(0),                 4191U);
    QCOMPARE (pmt.StreamInfoLength(0),             5U);
    QCOMPARE (pmt.IsStreamEncrypted(0),         false);
    QCOMPARE (pmt.IsVideo(0, ""),                true);
    QCOMPARE (pmt.IsAudio(0, ""),               false);

    QCOMPARE (pmt.StreamType(1),   StreamID::AC3Audio);
    QCOMPARE (pmt.StreamPID(1),                 4192U);
    QCOMPARE (pmt.StreamInfoLength(1),            12U);
    QCOMPARE (pmt.IsStreamEncrypted(1),         false);
    QCOMPARE (pmt.IsVideo(1, ""),               false);
    QCOMPARE (pmt.IsAudio(1, ""),                true);
    QCOMPARE (pmt.GetLanguage(1),                  "");
    QCOMPARE (pmt.GetAudioType(1),                 0U);

    // Queries
    QCOMPARE (pmt.IsEncrypted(""),               true);
    QCOMPARE (pmt.IsProgramEncrypted(),          true);
    QCOMPARE (pmt.IsStillPicture(""),            true);
    QCOMPARE (pmt.FindPID(4192),                    1);
    QCOMPARE (pmt.FindPID(4193),                   -1);
    QCOMPARE (pmt.FindUnusedPID(),                32U);
}

void TestMPEGTables::mpeg_pmt_test4(void)
{
    PSIPTable si_table(nz_pmt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  77U);

    // PSIP generic fields
    ProgramMapTable pmt(si_table);
    QCOMPARE (pmt.SectionSyntaxIndicator(),      true);
    QCOMPARE (pmt.PrivateIndicator(),           false);
    QCOMPARE (pmt.SectionLengthRaw(),             77U);
    QCOMPARE (pmt.ProgramNumber(),              1601U);
    QCOMPARE (pmt.Version(),                      25U);
    QCOMPARE (pmt.IsCurrent(),                   true);
    QCOMPARE (pmt.Section(),                       0U);
    QCOMPARE (pmt.LastSection(),                   0U);

    // PMT specific fields
    QCOMPARE (pmt.PCRPID(),                      361U);
    QCOMPARE (pmt.ProgramInfoLength(),             0U);

    // PMT stream table data
    QCOMPARE (pmt.StreamCount(),                   4U);
    QCOMPARE (pmt.StreamType(0),    StreamID::PrivSec);
    QCOMPARE (pmt.StreamPID(0),                 8000U);
    QCOMPARE (pmt.StreamInfoLength(0),             5U);

    QCOMPARE (pmt.StreamType(1),    StreamID::DSMCC_B);
    QCOMPARE (pmt.StreamPID(1),                 1004U);
    QCOMPARE (pmt.StreamInfoLength(1),            18U);

    QCOMPARE (pmt.StreamType(2),  StreamID::H264Video);
    QCOMPARE (pmt.StreamPID(2),                  361U);
    QCOMPARE (pmt.StreamInfoLength(2),             5U);
    QCOMPARE (pmt.IsStreamEncrypted(2),         false);
    QCOMPARE (pmt.IsVideo(2, ""),                true);
    QCOMPARE (pmt.IsAudio(2, ""),               false);

    QCOMPARE (pmt.StreamType(3), StreamID::MPEG2AudioAmd1); // MPEG-4 AAC Audio
    QCOMPARE (pmt.StreamPID(3),                  461U);
    QCOMPARE (pmt.StreamInfoLength(3),            16U);
    QCOMPARE (pmt.IsStreamEncrypted(3),         false);
    QCOMPARE (pmt.IsVideo(3, ""),               false);
    QCOMPARE (pmt.IsAudio(3, ""),                true);
    QCOMPARE (pmt.GetLanguage(3),               "eng");
    QCOMPARE (pmt.GetAudioType(3),                 0U);

    // Queries
    QCOMPARE (pmt.IsEncrypted(""),              false);
    QCOMPARE (pmt.IsProgramEncrypted(),         false);
    QCOMPARE (pmt.IsStillPicture(""),           false);
    QCOMPARE (pmt.FindPID(361),                     2);
    QCOMPARE (pmt.FindPID(362),                    -1);
    QCOMPARE (pmt.FindUnusedPID(),                32U);
}

//
// MasterGuideTable Tests
//

// Packet broadcast over cable from WBAL Baltimore.
const std::vector<uint8_t> wbal_mgt_data {
    0xC7, 0xF0, 0x7C, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x0A,
    // Table Entries
    0x00, 0x02, 0xFF, 0xFB, 0xF3, 0x00, 0x00, 0x01, 0xC2, 0xF0, 0x00,
    0x01, 0x00, 0xF0, 0x00, 0xE9, 0x00, 0x00, 0x06, 0x78, 0xF0, 0x00,
    0x01, 0x01, 0xF0, 0x01, 0xF2, 0x00, 0x00, 0x07, 0xAE, 0xF0, 0x00,
    0x01, 0x02, 0xF0, 0x02, 0xE8, 0x00, 0x00, 0x06, 0x3B, 0xF0, 0x00,
    0x01, 0x03, 0xF0, 0x03, 0xFB, 0x00, 0x00, 0x03, 0xE0, 0xF0, 0x00,
    0x02, 0x00, 0xF0, 0x80, 0xF4, 0x00, 0x00, 0x09, 0x16, 0xF0, 0x00,
    0x02, 0x01, 0xF0, 0x81, 0xE6, 0x00, 0x00, 0x0B, 0x14, 0xF0, 0x00,
    0x02, 0x02, 0xF0, 0x82, 0xFD, 0x00, 0x00, 0x03, 0xBE, 0xF0, 0x00,
    0x02, 0x03, 0xF0, 0x83, 0xE3, 0x00, 0x00, 0x06, 0x3B, 0xF0, 0x00,
    0x00, 0x04, 0xF1, 0x00, 0xEF, 0x00, 0x00, 0x00, 0x38, 0xF0, 0x00,
    // Global Descriptors
    0xF0, 0x00,
    // CRC
    0x9C, 0x95, 0xD2, 0xFD,
};

// Packet broadcast over the air from WJZ Baltimore.
const std::vector<uint8_t> wjz_mgt_data {
    0xC7, 0xF2, 0xE4, 0x00, 0x00, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0xFF, 0xFB, 0xE1,
    0x00, 0x00, 0x00, 0xFC, 0xF0, 0x00, 0x00, 0x04, 0xF4, 0x88, 0xE1, 0x00, 0x00, 0x00, 0x40, 0xF0,
    0x00, 0x01, 0x00, 0xF3, 0x88, 0xE5, 0x00, 0x00, 0x04, 0xAB, 0xF0, 0x00, 0x01, 0x01, 0xF3, 0x89,
    0xE5, 0x00, 0x00, 0x04, 0xAC, 0xF0, 0x00, 0x01, 0x02, 0xF3, 0x8A, 0xE5, 0x00, 0x00, 0x03, 0x9E,
    0xF0, 0x00, 0x01, 0x03, 0xF3, 0x8B, 0xE5, 0x00, 0x00, 0x04, 0xD3, 0xF0, 0x00, 0x01, 0x04, 0xF3,
    0x8C, 0xE5, 0x00, 0x00, 0x04, 0x07, 0xF0, 0x00, 0x01, 0x05, 0xF3, 0x8D, 0xE4, 0x00, 0x00, 0x04,
    0x32, 0xF0, 0x00, 0x01, 0x06, 0xF3, 0x8E, 0xE4, 0x00, 0x00, 0x04, 0x7F, 0xF0, 0x00, 0x01, 0x07,
    0xF3, 0x8F, 0xE4, 0x00, 0x00, 0x05, 0x66, 0xF0, 0x00, 0x01, 0x08, 0xF3, 0x90, 0xE4, 0x00, 0x00,
    0x04, 0xB3, 0xF0, 0x00, 0x01, 0x09, 0xF3, 0x91, 0xE4, 0x00, 0x00, 0x05, 0x4C, 0xF0, 0x00, 0x01,
    0x0A, 0xF3, 0x92, 0xE4, 0x00, 0x00, 0x04, 0x16, 0xF0, 0x00, 0x01, 0x0B, 0xF3, 0x93, 0xE4, 0x00,
    0x00, 0x04, 0x79, 0xF0, 0x00, 0x01, 0x0C, 0xF3, 0x94, 0xE4, 0x00, 0x00, 0x03, 0x3B, 0xF0, 0x00,
    0x01, 0x0D, 0xF3, 0x95, 0xE3, 0x00, 0x00, 0x04, 0x3C, 0xF0, 0x00, 0x01, 0x0E, 0xF3, 0x96, 0xE3,
    0x00, 0x00, 0x04, 0x7D, 0xF0, 0x00, 0x01, 0x0F, 0xF3, 0x97, 0xE3, 0x00, 0x00, 0x05, 0x38, 0xF0,
    0x00, 0x01, 0x10, 0xF3, 0x98, 0xE3, 0x00, 0x00, 0x04, 0xD8, 0xF0, 0x00, 0x01, 0x11, 0xF3, 0x99,
    0xE3, 0x00, 0x00, 0x05, 0x68, 0xF0, 0x00, 0x01, 0x12, 0xF3, 0x9A, 0xE3, 0x00, 0x00, 0x04, 0x30,
    0xF0, 0x00, 0x01, 0x13, 0xF3, 0x9B, 0xE3, 0x00, 0x00, 0x04, 0x94, 0xF0, 0x00, 0x01, 0x14, 0xF3,
    0x9C, 0xE3, 0x00, 0x00, 0x03, 0xCE, 0xF0, 0x00, 0x01, 0x15, 0xF3, 0x9D, 0xE2, 0x00, 0x00, 0x04,
    0x9C, 0xF0, 0x00, 0x01, 0x16, 0xF3, 0x9E, 0xE2, 0x00, 0x00, 0x05, 0x33, 0xF0, 0x00, 0x01, 0x17,
    0xF3, 0x9F, 0xE2, 0x00, 0x00, 0x04, 0x93, 0xF0, 0x00, 0x01, 0x18, 0xF3, 0xA0, 0xE2, 0x00, 0x00,
    0x05, 0x02, 0xF0, 0x00, 0x01, 0x19, 0xF3, 0xA1, 0xE2, 0x00, 0x00, 0x04, 0x93, 0xF0, 0x00, 0x01,
    0x1A, 0xF3, 0xA2, 0xE2, 0x00, 0x00, 0x03, 0xAE, 0xF0, 0x00, 0x01, 0x1B, 0xF3, 0xA3, 0xE2, 0x00,
    0x00, 0x04, 0xB2, 0xF0, 0x00, 0x01, 0x1C, 0xF3, 0xA4, 0xE2, 0x00, 0x00, 0x04, 0xF2, 0xF0, 0x00,
    0x01, 0x1D, 0xF3, 0xA5, 0xE1, 0x00, 0x00, 0x04, 0x82, 0xF0, 0x00, 0x01, 0x1E, 0xF3, 0xA6, 0xE1,
    0x00, 0x00, 0x05, 0x4D, 0xF0, 0x00, 0x01, 0x1F, 0xF3, 0xA7, 0xE1, 0x00, 0x00, 0x04, 0xF5, 0xF0,
    0x00, 0x02, 0x00, 0xF4, 0x08, 0xE5, 0x00, 0x00, 0x06, 0xA2, 0xF0, 0x00, 0x02, 0x01, 0xF4, 0x09,
    0xE5, 0x00, 0x00, 0x05, 0xD3, 0xF0, 0x00, 0x02, 0x02, 0xF4, 0x0A, 0xE5, 0x00, 0x00, 0x04, 0xC2,
    0xF0, 0x00, 0x02, 0x03, 0xF4, 0x0B, 0xE5, 0x00, 0x00, 0x05, 0xE9, 0xF0, 0x00, 0x02, 0x04, 0xF4,
    0x0C, 0xE5, 0x00, 0x00, 0x05, 0xA7, 0xF0, 0x00, 0x02, 0x05, 0xF4, 0x0D, 0xE4, 0x00, 0x00, 0x05,
    0x8E, 0xF0, 0x00, 0x02, 0x06, 0xF4, 0x0E, 0xE4, 0x00, 0x00, 0x06, 0x8F, 0xF0, 0x00, 0x02, 0x07,
    0xF4, 0x0F, 0xE4, 0x00, 0x00, 0x07, 0x65, 0xF0, 0x00, 0x02, 0x08, 0xF4, 0x10, 0xE4, 0x00, 0x00,
    0x07, 0x29, 0xF0, 0x00, 0x02, 0x09, 0xF4, 0x11, 0xE4, 0x00, 0x00, 0x07, 0x35, 0xF0, 0x00, 0x02,
    0x0A, 0xF4, 0x12, 0xE4, 0x00, 0x00, 0x05, 0x4B, 0xF0, 0x00, 0x02, 0x0B, 0xF4, 0x13, 0xE4, 0x00,
    0x00, 0x04, 0xDC, 0xF0, 0x00, 0x02, 0x0C, 0xF4, 0x14, 0xE4, 0x00, 0x00, 0x04, 0xCE, 0xF0, 0x00,
    0x02, 0x0D, 0xF4, 0x15, 0xE3, 0x00, 0x00, 0x05, 0x8B, 0xF0, 0x00, 0x02, 0x0E, 0xF4, 0x16, 0xE3,
    0x00, 0x00, 0x06, 0x1E, 0xF0, 0x00, 0x02, 0x0F, 0xF4, 0x17, 0xE3, 0x00, 0x00, 0x06, 0x97, 0xF0,
    0x00, 0x02, 0x10, 0xF4, 0x18, 0xE3, 0x00, 0x00, 0x06, 0x19, 0xF0, 0x00, 0x02, 0x11, 0xF4, 0x19,
    0xE3, 0x00, 0x00, 0x07, 0x81, 0xF0, 0x00, 0x02, 0x12, 0xF4, 0x1A, 0xE3, 0x00, 0x00, 0x04, 0xEA,
    0xF0, 0x00, 0x02, 0x13, 0xF4, 0x1B, 0xE3, 0x00, 0x00, 0x04, 0xEE, 0xF0, 0x00, 0x02, 0x14, 0xF4,
    0x1C, 0xE3, 0x00, 0x00, 0x05, 0x99, 0xF0, 0x00, 0x02, 0x15, 0xF4, 0x1D, 0xE2, 0x00, 0x00, 0x05,
    0xD3, 0xF0, 0x00, 0x02, 0x16, 0xF4, 0x1E, 0xE2, 0x00, 0x00, 0x06, 0x84, 0xF0, 0x00, 0x02, 0x17,
    0xF4, 0x1F, 0xE2, 0x00, 0x00, 0x06, 0x57, 0xF0, 0x00, 0x02, 0x18, 0xF4, 0x20, 0xE2, 0x00, 0x00,
    0x07, 0x1F, 0xF0, 0x00, 0x02, 0x19, 0xF4, 0x21, 0xE2, 0x00, 0x00, 0x05, 0xA2, 0xF0, 0x00, 0x02,
    0x1A, 0xF4, 0x22, 0xE2, 0x00, 0x00, 0x04, 0x87, 0xF0, 0x00, 0x02, 0x1B, 0xF4, 0x23, 0xE2, 0x00,
    0x00, 0x05, 0x5C, 0xF0, 0x00, 0x02, 0x1C, 0xF4, 0x24, 0xE2, 0x00, 0x00, 0x06, 0xB6, 0xF0, 0x00,
    0x02, 0x1D, 0xF4, 0x25, 0xE1, 0x00, 0x00, 0x04, 0xE5, 0xF0, 0x00, 0x02, 0x1E, 0xF4, 0x26, 0xE1,
    0x00, 0x00, 0x06, 0x62, 0xF0, 0x00, 0x02, 0x1F, 0xF4, 0x27, 0xE1, 0x00, 0x00, 0x07, 0x04, 0xF0,
    0x00, 0xF0, 0x00, 0xE2, 0xE8, 0xF2, 0x35,
};

// This MGT table from WBFF was dumped from the MasterGuideTable::Parse
// function.  Starting at pesdata(), TSSizeInBuffer() bytes were dumped.
// Its unclear what the bytes are after the table data.  The 0xFF bytes
// look like stuffing and the 0x00 bytes look like ???, but the TSDuck
// tools complain about all those bytes when converting this data stream
// into a textual version of the table.
const std::vector<uint8_t> wbff_mgt_data {
    0xC7, 0xF0, 0x71, 0x00, 0x00, 0xD7, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x02, 0xFF, 0xFB, 0xF8,
    0x00, 0x00, 0x01, 0x11, 0xF0, 0x00, 0x01, 0x00, 0xF0, 0x00, 0xFB, 0x00, 0x00, 0x03, 0xD0, 0xF0,
    0x00, 0x01, 0x01, 0xF0, 0x01, 0xE1, 0x00, 0x00, 0x06, 0x59, 0xF0, 0x00, 0x01, 0x02, 0xF0, 0x02,
    0xEA, 0x00, 0x00, 0x07, 0x72, 0xF0, 0x00, 0x01, 0x03, 0xF0, 0x03, 0xF7, 0x00, 0x00, 0x06, 0x54,
    0xF0, 0x00, 0x02, 0x00, 0xF0, 0x80, 0xFC, 0x00, 0x00, 0x07, 0xBC, 0xF0, 0x00, 0x02, 0x01, 0xF0,
    0x81, 0xF2, 0x00, 0x00, 0x0E, 0xA7, 0xF0, 0x00, 0x02, 0x02, 0xF0, 0x82, 0xE4, 0x00, 0x00, 0x07,
    0x73, 0xF0, 0x00, 0x02, 0x03, 0xF0, 0x83, 0xE0, 0x00, 0x00, 0x07, 0x96, 0xF0, 0x00, 0xF0, 0x00,
    0x95, 0xBD, 0xD8, 0x90, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x64, 0x20, 0x77, 0x69, 0x74,
};

void TestMPEGTables::atsc_mgt_test1a(void)
{
    PSIPTable si_table(wbal_mgt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 124U);

    // PSIP generic fields
    MasterGuideTable mgt(si_table);
    QCOMPARE (mgt.SectionSyntaxIndicator(),      true);
    QCOMPARE (mgt.PrivateIndicator(),            true);
    QCOMPARE (mgt.SCTEMapId(),                     0U);
    QCOMPARE (mgt.SectionLengthRaw(),            124U);
    QCOMPARE (mgt.TableIDExtension(),              0U);
    QCOMPARE (mgt.Version(),                      19U);
    QCOMPARE (mgt.IsCurrent(),                   true);
    QCOMPARE (mgt.Section(),                       0U);
    QCOMPARE (mgt.LastSection(),                   0U);
    QCOMPARE (mgt.ATSCProtocolVersion(),           0U);

    // MGT specific fields
    QCOMPARE (mgt.TableCount(),                   10U);

    // MGT table data
    QCOMPARE (mgt.TableType(0),                    2U);
    QCOMPARE (mgt.TableClass(0),    TableClass::CVCTc);
    QCOMPARE (mgt.TablePID(0),                0x1FFBU);
    QCOMPARE (mgt.TableVersion(0),                19U);
    QCOMPARE (mgt.TableDescriptorsBytes(0),      450U);
    QCOMPARE (mgt.TableDescriptorsLength(0),       0U);

    QCOMPARE (mgt.TableType(1),               0x0100U);
    QCOMPARE (mgt.TableClass(1),      TableClass::EIT);
    QCOMPARE (mgt.TablePID(1),                0x1000U);
    QCOMPARE (mgt.TableVersion(1),                 9U);
    QCOMPARE (mgt.TableDescriptorsBytes(1),     1656U);
    QCOMPARE (mgt.TableDescriptorsLength(1),       0U);

    QCOMPARE (mgt.TableType(8),               0x0203U);
    QCOMPARE (mgt.TableClass(8),     TableClass::ETTe);
    QCOMPARE (mgt.TablePID(8),                0x1083U);
    QCOMPARE (mgt.TableVersion(8),                 3U);
    QCOMPARE (mgt.TableDescriptorsBytes(8),     1595U);
    QCOMPARE (mgt.TableDescriptorsLength(8),       0U);

    QCOMPARE (mgt.TableType(9),               0x0004U);
    QCOMPARE (mgt.TableClass(9),     TableClass::ETTc);
    QCOMPARE (mgt.TablePID(9),                0x1100U);
    QCOMPARE (mgt.TableVersion(9),                15U);
    QCOMPARE (mgt.TableDescriptorsBytes(9),       56U);
    QCOMPARE (mgt.TableDescriptorsLength(9),       0U);
    // There are no table descriptors, but this is where they would be.
    QCOMPARE (mgt.TableDescriptors(9),       mgt.data() + 121);

    // MGT global descriptors
    QCOMPARE (mgt.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (mgt.GlobalDescriptors(),       mgt.data() + 123);

    // Validate that we reached the end of the table
    const uint8_t *end = mgt.data() + mgt.SectionLength();
    const uint8_t *end2 = mgt.GlobalDescriptors() +
        mgt.GlobalDescriptorsLength() + MasterGuideTable::kMpegCRCSize;
    QCOMPARE(end, end2);
}

void TestMPEGTables::atsc_mgt_test1b(void)
{
    // This is the packet from the test 1a modified to add descriptors
    // to each entry. The CRC on this packet is valid.
    const std::vector<uint8_t> si_data {
        0xC7, 0xF0, 0xB4, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x0A,
        // Table Entries
        0x00, 0x02, 0xFF, 0xFB, 0xF3, 0x00, 0x00, 0x01, 0xC2, 0xF0, 0x02,
              0x80, 0x00,
        0x01, 0x00, 0xF0, 0x00, 0xE9, 0x00, 0x00, 0x06, 0x78, 0xF0, 0x04,
              0x80, 0x02, 0x00, 0x00,
        0x01, 0x01, 0xF0, 0x01, 0xF2, 0x00, 0x00, 0x07, 0xAE, 0xF0, 0x06,
              0x80, 0x04, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x02, 0xF0, 0x02, 0xE8, 0x00, 0x00, 0x06, 0x3B, 0xF0, 0x08,
              0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x03, 0xF0, 0x03, 0xFB, 0x00, 0x00, 0x03, 0xE0, 0xF0, 0x03,
              0x80, 0x01, 0x00,
        0x02, 0x00, 0xF0, 0x80, 0xF4, 0x00, 0x00, 0x09, 0x16, 0xF0, 0x05,
              0x80, 0x03, 0x00, 0x00, 0x00,
        0x02, 0x01, 0xF0, 0x81, 0xE6, 0x00, 0x00, 0x0B, 0x14, 0xF0, 0x07,
              0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x02, 0xF0, 0x82, 0xFD, 0x00, 0x00, 0x03, 0xBE, 0xF0, 0x09,
              0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x03, 0xF0, 0x83, 0xE3, 0x00, 0x00, 0x06, 0x3B, 0xF0, 0x04,
              0x80, 0x02, 0x00, 0x00,
        0x00, 0x04, 0xF1, 0x00, 0xEF, 0x00, 0x00, 0x00, 0x38, 0xF0, 0x04,
              0x80, 0x02, 0x00, 0x00,
        // Global Descriptors
        0xF0, 0x04,
              0x80, 0x02, 0x00, 0x00,
        // CRC
        0x80, 0x4E, 0xF1, 0xF3,
    };
    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 180U);

    // PSIP generic fields
    MasterGuideTable mgt(si_table);
    QCOMPARE (mgt.SectionSyntaxIndicator(),      true);
    QCOMPARE (mgt.PrivateIndicator(),            true);
    QCOMPARE (mgt.SectionLengthRaw(),            180U);
    QCOMPARE (mgt.SCTEMapId(),                     0U);
    QCOMPARE (mgt.TableIDExtension(),              0U);
    QCOMPARE (mgt.Version(),                      19U);
    QCOMPARE (mgt.IsCurrent(),                   true);
    QCOMPARE (mgt.Section(),                       0U);
    QCOMPARE (mgt.LastSection(),                   0U);
    QCOMPARE (mgt.ATSCProtocolVersion(),           0U);

    // MGT specific fields
    QCOMPARE (mgt.TableCount(),                   10U);

    // MGT table data
    QCOMPARE (mgt.TableType(0),                    2U);
    QCOMPARE (mgt.TableClass(0),    TableClass::CVCTc);
    QCOMPARE (mgt.TablePID(0),                0x1FFBU);
    QCOMPARE (mgt.TableVersion(0),                19U);
    QCOMPARE (mgt.TableDescriptorsBytes(0),      450U);
    QCOMPARE (mgt.TableDescriptorsLength(0),       2U);
    QCOMPARE (mgt.TableDescriptors(0),       mgt.data() + 22);

    QCOMPARE (mgt.TableType(1),               0x0100U);
    QCOMPARE (mgt.TableClass(1),      TableClass::EIT);
    QCOMPARE (mgt.TablePID(1),                0x1000U);
    QCOMPARE (mgt.TableVersion(1),                 9U);
    QCOMPARE (mgt.TableDescriptorsBytes(1),     1656U);
    QCOMPARE (mgt.TableDescriptorsLength(1),       4U);
    QCOMPARE (mgt.TableDescriptors(1),       mgt.data() + 35);

    QCOMPARE (mgt.TableDescriptorsLength(2),       6U);
    QCOMPARE (mgt.TableDescriptorsLength(3),       8U);
    QCOMPARE (mgt.TableDescriptorsLength(4),       3U);
    QCOMPARE (mgt.TableDescriptorsLength(5),       5U);
    QCOMPARE (mgt.TableDescriptorsLength(6),       7U);
    QCOMPARE (mgt.TableDescriptorsLength(7),       9U);
    QCOMPARE (mgt.TableDescriptorsLength(8),       4U);

    QCOMPARE (mgt.TableType(9),               0x0004U);
    QCOMPARE (mgt.TableClass(9),     TableClass::ETTc);
    QCOMPARE (mgt.TablePID(9),                0x1100U);
    QCOMPARE (mgt.TableVersion(9),                15U);
    QCOMPARE (mgt.TableDescriptorsBytes(9),       56U);
    QCOMPARE (mgt.TableDescriptorsLength(9),       4U);
    QCOMPARE (mgt.TableDescriptors(9),       mgt.data() + 169);

    // MGT global descriptors
    QCOMPARE (mgt.GlobalDescriptorsLength(),       4U);
    QCOMPARE (mgt.GlobalDescriptors(),       mgt.data() + 175);

    // Validate that we reached the end of the table
    const uint8_t *end = mgt.data() + mgt.SectionLength();
    const uint8_t *end2 = mgt.GlobalDescriptors() +
        mgt.GlobalDescriptorsLength() + MasterGuideTable::kMpegCRCSize;
    QCOMPARE(end, end2);
}

void TestMPEGTables::atsc_mgt_test1c(void)
{
    // This is the packet from the test 1a modified to state there are
    // 14 tables present when there are only 10. The CRC on this
    // packet is valid.
    std::vector<uint8_t> si_data = wbal_mgt_data;
    si_data[10] = 0x0E;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 124U);

    atsc_test_throw<MasterGuideTable>(si_table, PsipParseException::MgtTableCount);
}

void TestMPEGTables::atsc_mgt_test1d(void)
{
    // This is the packet from the test 1a modified so that the second
    // table entry says it has 81 descriptor bytes. The CRC on this
    // packet is valid.
    std::vector<uint8_t> si_data = wbal_mgt_data;
    si_data[43] = 0x51;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 124U);

    atsc_test_throw<MasterGuideTable>(si_table, PsipParseException::MgtTableDescriptors);
}

void TestMPEGTables::atsc_mgt_test1e(void)
{
    // This is the packet from the test 1a modified so that the global
    // desriptor count says it has 100 bytes. The CRC on this packet is
    // valid.
    std::vector<uint8_t> si_data = wbal_mgt_data;
    si_data[122] = 0x4A;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 124U);

    atsc_test_throw<MasterGuideTable>(si_table, PsipParseException::MgtGlobalDescriptors);
}

void TestMPEGTables::atsc_mgt_test2(void)
{
    PSIPTable si_table(wjz_mgt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 740U);

    // PSIP generic fields
    MasterGuideTable mgt(si_table);
    QCOMPARE (mgt.SectionSyntaxIndicator(),      true);
    QCOMPARE (mgt.PrivateIndicator(),            true);
    QCOMPARE (mgt.SectionLengthRaw(),            740U);
    QCOMPARE (mgt.SCTEMapId(),                     0U);
    QCOMPARE (mgt.TableIDExtension(),              0U);
    QCOMPARE (mgt.Version(),                      28U);
    QCOMPARE (mgt.IsCurrent(),                   true);
    QCOMPARE (mgt.Section(),                       0U);
    QCOMPARE (mgt.LastSection(),                   0U);
    QCOMPARE (mgt.ATSCProtocolVersion(),           0U);

    // MGT specific fields
    QCOMPARE (mgt.TableCount(),                   66U);

    // MGT table data
    QCOMPARE (mgt.TableType(0),                    0U);
    QCOMPARE (mgt.TableClass(0),    TableClass::TVCTc);
    QCOMPARE (mgt.TablePID(0),                0x1FFBU);
    QCOMPARE (mgt.TableVersion(0),                 1U);
    QCOMPARE (mgt.TableDescriptorsBytes(0),      252U);
    QCOMPARE (mgt.TableDescriptorsLength(0),       0U);

    QCOMPARE (mgt.TableType(1),                    4U);
    QCOMPARE (mgt.TableClass(1),     TableClass::ETTc);
    QCOMPARE (mgt.TablePID(1),                0x1488U);
    QCOMPARE (mgt.TableVersion(1),                 1U);
    QCOMPARE (mgt.TableDescriptorsBytes(1),       64U);
    QCOMPARE (mgt.TableDescriptorsLength(1),       0U);

    QCOMPARE (mgt.TableType(64),              0x021EU);
    QCOMPARE (mgt.TableClass(64),    TableClass::ETTe);
    QCOMPARE (mgt.TablePID(64),               0x1426U);
    QCOMPARE (mgt.TableVersion(64),                1U);
    QCOMPARE (mgt.TableDescriptorsBytes(64),    1634U);
    QCOMPARE (mgt.TableDescriptorsLength(64),      0U);

    QCOMPARE (mgt.TableType(65),              0x021FU);
    QCOMPARE (mgt.TableClass(65),    TableClass::ETTe);
    QCOMPARE (mgt.TablePID(65),               0x1427U);
    QCOMPARE (mgt.TableVersion(65),                1U);
    QCOMPARE (mgt.TableDescriptorsBytes(65),    1796U);
    QCOMPARE (mgt.TableDescriptorsLength(65),      0U);
    // There are no table descriptors, but this is where they would be.
    QCOMPARE (mgt.TableDescriptors(65),      mgt.data() + 737);

    // MGT global descriptors
    QCOMPARE (mgt.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (mgt.GlobalDescriptors(),       mgt.data() + 739);

    // Validate that we reached the end of the table
    const uint8_t *end = mgt.data() + mgt.SectionLength();
    const uint8_t *end2 = mgt.GlobalDescriptors() +
        mgt.GlobalDescriptorsLength() + MasterGuideTable::kMpegCRCSize;
    QCOMPARE(end, end2);
}

void TestMPEGTables::atsc_mgt_test3(void)
{
    PSIPTable si_table(wbff_mgt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 113U);

    // PSIP generic fields
    MasterGuideTable mgt(si_table);
    QCOMPARE (mgt.SectionSyntaxIndicator(),      true);
    QCOMPARE (mgt.PrivateIndicator(),            true);
    QCOMPARE (mgt.SectionLengthRaw(),            113U);
    QCOMPARE (mgt.SCTEMapId(),                     0U);
    QCOMPARE (mgt.TableIDExtension(),              0U);
    QCOMPARE (mgt.Version(),                      11U);
    QCOMPARE (mgt.IsCurrent(),                   true);
    QCOMPARE (mgt.Section(),                       0U);
    QCOMPARE (mgt.LastSection(),                   0U);
    QCOMPARE (mgt.ATSCProtocolVersion(),           0U);

    // MGT specific fields
    QCOMPARE (mgt.TableCount(),                    9U);

    // MGT table data
    QCOMPARE (mgt.TableType(0),                    2U);
    QCOMPARE (mgt.TableClass(0),    TableClass::CVCTc);
    QCOMPARE (mgt.TablePID(0),                0x1FFBU);
    QCOMPARE (mgt.TableVersion(0),                24U);
    QCOMPARE (mgt.TableDescriptorsBytes(0),      273U);
    QCOMPARE (mgt.TableDescriptorsLength(0),       0U);

    QCOMPARE (mgt.TableType(1),                0x100U);
    QCOMPARE (mgt.TableClass(1),      TableClass::EIT);
    QCOMPARE (mgt.TablePID(1),                0x1000U);
    QCOMPARE (mgt.TableVersion(1),                27U);
    QCOMPARE (mgt.TableDescriptorsBytes(1),      976U);
    QCOMPARE (mgt.TableDescriptorsLength(1),       0U);

    QCOMPARE (mgt.TableType(8),               0x0203U);
    QCOMPARE (mgt.TableClass(8),     TableClass::ETTe);
    QCOMPARE (mgt.TablePID(8),                0x1083U);
    QCOMPARE (mgt.TableVersion(8),                 0U);
    QCOMPARE (mgt.TableDescriptorsBytes(8),     1942U);
    QCOMPARE (mgt.TableDescriptorsLength(8),       0U);
    // There are no table descriptors, but this is where they would be.
    QCOMPARE (mgt.TableDescriptors(8),       mgt.data() + 110);

    // MGT global descriptors
    QCOMPARE (mgt.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (mgt.GlobalDescriptors(),       mgt.data() + 112);

    // Validate that we reached the end of the table
    const uint8_t *end = mgt.data() + mgt.SectionLength();
    const uint8_t *end2 = mgt.GlobalDescriptors() +
        mgt.GlobalDescriptorsLength() + MasterGuideTable::kMpegCRCSize;
    QCOMPARE(end, end2);
}

//
// TerrestrialVirtualChannelTable Tests
//

// Packet broadcast over the air from WRC Washington.
const std::vector<uint8_t> wrc_tvct_data {
    0xC8, 0xF1, 0x94, 0x02, 0x11, 0xC3, 0x00, 0x00, 0x00, 0x06, 0x00, 0x57, 0x00, 0x52, 0x00, 0x43,
    0x00, 0x2D, 0x00, 0x48, 0x00, 0x44, 0x00, 0x20, 0xF0, 0x10, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x11, 0x00, 0x03, 0x0D, 0xC2, 0x00, 0x03, 0xFC, 0x38, 0xA0, 0x1F, 0x01, 0x65, 0x6E, 0x67,
    0x01, 0x00, 0x00, 0x17, 0x57, 0x52, 0x43, 0x2D, 0x48, 0x44, 0x20, 0x4E, 0x42, 0x43, 0x20, 0x34,
    0x20, 0x57, 0x61, 0x73, 0x68, 0x69, 0x6E, 0x67, 0x74, 0x6F, 0x6E, 0xA1, 0x15, 0xE0, 0x31, 0x03,
    0x02, 0xE0, 0x31, 0x00, 0x00, 0x00, 0x81, 0xE0, 0x34, 0x65, 0x6E, 0x67, 0x81, 0xE0, 0x35, 0x73,
    0x70, 0x61, 0x00, 0x43, 0x00, 0x4F, 0x00, 0x5A, 0x00, 0x49, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20,
    0xF0, 0x10, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 0x00, 0x04, 0x0D, 0xC2, 0x00, 0x04,
    0xFC, 0x2C, 0xA0, 0x19, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x11, 0x43, 0x4F, 0x5A, 0x49,
    0x20, 0x54, 0x56, 0x20, 0x6F, 0x6E, 0x20, 0x57, 0x52, 0x43, 0x2D, 0x54, 0x56, 0xA1, 0x0F, 0xE0,
    0x41, 0x02, 0x02, 0xE0, 0x41, 0x00, 0x00, 0x00, 0x81, 0xE0, 0x44, 0x65, 0x6E, 0x67, 0x00, 0x4C,
    0x00, 0x58, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0xF0, 0x10, 0x03, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 0x00, 0x07, 0x0D, 0xC2, 0x00, 0x07, 0xFC, 0x11, 0xA1, 0x0F,
    0xE0, 0x71, 0x02, 0x02, 0xE0, 0x71, 0x00, 0x00, 0x00, 0x81, 0xE0, 0x74, 0x65, 0x6E, 0x67, 0x00,
    0x4F, 0x00, 0x78, 0x00, 0x79, 0x00, 0x67, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x20, 0xF0, 0x10, 0x04,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 0x00, 0x08, 0x4D, 0xC2, 0x00, 0x08, 0xFC, 0x2A, 0xA0,
    0x17, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x0F, 0x4F, 0x78, 0x79, 0x67, 0x65, 0x6E, 0x20,
    0x2D, 0x20, 0x57, 0x52, 0x43, 0x2D, 0x54, 0x56, 0xA1, 0x0F, 0xE0, 0x81, 0x02, 0x02, 0xE0, 0x81,
    0x00, 0x00, 0x00, 0x81, 0xE0, 0x84, 0x65, 0x6E, 0x67, 0x00, 0x57, 0x00, 0x5A, 0x00, 0x44, 0x00,
    0x43, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0xF0, 0xB0, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x11, 0x00, 0x05, 0x0D, 0xC2, 0x00, 0x05, 0xFC, 0x17, 0xA1, 0x15, 0xE0, 0x51, 0x03, 0x02, 0xE0,
    0x51, 0x00, 0x00, 0x00, 0x81, 0xE0, 0x54, 0x65, 0x6E, 0x67, 0x81, 0xE0, 0x55, 0x73, 0x70, 0x61,
    0x00, 0x58, 0x00, 0x49, 0x00, 0x54, 0x00, 0x4F, 0x00, 0x53, 0x00, 0x20, 0x00, 0x20, 0xF0, 0xB0,
    0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 0x00, 0x06, 0x0D, 0xC2, 0x00, 0x06, 0xFC, 0x11,
    0xA1, 0x0F, 0xE0, 0x61, 0x02, 0x02, 0xE0, 0x61, 0x00, 0x00, 0x00, 0x81, 0xE0, 0x64, 0x65, 0x6E,
    0x67, 0xFC, 0x00, 0xB1, 0xDB, 0xD5, 0x8A,
};

void TestMPEGTables::atsc_tvct_test1a(void)
{
    PSIPTable si_table(wrc_tvct_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  404U);

    // PSIP generic fields
    TerrestrialVirtualChannelTable tvct(si_table);
    QCOMPARE (tvct.SectionSyntaxIndicator(),      true);
    QCOMPARE (tvct.PrivateIndicator(),            true);
    QCOMPARE (tvct.SectionLengthRaw(),            404U);
    QCOMPARE (tvct.TransportStreamID(),           529U);
    QCOMPARE (tvct.Version(),                       1U);
    QCOMPARE (tvct.IsCurrent(),                   true);
    QCOMPARE (tvct.Section(),                       0U);
    QCOMPARE (tvct.LastSection(),                   0U);
    QCOMPARE (tvct.ATSCProtocolVersion(),           0U);

    // TVCT specific fields
    QCOMPARE (tvct.ChannelCount(),                  6U);

    // TVCT table data
    QCOMPARE (tvct.ShortChannelName(0),       "WRC-HD");
    QCOMPARE (tvct.MajorChannel(0),                 4U);
    QCOMPARE (tvct.MinorChannel(0),                 1U);
    QCOMPARE (tvct.ModulationMode(0),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(0),   529U);
    QCOMPARE (tvct.ProgramNumber(0),                3U);
    QCOMPARE (tvct.ETMlocation(0),                  0U);
    QCOMPARE (tvct.IsAccessControlled(0),        false);
    QCOMPARE (tvct.IsHidden(0),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(0),           false);
    QCOMPARE (tvct.ServiceType(0),                  2U);
    QCOMPARE (tvct.SourceID(0),                     3U);
    QCOMPARE (tvct.DescriptorsLength(0),           56U);
    QCOMPARE (tvct.GetExtendedChannelName(0), "WRC-HD NBC 4 Washington");

    QCOMPARE (tvct.ShortChannelName(1),         "COZI");
    QCOMPARE (tvct.MajorChannel(1),                 4U);
    QCOMPARE (tvct.MinorChannel(1),                 2U);
    QCOMPARE (tvct.ModulationMode(1),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(1),   529U);
    QCOMPARE (tvct.ProgramNumber(1),                4U);
    QCOMPARE (tvct.ETMlocation(1),                  0U);
    QCOMPARE (tvct.IsAccessControlled(1),        false);
    QCOMPARE (tvct.IsHidden(1),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(1),           false);
    QCOMPARE (tvct.ServiceType(1),                  2U);
    QCOMPARE (tvct.SourceID(1),                     4U);
    QCOMPARE (tvct.DescriptorsLength(1),           44U);
    QCOMPARE (tvct.GetExtendedChannelName(1), "COZI TV on WRC-TV");

    QCOMPARE (tvct.ShortChannelName(2),           "LX");
    QCOMPARE (tvct.MajorChannel(2),                 4U);
    QCOMPARE (tvct.MinorChannel(2),                 3U);
    QCOMPARE (tvct.ModulationMode(2),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(2),   529U);
    QCOMPARE (tvct.ProgramNumber(2),                7U);
    QCOMPARE (tvct.ETMlocation(2),                  0U);
    QCOMPARE (tvct.IsAccessControlled(2),        false);
    QCOMPARE (tvct.IsHidden(2),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(2),           false);
    QCOMPARE (tvct.ServiceType(2),                  2U);
    QCOMPARE (tvct.SourceID(2),                     7U);
    QCOMPARE (tvct.DescriptorsLength(2),           17U);
    QCOMPARE (tvct.GetExtendedChannelName(2),       "");

    QCOMPARE (tvct.ShortChannelName(3),       "Oxygen");
    QCOMPARE (tvct.MajorChannel(3),                 4U);
    QCOMPARE (tvct.MinorChannel(3),                 4U);
    QCOMPARE (tvct.ModulationMode(3),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(3),   529U);
    QCOMPARE (tvct.ProgramNumber(3),                8U);
    QCOMPARE (tvct.ETMlocation(3),                  1U);
    QCOMPARE (tvct.IsAccessControlled(3),        false);
    QCOMPARE (tvct.IsHidden(3),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(3),           false);
    QCOMPARE (tvct.ServiceType(3),                  2U);
    QCOMPARE (tvct.SourceID(3),                     8U);
    QCOMPARE (tvct.DescriptorsLength(3),           42U);
    QCOMPARE (tvct.GetExtendedChannelName(3), "Oxygen - WRC-TV");

    QCOMPARE (tvct.ShortChannelName(4),         "WZDC");
    QCOMPARE (tvct.MajorChannel(4),                44U);
    QCOMPARE (tvct.MinorChannel(4),                 1U);
    QCOMPARE (tvct.ModulationMode(4),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(4),   529U);
    QCOMPARE (tvct.ProgramNumber(4),                5U);
    QCOMPARE (tvct.ETMlocation(4),                  0U);
    QCOMPARE (tvct.IsAccessControlled(4),        false);
    QCOMPARE (tvct.IsHidden(4),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(4),           false);
    QCOMPARE (tvct.ServiceType(4),                  2U);
    QCOMPARE (tvct.SourceID(4),                     5U);
    QCOMPARE (tvct.DescriptorsLength(4),           23U);
    QCOMPARE (tvct.GetExtendedChannelName(4),       "");

    QCOMPARE (tvct.ShortChannelName(5),        "XITOS");
    QCOMPARE (tvct.MajorChannel(5),                44U);
    QCOMPARE (tvct.MinorChannel(5),                 2U);
    QCOMPARE (tvct.ModulationMode(5),               4U);
    QCOMPARE (tvct.ChannelTransportStreamID(5),   529U);
    QCOMPARE (tvct.ProgramNumber(5),                6U);
    QCOMPARE (tvct.ETMlocation(5),                  0U);
    QCOMPARE (tvct.IsAccessControlled(5),        false);
    QCOMPARE (tvct.IsHidden(5),                  false);
    QCOMPARE (tvct.IsHiddenInGuide(5),           false);
    QCOMPARE (tvct.ServiceType(5),                  2U);
    QCOMPARE (tvct.SourceID(5),                     6U);
    QCOMPARE (tvct.DescriptorsLength(5),           17U);
    QCOMPARE (tvct.GetExtendedChannelName(5),       "");

    // TVCT global descriptors
    QCOMPARE (tvct.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (tvct.GlobalDescriptors(), tvct.data() + 403);

    QCOMPARE(tvct.Find(4,1), 0);
    QCOMPARE(tvct.Find(4,4), 3);
    QCOMPARE(tvct.Find(44,2), 5);
    QCOMPARE(tvct.Find(1,1), -1);
}

void TestMPEGTables::atsc_tvct_test1b(void)
{
    // This is the packet from the test 1a modified to state there are
    // 14 tables present when there are only 6. The CRC on this
    // packet is valid.
    std::vector<uint8_t> si_data = wrc_tvct_data;
    si_data[9] = 0x0E;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),             404U);

    atsc_test_throw<TerrestrialVirtualChannelTable>(si_table, PsipParseException::VctChannelCount);
}

void TestMPEGTables::atsc_tvct_test1c(void)
{
    // This is the packet from the test 1a modified so that the second
    // channel entry says it has 256 descriptor bytes. The CRC on this
    // packet is valid.
    std::vector<uint8_t> si_data = wrc_tvct_data;
    si_data[129] = 0xFF;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  404U);

    atsc_test_throw<TerrestrialVirtualChannelTable>(si_table, PsipParseException::VctChannelDescriptors);
}

void TestMPEGTables::atsc_tvct_test1d(void)
{
    // This is the packet from the test 1a modified so that the global
    // desriptor count says it has 100 bytes. The CRC on this packet is
    // valid.
    std::vector<uint8_t> si_data = wrc_tvct_data;
    si_data[402] = 0x4A;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  404U);

    atsc_test_throw<TerrestrialVirtualChannelTable>(si_table, PsipParseException::VctGlobalDescriptors);
}

//
// CableVirtualChannelTable Tests
//

// This CVCT table from WBAL was dumped from a captured mpts stream using
// the tsduck tools.
const std::vector<uint8_t> wbal_cvct_data {
    0xC9, 0xF1, 0x45, 0x00, 0x01, 0xEF, 0x00, 0x00, 0x00, 0x06, 0x00, 0x57, 0x00, 0x4D, 0x00, 0x41,
    0x00, 0x52, 0x00, 0x2D, 0x00, 0x48, 0x00, 0x44, 0xF0, 0x08, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x04, 0x02, 0x41, 0xC2, 0x00, 0x01, 0xFC, 0x17, 0xA1, 0x15, 0xF3, 0xC5, 0x03, 0x02,
    0xF3, 0xC5, 0x00, 0x00, 0x00, 0x81, 0xF3, 0xC6, 0x65, 0x6E, 0x67, 0x81, 0xF3, 0xC7, 0x73, 0x70,
    0x61, 0x00, 0x57, 0x00, 0x42, 0x00, 0x41, 0x00, 0x4C, 0x00, 0x2D, 0x00, 0x48, 0x00, 0x44, 0xF0,
    0x2C, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x41, 0xC2, 0x04, 0x04, 0xFC,
    0x17, 0xA1, 0x15, 0xF3, 0x89, 0x03, 0x02, 0xF3, 0x89, 0x00, 0x00, 0x00, 0x81, 0xF3, 0x8A, 0x65,
    0x6E, 0x67, 0x81, 0xF3, 0x8B, 0x73, 0x70, 0x61, 0x00, 0x57, 0x00, 0x4D, 0x00, 0x41, 0x00, 0x52,
    0x00, 0x20, 0x00, 0x4C, 0x00, 0x41, 0xF0, 0x08, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x18, 0x01, 0xC2, 0x00, 0x02, 0xFC, 0x11, 0xA1, 0x0F, 0xF3, 0xBB, 0x02, 0x02, 0xF3, 0xBB,
    0x00, 0x00, 0x00, 0x81, 0xF3, 0xBC, 0x65, 0x6E, 0x67, 0x00, 0x57, 0x00, 0x4D, 0x00, 0x41, 0x00,
    0x52, 0x00, 0x2D, 0x00, 0x42, 0x00, 0x4F, 0xF0, 0x08, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x1D, 0x01, 0xC2, 0x00, 0x03, 0xFC, 0x11, 0xA1, 0x0F, 0xED, 0xAD, 0x02, 0x02, 0xED,
    0xAD, 0x00, 0x00, 0x00, 0x81, 0xEB, 0xEC, 0x65, 0x6E, 0x67, 0x00, 0x57, 0x00, 0x4D, 0x00, 0x41,
    0x00, 0x52, 0x00, 0x5F, 0x00, 0x43, 0x00, 0x6F, 0xF0, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x0D, 0x1D, 0x01, 0xC2, 0x00, 0x04, 0xFC, 0x11, 0xA1, 0x0F, 0xED, 0xF3, 0x02, 0x02,
    0xED, 0xF3, 0x00, 0x00, 0x00, 0x81, 0xED, 0xF4, 0x65, 0x6E, 0x67, 0x00, 0x57, 0x00, 0x42, 0x00,
    0x41, 0x00, 0x4C, 0x00, 0x20, 0x00, 0x4D, 0x00, 0x65, 0xF0, 0x2C, 0x02, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x12, 0xF2, 0x41, 0xC2, 0x12, 0xF2, 0xFC, 0x17, 0xA1, 0x15, 0xF3, 0x93, 0x03,
    0x02, 0xF3, 0x93, 0x00, 0x00, 0x00, 0x81, 0xF3, 0x94, 0x65, 0x6E, 0x67, 0x81, 0xF3, 0x95, 0x73,
    0x70, 0x61, 0xFC, 0x00, 0x6D, 0x84, 0x89, 0xFD,
};

// This CVCT table from WBAL was dumped from the VirtualChannelTable::Parse
// function.  Starting at pesdata(), TSSizeInBuffer() bytes were dumped.
// Its unclear what the bytes are after the table data.  The 0xFF bytes
// look like stuffing and the 0x00 bytes look like ???, but the TSDuck
// tools complain about all those bytes when converting this data stream
// into a textual version of the table.
const std::vector<uint8_t> wbal_cvct_data2 {
    0xC9, 0xF1, 0xBF, 0x00, 0x01, 0xFD, 0x00, 0x00, 0x00, 0x08, 0x00, 0x57, 0x00, 0x46, 0x00, 0x44,
    0x00, 0x43, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x6E, 0xF0, 0x38, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x03, 0xF7, 0x01, 0xC2, 0x00, 0x06, 0xFC, 0x17, 0xA1, 0x15, 0xEC, 0xA9, 0x03, 0x81,
    0xEC, 0xAA, 0x00, 0x00, 0x00, 0x03, 0xEC, 0xAB, 0x00, 0x00, 0x00, 0x02, 0xEC, 0xA9, 0x00, 0x00,
    0x00, 0x00, 0x57, 0x00, 0x46, 0x00, 0x44, 0x00, 0x43, 0x00, 0x2D, 0x00, 0x55, 0x00, 0x6E, 0xF0,
    0x04, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xF8, 0x01, 0xC2, 0x00, 0x04, 0xFC,
    0x17, 0xA1, 0x15, 0xEC, 0xF9, 0x03, 0x02, 0xEC, 0xF9, 0x00, 0x00, 0x00, 0x81, 0xEC, 0xFA, 0x00,
    0x00, 0x00, 0x03, 0xEC, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x57, 0x00, 0x4D, 0x00, 0x41, 0x00, 0x52,
    0x00, 0x2D, 0x00, 0x53, 0x00, 0x44, 0xF0, 0x08, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x01, 0x41, 0xC2, 0x04, 0x01, 0xFC, 0x17, 0xA1, 0x15, 0xF3, 0xB1, 0x03, 0x02, 0xF3, 0xB1,
    0x00, 0x00, 0x00, 0x81, 0xF3, 0xB2, 0x65, 0x6E, 0x67, 0x81, 0xF3, 0xB3, 0x73, 0x70, 0x61, 0x00,
    0x57, 0x00, 0x42, 0x00, 0x41, 0x00, 0x4C, 0x00, 0x2D, 0x00, 0x53, 0x00, 0x44, 0xF0, 0x2C, 0x01,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x03, 0x41, 0xC2, 0x04, 0x03, 0xFC, 0x17, 0xA1,
    0x15, 0xF3, 0x9D, 0x03, 0x02, 0xF3, 0x9D, 0x00, 0x00, 0x00, 0x81, 0xF3, 0x9E, 0x65, 0x6E, 0x67,
    0x81, 0xF3, 0x9F, 0x73, 0x70, 0x61, 0x00, 0x57, 0x00, 0x4A, 0x00, 0x5A, 0x00, 0x2D, 0x00, 0x53,
    0x00, 0x44, 0x00, 0x00, 0xF0, 0x34, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x05,
    0x01, 0xC2, 0x04, 0x05, 0xFC, 0x17, 0xA1, 0x15, 0xEC, 0xDB, 0x03, 0x02, 0xEC, 0xDB, 0x00, 0x00,
    0x00, 0x81, 0xEC, 0xDC, 0x65, 0x6E, 0x67, 0x81, 0xEC, 0xDD, 0x73, 0x70, 0x61, 0x00, 0x57, 0x00,
    0x4E, 0x00, 0x55, 0x00, 0x56, 0x00, 0x2D, 0x00, 0x53, 0x00, 0x44, 0xF0, 0xD8, 0x01, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x07, 0x01, 0xC2, 0x00, 0x03, 0xFC, 0x17, 0xA1, 0x15, 0xED,
    0x2B, 0x03, 0x02, 0xED, 0x2B, 0x00, 0x00, 0x00, 0x81, 0xED, 0x2C, 0x65, 0x6E, 0x67, 0x81, 0xE0,
    0x35, 0x73, 0x70, 0x61, 0x00, 0x57, 0x00, 0x42, 0x00, 0x46, 0x00, 0x46, 0x00, 0x2D, 0x00, 0x4D,
    0x00, 0x79, 0xF0, 0xB4, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x08, 0x01, 0xC2,
    0x00, 0x02, 0xFC, 0x11, 0xA1, 0x0F, 0xED, 0x8F, 0x02, 0x02, 0xED, 0x8F, 0x00, 0x00, 0x00, 0x81,
    0xED, 0x90, 0x65, 0x6E, 0x67, 0x00, 0x57, 0x00, 0x42, 0x00, 0x46, 0x00, 0x46, 0x00, 0x2D, 0x00,
    0x53, 0x00, 0x44, 0xF0, 0xB4, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x09, 0x01,
    0xC2, 0x00, 0x01, 0xFC, 0x17, 0xA1, 0x15, 0xEC, 0x45, 0x03, 0x02, 0xEC, 0x45, 0x00, 0x00, 0x00,
    0x81, 0xEC, 0x46, 0x65, 0x6E, 0x67, 0x81, 0xEC, 0x47, 0x65, 0x6E, 0x67, 0xFC, 0x00, 0x3A, 0xBB,
    0x6E, 0x63, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void TestMPEGTables::atsc_cvct_test1(void)
{
    PSIPTable si_table(wbal_cvct_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  325U);

    // PSIP generic fields
    CableVirtualChannelTable cvct(si_table);
    QCOMPARE (cvct.SectionSyntaxIndicator(),      true);
    QCOMPARE (cvct.PrivateIndicator(),            true);
    QCOMPARE (cvct.SectionLengthRaw(),            325U);
    QCOMPARE (cvct.TransportStreamID(),             1U);
    QCOMPARE (cvct.Version(),                      23U);
    QCOMPARE (cvct.IsCurrent(),                   true);
    QCOMPARE (cvct.Section(),                       0U);
    QCOMPARE (cvct.LastSection(),                   0U);
    QCOMPARE (cvct.ATSCProtocolVersion(),           0U);

    // CVCT specific fields
    QCOMPARE (cvct.ChannelCount(),                  6U);

    // CVCT table data
    QCOMPARE (cvct.ShortChannelName(0),      "WMAR-HD");
    QCOMPARE (cvct.MajorChannel(0),                 2U);
    QCOMPARE (cvct.MinorChannel(0),                 1U);
    QCOMPARE (cvct.ModulationMode(0),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(0),     1U);
    QCOMPARE (cvct.ProgramNumber(0),             1026U);
    QCOMPARE (cvct.ETMlocation(0),                  1U);
    QCOMPARE (cvct.IsAccessControlled(0),        false);
    QCOMPARE (cvct.IsHidden(0),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(0),           false);
    QCOMPARE (cvct.ServiceType(0),                  2U);
    QCOMPARE (cvct.SourceID(0),                     1U);
    QCOMPARE (cvct.DescriptorsLength(0),           23U);

    QCOMPARE (cvct.ShortChannelName(1),      "WBAL-HD");
    QCOMPARE (cvct.MajorChannel(1),                11U);
    QCOMPARE (cvct.MinorChannel(1),                 1U);
    QCOMPARE (cvct.ModulationMode(1),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(1),     1U);
    QCOMPARE (cvct.ProgramNumber(1),             1028U);
    QCOMPARE (cvct.ETMlocation(1),                  1U);
    QCOMPARE (cvct.IsAccessControlled(1),        false);
    QCOMPARE (cvct.IsHidden(1),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(1),           false);
    QCOMPARE (cvct.ServiceType(1),                  2U);
    QCOMPARE (cvct.SourceID(1),                  1028U);
    QCOMPARE (cvct.DescriptorsLength(1),           23U);

    QCOMPARE (cvct.ShortChannelName(2),      "WMAR LA");
    QCOMPARE (cvct.MajorChannel(2),                 2U);
    QCOMPARE (cvct.MinorChannel(2),                 2U);
    QCOMPARE (cvct.ModulationMode(2),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(2),     1U);
    QCOMPARE (cvct.ProgramNumber(2),             1048U);
    QCOMPARE (cvct.ETMlocation(2),                  0U);
    QCOMPARE (cvct.IsAccessControlled(2),        false);
    QCOMPARE (cvct.IsHidden(2),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(2),           false);
    QCOMPARE (cvct.ServiceType(2),                  2U);
    QCOMPARE (cvct.SourceID(2),                     2U);
    QCOMPARE (cvct.DescriptorsLength(2),           17U);

    QCOMPARE (cvct.ShortChannelName(3),      "WMAR-BO");
    QCOMPARE (cvct.MajorChannel(3),                 2U);
    QCOMPARE (cvct.MinorChannel(3),                 3U);
    QCOMPARE (cvct.ModulationMode(3),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(3),     1U);
    QCOMPARE (cvct.ProgramNumber(3),             1053U);
    QCOMPARE (cvct.ETMlocation(3),                  0U);
    QCOMPARE (cvct.IsAccessControlled(3),        false);
    QCOMPARE (cvct.IsHidden(3),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(3),           false);
    QCOMPARE (cvct.ServiceType(3),                  2U);
    QCOMPARE (cvct.SourceID(3),                     3U);
    QCOMPARE (cvct.DescriptorsLength(3),           17U);

    QCOMPARE (cvct.ShortChannelName(4),      "WMAR_Co");
    QCOMPARE (cvct.MajorChannel(4),                 2U);
    QCOMPARE (cvct.MinorChannel(4),                 4U);
    QCOMPARE (cvct.ModulationMode(4),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(4),     1U);
    QCOMPARE (cvct.ProgramNumber(4),             3357U);
    QCOMPARE (cvct.ETMlocation(4),                  0U);
    QCOMPARE (cvct.IsAccessControlled(4),        false);
    QCOMPARE (cvct.IsHidden(4),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(4),           false);
    QCOMPARE (cvct.ServiceType(4),                  2U);
    QCOMPARE (cvct.SourceID(4),                     4U);
    QCOMPARE (cvct.DescriptorsLength(4),           17U);

    QCOMPARE (cvct.ShortChannelName(5),      "WBAL Me");
    QCOMPARE (cvct.MajorChannel(5),                11U);
    QCOMPARE (cvct.MinorChannel(5),                 2U);
    QCOMPARE (cvct.ModulationMode(5),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(5),     1U);
    QCOMPARE (cvct.ProgramNumber(5),             4850U);
    QCOMPARE (cvct.ETMlocation(5),                  1U);
    QCOMPARE (cvct.IsAccessControlled(5),        false);
    QCOMPARE (cvct.IsHidden(5),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(5),           false);
    QCOMPARE (cvct.ServiceType(5),                  2U);
    QCOMPARE (cvct.SourceID(5),                  4850U);
    QCOMPARE (cvct.DescriptorsLength(5),           23U);

    // CVCT global descriptors
    QCOMPARE (cvct.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (cvct.GlobalDescriptors(), cvct.data() + 324);

    QCOMPARE(cvct.Find(2,1), 0);
    QCOMPARE(cvct.Find(11,2), 5);
    QCOMPARE(cvct.Find(11,3), -1);
}

void TestMPEGTables::atsc_cvct_test2(void)
{
    PSIPTable si_table(wbal_cvct_data2);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  447U);

    // PSIP generic fields
    CableVirtualChannelTable cvct(si_table);
    QCOMPARE (cvct.SectionSyntaxIndicator(),      true);
    QCOMPARE (cvct.PrivateIndicator(),            true);
    QCOMPARE (cvct.SectionLengthRaw(),            447U);
    QCOMPARE (cvct.TransportStreamID(),             1U);
    QCOMPARE (cvct.Version(),                      30U);
    QCOMPARE (cvct.IsCurrent(),                   true);
    QCOMPARE (cvct.Section(),                       0U);
    QCOMPARE (cvct.LastSection(),                   0U);
    QCOMPARE (cvct.ATSCProtocolVersion(),           0U);

    // CVCT specific fields
    QCOMPARE (cvct.ChannelCount(),                  8U);

    // CVCT table data
    QCOMPARE (cvct.ShortChannelName(0),      "WFDC_Un");
    QCOMPARE (cvct.MajorChannel(0),                14U);
    QCOMPARE (cvct.MinorChannel(0),                 1U);
    QCOMPARE (cvct.ModulationMode(0),               3U);
    QCOMPARE (cvct.ChannelTransportStreamID(0),     1U);
    QCOMPARE (cvct.ProgramNumber(0),             1015U);
    QCOMPARE (cvct.ETMlocation(0),                  0U);
    QCOMPARE (cvct.IsAccessControlled(0),        false);
    QCOMPARE (cvct.IsHidden(0),                  false);
    QCOMPARE (cvct.IsHiddenInGuide(0),           false);
    QCOMPARE (cvct.ServiceType(0),                  2U);
    QCOMPARE (cvct.SourceID(0),                     6U);
    QCOMPARE (cvct.DescriptorsLength(0),           23U);

    QCOMPARE (cvct.ShortChannelName(1),      "WFDC-Un");
    QCOMPARE (cvct.ShortChannelName(2),      "WMAR-SD");
    QCOMPARE (cvct.ShortChannelName(3),      "WBAL-SD");
    QCOMPARE (cvct.ShortChannelName(4),      "WJZ-SD");
    QCOMPARE (cvct.ShortChannelName(5),      "WNUV-SD");
    QCOMPARE (cvct.ShortChannelName(6),      "WBFF-My");
    QCOMPARE (cvct.ShortChannelName(7),      "WBFF-SD");

    // CVCT global descriptors
    QCOMPARE (cvct.GlobalDescriptorsLength(),       0U);
    // No global descriptors, but this is where they would be.
    QCOMPARE (cvct.GlobalDescriptors(), cvct.data() + 446);
}

//
// EventInformationTable Tests
//

// Packet broadcast over cable from WBAL Baltimore.
const std::vector<uint8_t> wbal_eit_data {
    0xCB, 0xF1, 0xFB, 0x04, 0x03, 0xDB, 0x00, 0x00, 0x00, 0x06, 0xC0, 0x03, 0x50, 0x00, 0x7E, 0x52,
    0xD0, 0x0E, 0x10, 0x20, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x18, 0x54, 0x68, 0x65, 0x20,
    0x45, 0x6C, 0x6C, 0x65, 0x6E, 0x20, 0x44, 0x65, 0x47, 0x65, 0x6E, 0x65, 0x72, 0x65, 0x73, 0x20,
    0x53, 0x68, 0x6F, 0x77, 0xF0, 0x36, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF, 0x87,
    0x13, 0xC1, 0x01, 0x01, 0x00, 0xF3, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54,
    0x56, 0x2D, 0x50, 0x47, 0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01, 0xBF, 0x65, 0x6E, 0x67,
    0x81, 0x0A, 0x08, 0x28, 0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61, 0xC0, 0x04, 0x50, 0x00,
    0x8C, 0x62, 0xD0, 0x0E, 0x10, 0x16, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x0E, 0x31, 0x31,
    0x20, 0x4E, 0x65, 0x77, 0x73, 0x20, 0x61, 0x74, 0x20, 0x35, 0x70, 0x6D, 0xF0, 0x21, 0x86, 0x07,
    0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF, 0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01, 0xBF,
    0x65, 0x6E, 0x67, 0x81, 0x0A, 0x08, 0x28, 0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61, 0xC0,
    0x05, 0x50, 0x00, 0x9A, 0x72, 0xD0, 0x07, 0x08, 0x16, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00,
    0x0E, 0x31, 0x31, 0x20, 0x4E, 0x65, 0x77, 0x73, 0x20, 0x61, 0x74, 0x20, 0x36, 0x70, 0x6D, 0xF0,
    0x21, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF, 0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF,
    0x0F, 0x01, 0xBF, 0x65, 0x6E, 0x67, 0x81, 0x0A, 0x08, 0x28, 0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73,
    0x70, 0x61, 0xC0, 0x06, 0x50, 0x00, 0xA1, 0x7A, 0xC0, 0x07, 0x08, 0x29, 0x01, 0x65, 0x6E, 0x67,
    0x01, 0x00, 0x00, 0x21, 0x4E, 0x42, 0x43, 0x20, 0x4E, 0x69, 0x67, 0x68, 0x74, 0x6C, 0x79, 0x20,
    0x4E, 0x65, 0x77, 0x73, 0x20, 0x57, 0x69, 0x74, 0x68, 0x20, 0x4C, 0x65, 0x73, 0x74, 0x65, 0x72,
    0x20, 0x48, 0x6F, 0x6C, 0x74, 0xF0, 0x21, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF,
    0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01, 0xBF, 0x65, 0x6E, 0x67, 0x81, 0x0A, 0x08, 0x28,
    0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61, 0xC0, 0x07, 0x50, 0x00, 0xA8, 0x82, 0xD0, 0x07,
    0x08, 0x16, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x0E, 0x49, 0x6E, 0x73, 0x69, 0x64, 0x65,
    0x20, 0x45, 0x64, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0xF0, 0x36, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67,
    0xC1, 0x3F, 0xFF, 0x87, 0x13, 0xC1, 0x01, 0x01, 0x00, 0xF3, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01,
    0x00, 0x00, 0x05, 0x54, 0x56, 0x2D, 0x50, 0x47, 0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01,
    0xBF, 0x65, 0x6E, 0x67, 0x81, 0x0A, 0x08, 0x28, 0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61,
    0xC0, 0x08, 0x50, 0x00, 0xAF, 0x8A, 0xD0, 0x07, 0x08, 0x18, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00,
    0x00, 0x10, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x20, 0x48, 0x6F, 0x6C, 0x6C, 0x79, 0x77, 0x6F,
    0x6F, 0x64, 0xF0, 0x36, 0x86, 0x07, 0xE1, 0x65, 0x6E, 0x67, 0xC1, 0x3F, 0xFF, 0x87, 0x13, 0xC1,
    0x01, 0x01, 0x00, 0xF3, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54, 0x56, 0x2D,
    0x50, 0x47, 0x81, 0x0A, 0x08, 0x3A, 0x0F, 0xFF, 0x0F, 0x01, 0xBF, 0x65, 0x6E, 0x67, 0x81, 0x0A,
    0x08, 0x28, 0x45, 0xFF, 0x00, 0x01, 0xBF, 0x73, 0x70, 0x61, 0xF1, 0x60, 0x27, 0xA4,
};

void TestMPEGTables::atsc_eit_test1a(void)
{
    PSIPTable si_table(wbal_eit_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  507U);

    // PSIP generic fields
    EventInformationTable eit(si_table);
    QCOMPARE (eit.SectionSyntaxIndicator(),       true);
    QCOMPARE (eit.PrivateIndicator(),             true);
    QCOMPARE (eit.SectionLengthRaw(),             507U);
    QCOMPARE (eit.SourceID(),                    1027U);
    QCOMPARE (eit.Version(),                       13U);
    QCOMPARE (eit.IsCurrent(),                    true);
    QCOMPARE (eit.Section(),                        0U);
    QCOMPARE (eit.LastSection(),                    0U);
    QCOMPARE (eit.ATSCProtocolVersion(),            0U);

    // EIT specific fields
    QCOMPARE (eit.EventCount(),                     6U);

    // EIT table data
    QDateTime expectedDT {QDate(2022,07,18), QTime(20,00,18), Qt::UTC};
    QCOMPARE (eit.EventID(0),                       3U);
    QCOMPARE (eit.StartTimeRaw(0),         1342209618U);
    QCOMPARE (eit.StartTimeGPS(0),          expectedDT);
    QCOMPARE (eit.ETMLocation(0),                   1U);
    QCOMPARE (eit.LengthInSeconds(0),            3600U);
    QCOMPARE (eit.TitleLength(0),                  32U);
    {
        MultipleStringStructure mss { eit.title(0) };
        QCOMPARE (mss.StringCount(),                   1U);
        QCOMPARE (mss.LanguageKey(0),            0x656E67);
        QCOMPARE (mss.LanguageString(0),            "eng");
        QCOMPARE (mss.GetFullString(0), "The Ellen DeGeneres Show");
    }
    QCOMPARE (eit.DescriptorsLength(0),            54U);

    expectedDT = QDateTime(QDate(2022,07,18), QTime(21,00,18), Qt::UTC);
    QCOMPARE (eit.EventID(1),                       4U);
    QCOMPARE (eit.StartTimeRaw(1),         1342213218U);
    QCOMPARE (eit.StartTimeGPS(1),          expectedDT);
    QCOMPARE (eit.ETMLocation(1),                   1U);
    QCOMPARE (eit.LengthInSeconds(1),            3600U);
    QCOMPARE (eit.TitleLength(1),                  22U);
    {
        MultipleStringStructure mss { eit.title(1) };
        QCOMPARE (mss.StringCount(),                   1U);
        QCOMPARE (mss.LanguageKey(0),            0x656E67);
        QCOMPARE (mss.LanguageString(0),            "eng");
        QCOMPARE (mss.GetFullString(0),  "11 News at 5pm");
    }
    QCOMPARE (eit.DescriptorsLength(1),            33U);

    expectedDT = QDateTime(QDate(2022,07,18), QTime(22,00,18), Qt::UTC);
    QCOMPARE (eit.EventID(2),                       5U);
    QCOMPARE (eit.StartTimeRaw(2),         1342216818U);
    QCOMPARE (eit.StartTimeGPS(2),          expectedDT);
    QCOMPARE (eit.ETMLocation(2),                   1U);
    QCOMPARE (eit.LengthInSeconds(2),            1800U);
    QCOMPARE (eit.TitleLength(2),                  22U);
    {
        MultipleStringStructure mss { eit.title(2) };
        QCOMPARE (mss.StringCount(),                   1U);
        QCOMPARE (mss.LanguageKey(0),            0x656E67);
        QCOMPARE (mss.LanguageString(0),            "eng");
        QCOMPARE (mss.GetFullString(0),  "11 News at 6pm");
    }
    QCOMPARE (eit.DescriptorsLength(2),            33U);
}

void TestMPEGTables::atsc_eit_test1b(void)
{
    // This is the packet from the test 1a modified to state there are
    // 100 events present when there are only 6. The CRC on this packet
    // is valid.
    std::vector<uint8_t> si_data = wbal_eit_data;
    si_data[9] = 0x64;
    update_crc(si_data);

    PSIPTable si_table(si_data);
//    std::cerr << qPrintable(QString("%1").arg(si_table.CalcCRC(),8,16,QChar('0'))) << std::endl;
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 507U);

    atsc_test_throw<EventInformationTable>(si_table, PsipParseException::EitEventCount);
}

void TestMPEGTables::atsc_eit_test1c(void)
{
    // This is the packet from the test 1a modified so that the second
    // channel entry says it has 511 descriptor bytes. The CRC on this
    // packet is valid.
    std::vector<uint8_t> si_data = wbal_eit_data;
    si_data[140] = 0xF1;
    si_data[141] = 0x55;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 507U);

    atsc_test_throw<EventInformationTable>(si_table, PsipParseException::EitEventDescriptors);
}

//
// ExtendedTextTable Tests
//

// Packet broadcast over cable from WBAL Baltimore.
const std::vector<uint8_t> wbal_ett_data {
    0xCC, 0xF0, 0x2F, 0x06, 0xB3, 0xD7, 0x00, 0x00, 0x00, 0x04, 0x09, 0x00, 0x16, 0x01, 0x65, 0x6E,
    0x67, 0x01, 0x00, 0x00, 0x19, 0x4C, 0x6F, 0x63, 0x61, 0x6C, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x72,
    0x65, 0x67, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x20, 0x6E, 0x65, 0x77, 0x73, 0x2E, 0x20, 0x79, 0xA0,
    0x15, 0xFD,
};

void TestMPEGTables::atsc_ett_test(void)
{
    PSIPTable si_table(wbal_ett_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                   47U);

    // PSIP generic fields
    ExtendedTextTable eit(si_table);
    QCOMPARE (eit.SectionSyntaxIndicator(),       true);
    QCOMPARE (eit.PrivateIndicator(),             true);
    QCOMPARE (eit.SectionLengthRaw(),              47U);
    QCOMPARE (eit.ExtendedTextTableID(),       0x06B3U);
    QCOMPARE (eit.Version(),                       11U);
    QCOMPARE (eit.IsCurrent(),                    true);
    QCOMPARE (eit.Section(),                        0U);
    QCOMPARE (eit.LastSection(),                    0U);
    QCOMPARE (eit.ATSCProtocolVersion(),            0U);

    // ETT specific fields
    QCOMPARE (eit.IsChannelETM(),                false);
    QCOMPARE (eit.IsEventETM(),                   true);
    QCOMPARE (eit.SourceID(),                    1033U);
    QCOMPARE (eit.EventID(),                        5U);

    MultipleStringStructure mss { eit.ExtendedTextMessage() };
    QCOMPARE (mss.StringCount(),                   1U);
    QCOMPARE (mss.LanguageKey(0),            0x656E67);
    QCOMPARE (mss.LanguageString(0),            "eng");
    QCOMPARE (mss.GetFullString(0), "Local and regional news.");
}

//
// RatingRegionTable Tests
//

void TestMPEGTables::atsc_rrt_test(void)
{
    const std::vector<uint8_t> si_data {
        0xCA, 0xF3, 0xD0, 0xFF, 0x01, 0xC1, 0x00, 0x00, 0x00, 0x26, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00,
        0x00, 0x1E, 0x55, 0x2E, 0x53, 0x2E, 0x20, 0x28, 0x35, 0x30, 0x20, 0x73, 0x74, 0x61, 0x74, 0x65,
        0x73, 0x20, 0x2B, 0x20, 0x70, 0x6F, 0x73, 0x73, 0x65, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x73, 0x29,
        0x08, 0x17, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x0F, 0x45, 0x6E, 0x74, 0x69, 0x72, 0x65,
        0x20, 0x41, 0x75, 0x64, 0x69, 0x65, 0x6E, 0x63, 0x65, 0xF6, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00,
        0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x0C, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x4E,
        0x6F, 0x6E, 0x65, 0x0C, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x4E, 0x6F, 0x6E, 0x65,
        0x0C, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x54, 0x56, 0x2D, 0x47, 0x0C, 0x01, 0x65,
        0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x54, 0x56, 0x2D, 0x47, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01,
        0x00, 0x00, 0x05, 0x54, 0x56, 0x2D, 0x50, 0x47, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00,
        0x05, 0x54, 0x56, 0x2D, 0x50, 0x47, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54,
        0x56, 0x2D, 0x31, 0x34, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54, 0x56, 0x2D,
        0x31, 0x34, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54, 0x56, 0x2D, 0x4D, 0x41,
        0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x54, 0x56, 0x2D, 0x4D, 0x41, 0x10, 0x01,
        0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x08, 0x44, 0x69, 0x61, 0x6C, 0x6F, 0x67, 0x75, 0x65, 0xE2,
        0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x09, 0x01, 0x65, 0x6E,
        0x67, 0x01, 0x00, 0x00, 0x01, 0x44, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x01, 0x44,
        0x10, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x08, 0x4C, 0x61, 0x6E, 0x67, 0x75, 0x61, 0x67,
        0x65, 0xE2, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x09, 0x01,
        0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x01, 0x4C, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00,
        0x01, 0x4C, 0x0B, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x03, 0x53, 0x65, 0x78, 0xE2, 0x05,
        0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x09, 0x01, 0x65, 0x6E, 0x67,
        0x01, 0x00, 0x00, 0x01, 0x53, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x01, 0x53, 0x10,
        0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x08, 0x56, 0x69, 0x6F, 0x6C, 0x65, 0x6E, 0x63, 0x65,
        0xE2, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x09, 0x01, 0x65,
        0x6E, 0x67, 0x01, 0x00, 0x00, 0x01, 0x56, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x01,
        0x56, 0x10, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x08, 0x43, 0x68, 0x69, 0x6C, 0x64, 0x72,
        0x65, 0x6E, 0xF3, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x0C,
        0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x54, 0x56, 0x2D, 0x59, 0x0C, 0x01, 0x65, 0x6E,
        0x67, 0x01, 0x00, 0x00, 0x04, 0x54, 0x56, 0x2D, 0x59, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00,
        0x00, 0x05, 0x54, 0x56, 0x2D, 0x59, 0x37, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05,
        0x54, 0x56, 0x2D, 0x59, 0x37, 0x18, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x10, 0x46, 0x61,
        0x6E, 0x74, 0x61, 0x73, 0x79, 0x20, 0x56, 0x69, 0x6F, 0x6C, 0x65, 0x6E, 0x63, 0x65, 0xE2, 0x05,
        0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x0A, 0x01, 0x65, 0x6E, 0x67,
        0x01, 0x00, 0x00, 0x02, 0x46, 0x56, 0x0A, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x02, 0x46,
        0x56, 0x0C, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x04, 0x4D, 0x50, 0x41, 0x41, 0xE9, 0x05,
        0x01, 0x65, 0x6E, 0x67, 0x00, 0x05, 0x01, 0x65, 0x6E, 0x67, 0x00, 0x0B, 0x01, 0x65, 0x6E, 0x67,
        0x01, 0x00, 0x00, 0x03, 0x4E, 0x2F, 0x41, 0x22, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x1A,
        0x4D, 0x50, 0x41, 0x41, 0x20, 0x52, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x4E, 0x6F, 0x74, 0x20,
        0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x62, 0x6C, 0x65, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01,
        0x00, 0x00, 0x01, 0x47, 0x1D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x15, 0x53, 0x75, 0x69,
        0x74, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x41, 0x6C, 0x6C, 0x20, 0x41, 0x67,
        0x65, 0x73, 0x0A, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x02, 0x50, 0x47, 0x23, 0x01, 0x65,
        0x6E, 0x67, 0x01, 0x00, 0x00, 0x1B, 0x50, 0x61, 0x72, 0x65, 0x6E, 0x74, 0x61, 0x6C, 0x20, 0x47,
        0x75, 0x69, 0x64, 0x61, 0x6E, 0x63, 0x65, 0x20, 0x53, 0x75, 0x67, 0x67, 0x65, 0x73, 0x74, 0x65,
        0x64, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x50, 0x47, 0x2D, 0x31, 0x33, 0x22,
        0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x1A, 0x50, 0x61, 0x72, 0x65, 0x6E, 0x74, 0x73, 0x20,
        0x53, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x6C, 0x79, 0x20, 0x43, 0x61, 0x75, 0x74, 0x69, 0x6F, 0x6E,
        0x65, 0x64, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x01, 0x52, 0x39, 0x01, 0x65, 0x6E,
        0x67, 0x01, 0x00, 0x00, 0x31, 0x52, 0x65, 0x73, 0x74, 0x72, 0x69, 0x63, 0x74, 0x65, 0x64, 0x2C,
        0x20, 0x75, 0x6E, 0x64, 0x65, 0x72, 0x20, 0x31, 0x37, 0x20, 0x6D, 0x75, 0x73, 0x74, 0x20, 0x62,
        0x65, 0x20, 0x61, 0x63, 0x63, 0x6F, 0x6D, 0x70, 0x61, 0x6E, 0x69, 0x65, 0x64, 0x20, 0x62, 0x79,
        0x20, 0x61, 0x64, 0x75, 0x6C, 0x74, 0x0D, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x05, 0x4E,
        0x43, 0x2D, 0x31, 0x37, 0x24, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x1C, 0x4E, 0x6F, 0x20,
        0x4F, 0x6E, 0x65, 0x20, 0x31, 0x37, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x55, 0x6E, 0x64, 0x65, 0x72,
        0x20, 0x41, 0x64, 0x6D, 0x69, 0x74, 0x74, 0x65, 0x64, 0x09, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00,
        0x00, 0x01, 0x58, 0x24, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x1C, 0x4E, 0x6F, 0x20, 0x4F,
        0x6E, 0x65, 0x20, 0x31, 0x37, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x55, 0x6E, 0x64, 0x65, 0x72, 0x20,
        0x41, 0x64, 0x6D, 0x69, 0x74, 0x74, 0x65, 0x64, 0x0A, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00,
        0x02, 0x4E, 0x52, 0x19, 0x01, 0x65, 0x6E, 0x67, 0x01, 0x00, 0x00, 0x11, 0x4E, 0x6F, 0x74, 0x20,
        0x52, 0x61, 0x74, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x4D, 0x50, 0x41, 0x41, 0xFC, 0x00, 0xF9,
        0x92, 0xF3, 0x2D,
    };

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                 976U);

    // PSIP generic fields
    RatingRegionTable rrt(si_table);
    QCOMPARE (rrt.SectionSyntaxIndicator(),      true);
    QCOMPARE (rrt.PrivateIndicator(),            true);
    QCOMPARE (rrt.SectionLengthRaw(),            976U);
    QCOMPARE (rrt.TableIDExtension(),         0xFF01U);
    QCOMPARE (rrt.Version(),                       0U);
    QCOMPARE (rrt.IsCurrent(),                   true);
    QCOMPARE (rrt.Section(),                       0U);
    QCOMPARE (rrt.LastSection(),                   0U);
    QCOMPARE (rrt.ATSCProtocolVersion(),           0U);

    // RRT specific fields - none parsed
}

//
// SystemTimeTable Tests
//

void TestMPEGTables::atsc_stt_test(void)
{
    const std::vector<uint8_t> si_data {
        0xCD, 0xF0, 0x11, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x4F, 0xFC, 0x7B, 0x4B, 0x12, 0xFE, 0x02,
        0XD7, 0xF8, 0xF6, 0x81,
    };
    QDateTime expectedDT { QDate(2022,07,15), QTime(18,58,19), Qt::UTC };

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                   17U);

    // PSIP generic fields
    SystemTimeTable stt(si_table);
    QCOMPARE (stt.SectionSyntaxIndicator(),       true);
    QCOMPARE (stt.PrivateIndicator(),             true);
    QCOMPARE (stt.SectionLengthRaw(),              17U);
    QCOMPARE (stt.TableIDExtension(),               0U);
    QCOMPARE (stt.Version(),                        0U);
    QCOMPARE (stt.IsCurrent(),                    true);
    QCOMPARE (stt.Section(),                        0U);
    QCOMPARE (stt.LastSection(),                    0U);
    QCOMPARE (stt.ATSCProtocolVersion(),            0U);

    // STT specific fields
    QCOMPARE (stt.GPSRaw(),                0x4FFC7B4BU);
    QCOMPARE (stt.SystemTimeGPS(),          expectedDT);
    QCOMPARE (stt.GPSOffset(),                     18U);
    QCOMPARE (stt.InDaylightSavingsTime(),        true);
    QCOMPARE (stt.DayDaylightSavingsStarts(),      30U);
    QCOMPARE (stt.HourDaylightSavingsStarts(),      2U);
}

//
// SCTE 35 SpliceInformationTable Tests
//

void TestMPEGTables::scte35_sit_test1(void)
{
    // An actual dummy splice infomation broadcast repeatedly on the
    // MPTS containing MPT-SD and five other stations.
    const std::vector<uint8_t> si_data {
        0xFC, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00,
        0xD0, 0x8C, 0xB5, 0x52,
    };
    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  17U);

    // Splice specific fields
    SpliceInformationTable sit(si_table);
    QCOMPARE (sit.SectionSyntaxIndicator(),     false);
    QCOMPARE (sit.PrivateIndicator(),           false);
    QCOMPARE (sit.SectionLengthRaw(),             17U);
    QCOMPARE (sit.SpliceProtocolVersion(),         0U);
    QCOMPARE (sit.IsEncryptedPacket(),          false);
    QCOMPARE (sit.EncryptionAlgorithm(), (uint)SpliceInformationTable::kNoEncryption);
    QCOMPARE (sit.PTSAdjustment(),               0ULL);
    QCOMPARE (sit.CodeWordIndex(),                 0U);
    QCOMPARE (sit.SpliceCommandLength(),        4095U); // -1
    QCOMPARE (sit.SpliceCommandType(), (uint)SpliceInformationTable::kSCTNull);
}

// A sample splict_scheule packet created by the tstabcomp command
// from https://tsduck.io/.  These values were picked randomly for
// testing and probably don't match what would be in an actual
// packet.
const std::vector<uint8_t> tsduck_sit_schedule {
    0xFC, 0x30, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0xC0, 0x33, 0x04, 0x03, 0x00,
    0xBC, 0x61, 0x4E, 0xFF, 0x00, 0xBC, 0x61, 0x4F, 0x7F, 0xFF, 0x50, 0xB2, 0xBF, 0x22, 0x7E, 0x00,
    0x29, 0x32, 0xE0, 0x38, 0xA8, 0x03, 0x04, 0x00, 0xBC, 0x61, 0x50, 0x7F, 0xBF, 0x02, 0x7B, 0x50,
    0xB2, 0xC0, 0x4E, 0x7C, 0x50, 0xB2, 0xC1, 0x7A, 0xFE, 0x00, 0x52, 0x65, 0xC0, 0x38, 0xA9, 0x05,
    0x06, 0x00, 0x00, 0xA8, 0xA5, 0xA9, 0x06,
};

// A sample splict_insert packet created by the tstabcomp command
// from https://tsduck.io/. This tests the splice_cancel section
// of SpliceInformationTable::Parse insert processing.  These
// values were picked randomly for testing and probably don't
// match what would be in an actual packet.
const std::vector<uint8_t> tsduck_sit_insert1 {
    0xFC, 0x30, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0xC0, 0x05, 0x05, 0x00, 0xBC,
    0x61, 0x4E, 0xFF, 0x00, 0x14, 0x00, 0x08, 0x43, 0x55, 0x45, 0x49, 0x64, 0x75, 0x64, 0x65, 0x00,
    0x08, 0x43, 0x55, 0x45, 0x4A, 0x64, 0x75, 0x64, 0x65, 0xA9, 0x3A, 0x69, 0xAE,
};

// A sample splict_insert packet created by the tstabcomp command
// from https://tsduck.io/. This tests the program_slice section
// of SpliceInformationTable::Parse insert processing.  These
// values were picked randomly for testing and probably don't
// match what would be in an actual packet.
const std::vector<uint8_t> tsduck_sit_insert2 {
    0xFC, 0x30, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0xC0, 0x14, 0x05, 0x00, 0xBC,
    0x61, 0x4E, 0x7F, 0x6F, 0xFE, 0x34, 0x3E, 0xFC, 0xEA, 0xFE, 0x00, 0x29, 0x32, 0xE0, 0x38, 0xA8,
    0x03, 0x04, 0x00, 0x14, 0x00, 0x08, 0x43, 0x55, 0x45, 0x49, 0x64, 0x75, 0x64, 0x65, 0x00, 0x08,
    0x43, 0x55, 0x45, 0x4A, 0x64, 0x75, 0x64, 0x65, 0x73, 0x4C, 0x98, 0x04,
};

// A sample splict_insert packet created by the tstabcomp command
// from https://tsduck.io/. This tests the splice_immediate
// section of SpliceInformationTable::Parse insert processing.
// These values were picked randomly for testing and probably
// don't match what would be in an actual packet.
const std::vector<uint8_t> tsduck_sit_insert3 {
    0xFC, 0x30, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0xC0, 0x1C, 0x05, 0x00, 0xBC,
    0x61, 0x4E, 0x7F, 0x2F, 0x02, 0xD3, 0xFE, 0x34, 0x3E, 0xFC, 0xF4, 0xD5, 0xFE, 0x34, 0x3E, 0xFC,
    0xF6, 0x7E, 0x00, 0x52, 0x65, 0xC0, 0x38, 0xA8, 0x05, 0x06, 0x00, 0x14, 0x00, 0x08, 0x43, 0x55,
    0x45, 0x49, 0x64, 0x75, 0x64, 0x65, 0x00, 0x08, 0x43, 0x55, 0x45, 0x4A, 0x64, 0x75, 0x64, 0x65,
    0xF6, 0x15, 0x88, 0x56,
};

void TestMPEGTables::scte35_sit_schedule_test1a(void)
{
    PSIPTable si_table(tsduck_sit_schedule);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  68U);

    // Splice specific fields
    SpliceInformationTable sit(si_table);
    QCOMPARE (sit.SectionSyntaxIndicator(),     false);
    QCOMPARE (sit.PrivateIndicator(),           false);
    QCOMPARE (sit.SectionLengthRaw(),             68U);
    QCOMPARE (sit.SpliceProtocolVersion(),         0U);
    QCOMPARE (sit.IsEncryptedPacket(),          false);
    QCOMPARE (sit.EncryptionAlgorithm(), (uint)SpliceInformationTable::kNoEncryption);
    QCOMPARE (sit.PTSAdjustment(),              90ULL);
    QCOMPARE (sit.CodeWordIndex(),                 0U);
    QCOMPARE (sit.Tier(),                         12U);

    QCOMPARE (sit.SpliceCommandLength(),          51U);
    QCOMPARE (sit.SpliceCommandType(), (uint)SpliceInformationTable::kSCTSpliceSchedule);

    SpliceScheduleView schedule = sit.SpliceSchedule();
    QCOMPARE (schedule.SpliceCount(),              3U);
    QCOMPARE (schedule.SpliceEventID(0),    12345678U);
    QCOMPARE (schedule.IsSpliceEventCancel(0),   true);

    QCOMPARE (schedule.SpliceEventID(1),    12345679U);
    QCOMPARE (schedule.IsSpliceEventCancel(1),  false);
    QCOMPARE (schedule.IsOutOfNetwork(1),        true);
    QCOMPARE (schedule.IsProgramSplice(1),       true);
    QCOMPARE (schedule.IsDuration(1),            true);
    BreakDurationView bd = schedule.BreakDuration(1);
    QCOMPARE (bd.IsAutoReturn(),                false);
    QCOMPARE (bd.PTSTime(),              0x0002932E0U);
    QCOMPARE (schedule.SpliceTime(1),     0x50B2BF22U);
    QCOMPARE (schedule.UniqueProgramID(1),     14504U);
    QCOMPARE (schedule.AvailNum(1),                3U);
    QCOMPARE (schedule.AvailsExpected(1),          4U);

    QCOMPARE (schedule.SpliceEventID(2),    12345680U);
    QCOMPARE (schedule.IsSpliceEventCancel(2),  false);
    QCOMPARE (schedule.IsOutOfNetwork(2),        true);
    QCOMPARE (schedule.IsProgramSplice(2),      false);
    QCOMPARE (schedule.IsDuration(2),            true);
    bd = schedule.BreakDuration(2);
    QCOMPARE (bd.IsAutoReturn(),                 true);
    QCOMPARE (bd.PTSTime(),              0x0005265C0U);
    QCOMPARE (schedule.ComponentCount(2),          2U);

    QCOMPARE (schedule.ComponentTag(2,0),        123U);
    QCOMPARE (schedule.ComponentSpliceTime(2,0), 0x50B2C04EU);
    QCOMPARE (schedule.ComponentTag(2,1),        124U);
    QCOMPARE (schedule.ComponentSpliceTime(2,1), 0x50B2C17AU);

    QCOMPARE (schedule.UniqueProgramID(2),     14505U);
    QCOMPARE (schedule.AvailNum(2),                5U);
    QCOMPARE (schedule.AvailsExpected(2),          6U);

    QCOMPARE (sit.SpliceDescriptorsLength(),       0U);
}

void TestMPEGTables::scte35_sit_schedule_test1b(void)
{
    // This is the packet from the test 1a modified so that the splice
    // count is 16 instead of 3. Muck the first splice event identifier
    // to ensure that the code sees the phantom 4th splice as a
    // cancel. (The bit is in the checksum field).
    std::vector<uint8_t> si_data = tsduck_sit_schedule;
    si_data[14] = 0x4;
    si_data[18] = 0x67;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 68U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceSchedInfo1);
}

void TestMPEGTables::scte35_sit_schedule_test1c(void)
{
    // This is the packet from the test 1a modified so that the splice
    // count is 16 instead of 3. Muck the first splice event identifier
    // to ensure that the code doesn't sees the phantom 4th splice as a
    // cancel. (The bit is in the checksum field).
    std::vector<uint8_t> si_data = tsduck_sit_schedule;
    si_data[14] = 0x10;
    si_data[18] = 0xC6;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 68U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceSchedInfo2);
}

void TestMPEGTables::scte35_sit_insert_test1a(void)
{
    PSIPTable si_table(tsduck_sit_insert1);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  42U);

    // Splice specific fields
    SpliceInformationTable sit(si_table);
    QCOMPARE (sit.SectionSyntaxIndicator(),     false);
    QCOMPARE (sit.PrivateIndicator(),           false);
    QCOMPARE (sit.SectionLengthRaw(),             42U);
    QCOMPARE (sit.SpliceProtocolVersion(),         0U);
    QCOMPARE (sit.IsEncryptedPacket(),          false);
    QCOMPARE (sit.EncryptionAlgorithm(), (uint)SpliceInformationTable::kNoEncryption);
    QCOMPARE (sit.PTSAdjustment(),              90ULL);
    QCOMPARE (sit.CodeWordIndex(),                 0U);
    QCOMPARE (sit.Tier(),                         12U);

    QCOMPARE (sit.SpliceCommandLength(),           5U);
    QCOMPARE (sit.SpliceCommandType(), (uint)SpliceInformationTable::kSCTSpliceInsert);

    SpliceInsertView insert = sit.SpliceInsert();
    QCOMPARE (insert.SpliceEventID(),       12345678U);
    QCOMPARE (insert.IsSpliceEventCancel(),      true);
    QCOMPARE (insert.IsOutOfNetwork(),          false);
    QCOMPARE (insert.IsProgramSplice(),         false);
    QCOMPARE (insert.IsDuration(),              false);

    QCOMPARE (sit.SpliceDescriptorsLength(),      20U);
    QCOMPARE (sit.SpliceDescriptors(),  sit.data()+21);
    QCOMPARE (sit.SpliceDescriptors()[2],       0x43U);
}

void TestMPEGTables::scte35_sit_insert_test1b(void)
{
    // This is the packet from the test 1a truncated right after the
    // byte containing the cancel indicator.
    std::vector<uint8_t> si_data = tsduck_sit_insert1;
    si_data.resize(19);
    si_data[02] = 16;
    si_data[14] = 0x67;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 16U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceInsertInfo1);
}

void TestMPEGTables::scte35_sit_insert_test2a(void)
{
    PSIPTable si_table(tsduck_sit_insert2);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  57U);

    // Splice specific fields
    SpliceInformationTable sit(si_table);
    QCOMPARE (sit.SectionSyntaxIndicator(),     false);
    QCOMPARE (sit.PrivateIndicator(),           false);
    QCOMPARE (sit.SectionLengthRaw(),             57U);
    QCOMPARE (sit.SpliceProtocolVersion(),         0U);
    QCOMPARE (sit.IsEncryptedPacket(),          false);
    QCOMPARE (sit.EncryptionAlgorithm(), (uint)SpliceInformationTable::kNoEncryption);
    QCOMPARE (sit.PTSAdjustment(),              90ULL);
    QCOMPARE (sit.CodeWordIndex(),                 0U);
    QCOMPARE (sit.Tier(),                         12U);

    QCOMPARE (sit.SpliceCommandLength(),          20U);
    QCOMPARE (sit.SpliceCommandType(), (uint)SpliceInformationTable::kSCTSpliceInsert);

    SpliceInsertView insert = sit.SpliceInsert();
    QCOMPARE (insert.SpliceEventID(),       12345678U);
    QCOMPARE (insert.IsSpliceEventCancel(),     false);
    QCOMPARE (insert.IsOutOfNetwork(),          false);
    QCOMPARE (insert.IsProgramSplice(),          true);
    QCOMPARE (insert.IsDuration(),               true);
    QCOMPARE (insert.IsSpliceImmediate(),       false);
    BreakDurationView bd = insert.BreakDuration();
    QCOMPARE (bd.IsAutoReturn(),                 true);
    QCOMPARE (bd.PTSTime(),              0x0002932E0U);
    QCOMPARE (insert.UniqueProgramID(),        14504U);
    QCOMPARE (insert.AvailNum(),                   3U);
    QCOMPARE (insert.AvailsExpected(),             4U);
    QCOMPARE (insert.ComponentCount(),             0U);

    SpliceTimeView time = insert.SpliceTime();
    QCOMPARE (time.IsTimeSpecified(),            true);
    QCOMPARE (time.PTSTime(),            0x0343EFCEAU);

    QCOMPARE (sit.SpliceDescriptorsLength(),      20U);
    QCOMPARE (sit.SpliceDescriptors(),  sit.data()+36);
    QCOMPARE (sit.SpliceDescriptors()[2],       0x43U);
}

void TestMPEGTables::scte35_sit_insert_test2b(void)
{
    // This is the packet from the test 2a truncated right after the
    // byte containing the program_splice and splice_immediate flags.
    std::vector<uint8_t> si_data = tsduck_sit_insert2;
    si_data.resize(25);
    si_data[02] = 22;
    si_data[20] = 0xFE;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 22U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceInsertInfo2);
}

void TestMPEGTables::scte35_sit_insert_test3a(void)
{
    PSIPTable si_table(tsduck_sit_insert3);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  65U);

    // Splice specific fields
    SpliceInformationTable sit(si_table);
    QCOMPARE (sit.SectionSyntaxIndicator(),     false);
    QCOMPARE (sit.PrivateIndicator(),           false);
    QCOMPARE (sit.SectionLengthRaw(),             65U);
    QCOMPARE (sit.SpliceProtocolVersion(),         0U);
    QCOMPARE (sit.IsEncryptedPacket(),          false);
    QCOMPARE (sit.EncryptionAlgorithm(), (uint)SpliceInformationTable::kNoEncryption);
    QCOMPARE (sit.PTSAdjustment(),              90ULL);
    QCOMPARE (sit.CodeWordIndex(),                 0U);
    QCOMPARE (sit.Tier(),                         12U);

    QCOMPARE (sit.SpliceCommandLength(),          28U);
    QCOMPARE (sit.SpliceCommandType(), (uint)SpliceInformationTable::kSCTSpliceInsert);

    SpliceInsertView insert = sit.SpliceInsert();
    QCOMPARE (insert.SpliceEventID(),       12345678U);
    QCOMPARE (insert.IsSpliceEventCancel(),     false);
    QCOMPARE (insert.IsOutOfNetwork(),          false);
    QCOMPARE (insert.IsProgramSplice(),         false);
    QCOMPARE (insert.IsDuration(),               true);
    QCOMPARE (insert.IsSpliceImmediate(),       false);
    BreakDurationView bd = insert.BreakDuration();
    QCOMPARE (bd.IsAutoReturn(),                false);
    QCOMPARE (bd.PTSTime(),              0x0005265C0U);
    QCOMPARE (insert.UniqueProgramID(),        14504U);
    QCOMPARE (insert.AvailNum(),                   5U);
    QCOMPARE (insert.AvailsExpected(),             6U);
    QCOMPARE (insert.ComponentCount(),             2U);

    QCOMPARE (insert.ComponentTag(0),            211U);
    SpliceTimeView time = insert.ComponentSpliceTime(0);
    QCOMPARE (time.IsTimeSpecified(),            true);
    QCOMPARE (time.PTSTime(),            0x0343EFCF4U);

    QCOMPARE (insert.ComponentTag(1),            213U);
    time = insert.ComponentSpliceTime(1);
    QCOMPARE (time.IsTimeSpecified(),            true);
    QCOMPARE (time.PTSTime(),            0x0343EFCF6U);

    QCOMPARE (sit.SpliceDescriptorsLength(),      20U);
    QCOMPARE (sit.SpliceDescriptors(),  sit.data()+44);
    QCOMPARE (sit.SpliceDescriptors()[2],       0x43U);
}

void TestMPEGTables::scte35_sit_insert_test3b(void)
{
    // This is the packet from the test 3a modified to say there are 100
    // components.
    std::vector<uint8_t> si_data = tsduck_sit_insert3;
    si_data[20] = 0x64;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 65U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceInsertInfo3);
}

void TestMPEGTables::scte35_sit_insert_test3c(void)
{
    // This is the packet from the test 3a modified to truncate the packet
    // just after the component tags.
    std::vector<uint8_t> si_data = tsduck_sit_insert3;
    si_data.resize(42);
    si_data[2] = 0x27;
    update_crc(si_data);

    PSIPTable si_table(si_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(), 39U);

    mpeg_test_throw<SpliceInformationTable>(si_table, PsipParseException::SitSpliceInsertInfo4);
}

//
// DVB NetworkInformationTable Tests
//
const std::vector<uint8_t> au_nit_data {
    0x40, 0xF0, 0x67, 0x32, 0xA0, 0xE1, 0x00, 0x00, 0xF0, 0x05, 0x40, 0x03, 0x53, 0x43, 0x41, 0xF0,
    0x55, 0x08, 0x07, 0x32, 0x0C, 0xF0, 0x4F, 0x5A, 0x0B, 0x01, 0x0E, 0xD7, 0xF0, 0x3B, 0x82, 0x4A,
    0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x18, 0x08, 0x07, 0x01, 0x08, 0x27, 0x01, 0x08, 0x47, 0x01, 0x08,
    0x67, 0x16, 0x08, 0x87, 0x16, 0x08, 0xA7, 0x01, 0x08, 0xC7, 0x19, 0x08, 0xE7, 0x16, 0x5F, 0x04,
    0x00, 0x00, 0x32, 0x00, 0x83, 0x20, 0x08, 0x07, 0xFC, 0x05, 0x08, 0xC7, 0xFC, 0x32, 0x08, 0xA7,
    0xFC, 0x33, 0x08, 0x27, 0xFC, 0x34, 0x08, 0x47, 0xFC, 0x35, 0x08, 0xE7, 0xFC, 0x36, 0x08, 0x67,
    0xFC, 0x37, 0x08, 0x87, 0xFC, 0x38, 0x7B, 0xC3, 0xA9, 0xED,
};

const std::vector<uint8_t> nz_nit_data {
    0x40, 0xF3, 0xDB, 0x34, 0x01, 0xF1, 0x00, 0x01, 0xF0, 0x0B, 0x40, 0x09, 0x46, 0x72, 0x65, 0x65,
    0x76, 0x69, 0x65, 0x77, 0x20, 0xF3, 0xC3, 0x00, 0x19, 0x22, 0x2A, 0xF0, 0x8A, 0x5A, 0x0B, 0x00,
    0x00, 0x00, 0x00, 0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x21, 0x04, 0xB0, 0x19, 0x04,
    0xB1, 0x19, 0x04, 0xB5, 0x19, 0x04, 0xB6, 0x16, 0x04, 0xB7, 0x16, 0x04, 0xB8, 0x16, 0x30, 0xD4,
    0x0C, 0x30, 0xD5, 0x19, 0x30, 0xD6, 0x19, 0x30, 0xD7, 0x19, 0x30, 0xD8, 0x19, 0x62, 0x0D, 0xFF,
    0x03, 0x34, 0xEC, 0x40, 0x03, 0x28, 0xB7, 0x40, 0x03, 0x71, 0xF5, 0x40, 0x5F, 0x04, 0x00, 0x00,
    0x00, 0x37, 0x83, 0x2C, 0x04, 0xB0, 0xFC, 0x01, 0x04, 0xB1, 0xFC, 0x02, 0x04, 0xB5, 0xFC, 0x06,
    0x04, 0xB6, 0xFC, 0x0B, 0x04, 0xB7, 0xFC, 0x07, 0x04, 0xB8, 0xFC, 0x0C, 0x30, 0xD4, 0x7D, 0xF4,
    0x30, 0xD5, 0x7E, 0x58, 0x30, 0xD6, 0x7E, 0x59, 0x30, 0xD7, 0x7E, 0x5A, 0x30, 0xD8, 0x7E, 0x5B,
    0x6D, 0x15, 0x27, 0x74, 0x03, 0x34, 0xEC, 0x40, 0x00, 0x27, 0x75, 0x03, 0x28, 0xB7, 0x40, 0x00,
    0x27, 0xA6, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x00, 0x1A, 0x22, 0x2A, 0xF0, 0x7F, 0x5A, 0x0B, 0x00,
    0x00, 0x00, 0x00, 0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x18, 0x04, 0xB0, 0x19, 0x04,
    0xB1, 0x19, 0x04, 0xB5, 0x19, 0x04, 0xB6, 0x16, 0x04, 0xB7, 0x16, 0x04, 0xB8, 0x16, 0x30, 0xD4,
    0x0C, 0x30, 0xD5, 0x19, 0x62, 0x09, 0xFF, 0x03, 0x71, 0xF5, 0x40, 0x03, 0x7E, 0x2A, 0x40, 0x5F,
    0x04, 0x00, 0x00, 0x00, 0x37, 0x83, 0x20, 0x04, 0xB0, 0xFC, 0x01, 0x04, 0xB1, 0xFC, 0x02, 0x04,
    0xB5, 0xFC, 0x06, 0x04, 0xB6, 0xFC, 0x0B, 0x04, 0xB7, 0xFC, 0x07, 0x04, 0xB8, 0xFC, 0x0C, 0x30,
    0xD4, 0x7D, 0xF4, 0x30, 0xD5, 0x7E, 0x58, 0x6D, 0x23, 0x27, 0xD8, 0x03, 0x71, 0xF5, 0x40, 0x00,
    0x27, 0xD9, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x27, 0xDA, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x27, 0xEC,
    0x03, 0x7E, 0x2A, 0x40, 0x00, 0x28, 0x00, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x00, 0x1B, 0x22, 0x2A,
    0xF0, 0xA2, 0x5A, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41,
    0x15, 0x04, 0xB0, 0x19, 0x04, 0xB1, 0x19, 0x04, 0xB5, 0x19, 0x04, 0xB6, 0x16, 0x04, 0xB7, 0x16,
    0x04, 0xB8, 0x16, 0x30, 0xD4, 0x0C, 0x62, 0x09, 0xFF, 0x03, 0x7E, 0x2A, 0x40, 0x03, 0x71, 0xF5,
    0x40, 0x5F, 0x04, 0x00, 0x00, 0x00, 0x37, 0x83, 0x1C, 0x04, 0xB0, 0xFC, 0x01, 0x04, 0xB1, 0xFC,
    0x02, 0x04, 0xB5, 0xFC, 0x06, 0x04, 0xB6, 0xFC, 0x0B, 0x04, 0xB7, 0xFC, 0x07, 0x30, 0xD4, 0x7D,
    0xF4, 0x04, 0xB8, 0xFC, 0x0C, 0x6D, 0x4D, 0x28, 0x3C, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x28, 0x3D,
    0x03, 0x71, 0xF5, 0x40, 0x00, 0x28, 0x46, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x28, 0x47, 0x03, 0x7E,
    0x2A, 0x40, 0x00, 0x28, 0xA0, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x28, 0xAA, 0x03, 0x71, 0xF5, 0x40,
    0x00, 0x28, 0xB4, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x28, 0xBE, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x29,
    0x04, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x29, 0x05, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x29, 0x08, 0x03,
    0x7E, 0x2A, 0x40, 0x00, 0x00, 0x1C, 0x22, 0x2A, 0xF0, 0x7F, 0x5A, 0x0B, 0x00, 0x00, 0x00, 0x00,
    0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x15, 0x04, 0xB0, 0x19, 0x04, 0xB1, 0x19, 0x04,
    0xB5, 0x19, 0x04, 0xB6, 0x16, 0x04, 0xB7, 0x16, 0x04, 0xB8, 0x16, 0x30, 0xD4, 0x0C, 0x62, 0x09,
    0xFF, 0x03, 0x71, 0xF5, 0x40, 0x03, 0x7E, 0x2A, 0x40, 0x5F, 0x04, 0x00, 0x00, 0x00, 0x37, 0x83,
    0x1C, 0x04, 0xB0, 0xFC, 0x01, 0x04, 0xB1, 0xFC, 0x02, 0x04, 0xB5, 0xFC, 0x06, 0x04, 0xB6, 0xFC,
    0x0B, 0x04, 0xB7, 0xFC, 0x07, 0x04, 0xB8, 0xFC, 0x0C, 0x30, 0xD4, 0x7D, 0xF4, 0x6D, 0x2A, 0x29,
    0x68, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x29, 0x72, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x29, 0x73, 0x03,
    0x71, 0xF5, 0x40, 0x00, 0x29, 0x9A, 0x03, 0x7E, 0x2A, 0x40, 0x00, 0x29, 0xCC, 0x03, 0x71, 0xF5,
    0x40, 0x00, 0x29, 0xFE, 0x03, 0x71, 0xF5, 0x40, 0x00, 0x00, 0x1D, 0x22, 0x2A, 0xF0, 0x63, 0x5A,
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x12, 0x05, 0x14,
    0x19, 0x05, 0x16, 0x16, 0x05, 0x15, 0x19, 0x05, 0x17, 0x19, 0x05, 0x1B, 0x16, 0x05, 0x1C, 0x16,
    0x62, 0x09, 0xFF, 0x03, 0x65, 0xC0, 0x40, 0x03, 0x59, 0x8B, 0x40, 0x5F, 0x04, 0x00, 0x00, 0x00,
    0x37, 0x83, 0x18, 0x05, 0x14, 0xFC, 0x03, 0x05, 0x16, 0xFC, 0x0D, 0x05, 0x15, 0xFC, 0x0E, 0x05,
    0x17, 0xFC, 0x08, 0x05, 0x1B, 0xFC, 0x04, 0x05, 0x1C, 0xFC, 0x09, 0x6D, 0x15, 0x4E, 0x84, 0x03,
    0x65, 0xC0, 0x40, 0x00, 0x4E, 0x85, 0x03, 0x59, 0x8B, 0x40, 0x00, 0x4E, 0xB6, 0x03, 0x59, 0x8B,
    0x40, 0x00, 0x00, 0x1E, 0x22, 0x2A, 0xF0, 0x71, 0x5A, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x82,
    0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x12, 0x05, 0x14, 0x19, 0x05, 0x16, 0x16, 0x05, 0x15, 0x19,
    0x05, 0x17, 0x19, 0x05, 0x1B, 0x16, 0x05, 0x1C, 0x16, 0x62, 0x09, 0xFF, 0x03, 0x65, 0xC0, 0x40,
    0x03, 0x59, 0x8B, 0x40, 0x5F, 0x04, 0x00, 0x00, 0x00, 0x37, 0x83, 0x18, 0x05, 0x14, 0xFC, 0x03,
    0x05, 0x16, 0xFC, 0x0D, 0x05, 0x15, 0xFC, 0x0E, 0x05, 0x17, 0xFC, 0x08, 0x05, 0x1B, 0xFC, 0x04,
    0x05, 0x1C, 0xFC, 0x09, 0x6D, 0x23, 0x4E, 0xE8, 0x03, 0x59, 0x8B, 0x40, 0x00, 0x4E, 0xE9, 0x03,
    0x65, 0xC0, 0x40, 0x00, 0x4E, 0xEA, 0x03, 0x65, 0xC0, 0x40, 0x00, 0x4E, 0xFC, 0x03, 0x65, 0xC0,
    0x40, 0x00, 0x4F, 0x10, 0x03, 0x65, 0xC0, 0x40, 0x00, 0x00, 0x1F, 0x22, 0x2A, 0xF0, 0x9B, 0x5A,
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x82, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x12, 0x05, 0x14,
    0x19, 0x05, 0x16, 0x16, 0x05, 0x15, 0x19, 0x05, 0x17, 0x19, 0x05, 0x1B, 0x16, 0x05, 0x1C, 0x16,
    0x62, 0x09, 0xFF, 0x03, 0x65, 0xC0, 0x40, 0x03, 0x59, 0x8B, 0x40, 0x5F, 0x04, 0x00, 0x00, 0x00,
    0x37, 0x83, 0x18, 0x05, 0x14, 0xFC, 0x03, 0x05, 0x16, 0xFC, 0x0D, 0x05, 0x15, 0xFC, 0x0E, 0x05,
    0x17, 0xFC, 0x08, 0x05, 0x1B, 0xFC, 0x04, 0x05, 0x1C, 0xFC, 0x09, 0x6D, 0x4D, 0x4F, 0x4C, 0x03,
    0x65, 0xC0, 0x40, 0x00, 0x4F, 0x4D, 0x03, 0x59, 0x8B, 0x40, 0x00, 0x4F, 0x56, 0x03, 0x59, 0x8B,
    0x40, 0x00, 0x4F, 0x57, 0x03, 0x65, 0xC0, 0x40, 0x00, 0x4F, 0xB0, 0x03, 0x59, 0x8B, 0x40, 0x00,
    0x4F, 0xBA, 0x03, 0x59, 0x8B, 0x40, 0x00, 0x4F, 0xC4, 0x03, 0x65, 0xC0, 0x40, 0x00, 0x4F, 0xCE,
    0x03, 0x65, 0xC0, 0x40, 0x00, 0x50, 0x14, 0x03, 0x59, 0x8B, 0x40, 0x00, 0x50, 0x15, 0x03, 0x65,
    0xC0, 0x40, 0x00, 0x50, 0x18, 0x03, 0x65, 0xC0, 0x40, 0x00, 0x09, 0xC8, 0x3C, 0x18,
};

void TestMPEGTables::dvb_nit_test1(void)
{
    PSIPTable si_table(au_nit_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                    103U);

    // PSIP generic fields
    NetworkInformationTable nit(si_table);
    QCOMPARE (nit.SectionSyntaxIndicator(),         true);
    QCOMPARE (nit.PrivateIndicator(),               true);
    QCOMPARE (nit.SectionLengthRaw(),               103U);
    QCOMPARE (nit.NetworkID(),                    12960U);
    QCOMPARE (nit.Version(),                         16U);
    QCOMPARE (nit.IsCurrent(),                      true);
    QCOMPARE (nit.Section(),                          0U);
    QCOMPARE (nit.LastSection(),                      0U);

    // NIT specific fields
    QCOMPARE (nit.NetworkDescriptorsLength(),         5U);
    QCOMPARE (nit.NetworkName(),                   "SCA");

    // NIT Transport Streams
    QCOMPARE (nit.TransportStreamDataLength(),       85U);
    QCOMPARE (nit.TransportStreamCount(),             1U);
    QCOMPARE (nit.TSID(0),                         2055U);
    QCOMPARE (nit.OriginalNetworkID(0),           12812U);
    QCOMPARE (nit.TransportDescriptorsLength(0),     79U);
    QCOMPARE (nit.TransportDescriptors(0), nit.data()+23);
    // No parsing of transport stream descriptors
}

void TestMPEGTables::dvb_nit_test2(void)
{
    PSIPTable si_table(nz_nit_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                     987U);

    // PSIP generic fields
    NetworkInformationTable nit(si_table);
    QCOMPARE (nit.SectionSyntaxIndicator(),          true);
    QCOMPARE (nit.PrivateIndicator(),                true);
    QCOMPARE (nit.SectionLengthRaw(),                987U);
    QCOMPARE (nit.NetworkID(),                     13313U);
    QCOMPARE (nit.Version(),                          24U);
    QCOMPARE (nit.IsCurrent(),                       true);
    QCOMPARE (nit.Section(),                           0U);
    QCOMPARE (nit.LastSection(),                       1U);

    // NIT specific fields
    QCOMPARE (nit.NetworkDescriptorsLength(),         11U);
    QCOMPARE (nit.NetworkName(),              "Freeview ");

    // NIT Transport Streams
    // No parsing of transport stream descriptors
    QCOMPARE (nit.TransportStreamDataLength(),       963U);
    QCOMPARE (nit.TransportStreamCount(),              7U);

    QCOMPARE (nit.TSID(0),                            25U);
    QCOMPARE (nit.OriginalNetworkID(0),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(0),     138U);
    QCOMPARE (nit.TransportDescriptors(0),  nit.data()+29);

    QCOMPARE (nit.TSID(1),                            26U);
    QCOMPARE (nit.OriginalNetworkID(1),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(1),     127U);
    QCOMPARE (nit.TransportDescriptors(1), nit.data()+173);

    QCOMPARE (nit.TSID(2),                            27U);
    QCOMPARE (nit.OriginalNetworkID(2),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(2),     162U);
    QCOMPARE (nit.TransportDescriptors(2), nit.data()+306);

    QCOMPARE (nit.TSID(3),                            28U);
    QCOMPARE (nit.OriginalNetworkID(3),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(3),     127U);
    QCOMPARE (nit.TransportDescriptors(3), nit.data()+474);

    QCOMPARE (nit.TSID(4),                            29U);
    QCOMPARE (nit.OriginalNetworkID(4),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(4),      99U);
    QCOMPARE (nit.TransportDescriptors(4), nit.data()+607);

    QCOMPARE (nit.TSID(5),                            30U);
    QCOMPARE (nit.OriginalNetworkID(5),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(5),     113U);
    QCOMPARE (nit.TransportDescriptors(5), nit.data()+712);

    QCOMPARE (nit.TSID(6),                            31U);
    QCOMPARE (nit.OriginalNetworkID(6),             8746U);
    QCOMPARE (nit.TransportDescriptorsLength(6),     155U);
    QCOMPARE (nit.TransportDescriptors(6), nit.data()+831);
}

//
// DVB ServiceDescriptionTable Tests
//
const std::vector<uint8_t> au_sdt_data {
    0x42, 0xF1, 0x5B, 0x08, 0x07, 0xED, 0x00, 0x00, 0x32, 0x0C, 0xFF, 0x08, 0x07, 0xFF, 0x80, 0x26,
    0x73, 0x11, 0x63, 0x61, 0x6E, 0x62, 0x65, 0x72, 0x72, 0x61, 0x2E, 0x31, 0x2E, 0x73, 0x63, 0x61,
    0x2E, 0x61, 0x75, 0x48, 0x11, 0x01, 0x03, 0x53, 0x43, 0x41, 0x0B, 0x31, 0x30, 0x20, 0x43, 0x61,
    0x6E, 0x62, 0x65, 0x72, 0x72, 0x61, 0x08, 0x27, 0xFF, 0x80, 0x23, 0x73, 0x11, 0x63, 0x61, 0x6E,
    0x62, 0x65, 0x72, 0x72, 0x61, 0x2E, 0x32, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61, 0x75, 0x48, 0x0E,
    0x01, 0x03, 0x53, 0x43, 0x41, 0x08, 0x31, 0x30, 0x20, 0x50, 0x65, 0x61, 0x63, 0x68, 0x08, 0x47,
    0xFF, 0x80, 0x22, 0x73, 0x11, 0x63, 0x61, 0x6E, 0x62, 0x65, 0x72, 0x72, 0x61, 0x2E, 0x33, 0x2E,
    0x73, 0x63, 0x61, 0x2E, 0x61, 0x75, 0x48, 0x0D, 0x01, 0x03, 0x53, 0x43, 0x41, 0x07, 0x31, 0x30,
    0x20, 0x42, 0x4F, 0x4C, 0x44, 0x08, 0x67, 0xFF, 0x80, 0x1E, 0x73, 0x11, 0x63, 0x61, 0x6E, 0x62,
    0x65, 0x72, 0x72, 0x61, 0x2E, 0x34, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61, 0x75, 0x48, 0x09, 0x16,
    0x03, 0x53, 0x43, 0x41, 0x03, 0x53, 0x42, 0x4E, 0x08, 0x87, 0xFF, 0x80, 0x2C, 0x73, 0x11, 0x63,
    0x61, 0x6E, 0x62, 0x65, 0x72, 0x72, 0x61, 0x2E, 0x35, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61, 0x75,
    0x48, 0x17, 0x16, 0x03, 0x53, 0x43, 0x41, 0x11, 0x53, 0x6B, 0x79, 0x20, 0x4E, 0x65, 0x77, 0x73,
    0x20, 0x52, 0x65, 0x67, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x08, 0xA7, 0xFF, 0x80, 0x26, 0x73, 0x11,
    0x63, 0x61, 0x6E, 0x62, 0x65, 0x72, 0x72, 0x61, 0x2E, 0x36, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61,
    0x75, 0x48, 0x11, 0x01, 0x03, 0x53, 0x43, 0x41, 0x0B, 0x31, 0x30, 0x20, 0x43, 0x61, 0x6E, 0x62,
    0x65, 0x72, 0x72, 0x61, 0x08, 0xC7, 0xFF, 0x80, 0x29, 0x73, 0x11, 0x63, 0x61, 0x6E, 0x62, 0x65,
    0x72, 0x72, 0x61, 0x2E, 0x37, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61, 0x75, 0x48, 0x14, 0x19, 0x03,
    0x53, 0x43, 0x41, 0x0E, 0x31, 0x30, 0x20, 0x48, 0x44, 0x20, 0x43, 0x61, 0x6E, 0x62, 0x65, 0x72,
    0x72, 0x61, 0x08, 0xE7, 0xFF, 0x80, 0x23, 0x73, 0x11, 0x63, 0x61, 0x6E, 0x62, 0x65, 0x72, 0x72,
    0x61, 0x2E, 0x38, 0x2E, 0x73, 0x63, 0x61, 0x2E, 0x61, 0x75, 0x48, 0x0E, 0x16, 0x03, 0x53, 0x43,
    0x41, 0x08, 0x31, 0x30, 0x20, 0x53, 0x48, 0x41, 0x4B, 0x45, 0xBF, 0x08, 0x88, 0xE6,
};

const std::vector<uint8_t> nz_sdt_actual_data {
    0x42, 0xF0, 0x98, 0x00, 0x23, 0xCD, 0x00, 0x00, 0x22, 0x2A, 0xFF, 0x06, 0x40, 0xFF, 0x80, 0x37,
    0x48, 0x19, 0x19, 0x06, 0x4B, 0x6F, 0x72, 0x64, 0x69, 0x61, 0x10, 0x4D, 0x61, 0x6F, 0x72, 0x69,
    0x20, 0x54, 0x65, 0x6C, 0x65, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x73, 0x1A, 0x63, 0x72, 0x69,
    0x64, 0x3A, 0x2F, 0x2F, 0x6D, 0x61, 0x6F, 0x72, 0x69, 0x74, 0x65, 0x6C, 0x65, 0x76, 0x69, 0x73,
    0x69, 0x6F, 0x6E, 0x2E, 0x63, 0x6F, 0x6D, 0x06, 0x41, 0xFF, 0x80, 0x2D, 0x48, 0x0F, 0x19, 0x06,
    0x4B, 0x6F, 0x72, 0x64, 0x69, 0x61, 0x06, 0x54, 0x65, 0x20, 0x52, 0x65, 0x6F, 0x73, 0x1A, 0x63,
    0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x6D, 0x61, 0x6F, 0x72, 0x69, 0x74, 0x65, 0x6C, 0x65, 0x76,
    0x69, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x63, 0x6F, 0x6D, 0x3E, 0x80, 0xFC, 0x80, 0x19, 0x48, 0x17,
    0x16, 0x06, 0x4B, 0x6F, 0x72, 0x64, 0x69, 0x61, 0x0E, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x69, 0x63, 0x65, 0x20, 0x31, 0xBF, 0x5E, 0x3C, 0x10,
};

const std::vector<uint8_t> nz_sdt_other_data {
    0x46, 0xF1, 0x1E, 0x00, 0x1B, 0xD7, 0x00, 0x00, 0x22, 0x2A, 0xFF, 0x04, 0xB0, 0xFD, 0x80, 0x22,
    0x48, 0x0D, 0x19, 0x04, 0x54, 0x56, 0x4E, 0x5A, 0x06, 0x54, 0x56, 0x4E, 0x5A, 0x20, 0x31, 0x73,
    0x11, 0x63, 0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x74, 0x76, 0x6E, 0x7A, 0x2E, 0x63, 0x6F, 0x2E,
    0x6E, 0x7A, 0x04, 0xB1, 0xFD, 0x80, 0x22, 0x48, 0x0D, 0x19, 0x04, 0x54, 0x56, 0x4E, 0x5A, 0x06,
    0x54, 0x56, 0x4E, 0x5A, 0x20, 0x32, 0x73, 0x11, 0x63, 0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x74,
    0x76, 0x6E, 0x7A, 0x2E, 0x63, 0x6F, 0x2E, 0x6E, 0x7A, 0x04, 0xB5, 0xFD, 0x80, 0x25, 0x48, 0x10,
    0x19, 0x04, 0x54, 0x56, 0x4E, 0x5A, 0x09, 0x54, 0x56, 0x4E, 0x5A, 0x20, 0x44, 0x55, 0x4B, 0x45,
    0x73, 0x11, 0x63, 0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x74, 0x76, 0x6E, 0x7A, 0x2E, 0x63, 0x6F,
    0x2E, 0x6E, 0x7A, 0x04, 0xB6, 0xFD, 0x80, 0x25, 0x48, 0x10, 0x16, 0x04, 0x54, 0x56, 0x4E, 0x5A,
    0x09, 0x54, 0x56, 0x4E, 0x5A, 0x20, 0x31, 0x20, 0x2B, 0x31, 0x73, 0x11, 0x63, 0x72, 0x69, 0x64,
    0x3A, 0x2F, 0x2F, 0x74, 0x76, 0x6E, 0x7A, 0x2E, 0x63, 0x6F, 0x2E, 0x6E, 0x7A, 0x04, 0xB7, 0xFD,
    0x80, 0x25, 0x48, 0x10, 0x16, 0x04, 0x54, 0x56, 0x4E, 0x5A, 0x09, 0x54, 0x56, 0x4E, 0x5A, 0x20,
    0x32, 0x20, 0x2B, 0x31, 0x73, 0x11, 0x63, 0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x74, 0x76, 0x6E,
    0x7A, 0x2E, 0x63, 0x6F, 0x2E, 0x6E, 0x7A, 0x04, 0xB8, 0xFD, 0x80, 0x27, 0x48, 0x12, 0x16, 0x04,
    0x54, 0x56, 0x4E, 0x5A, 0x0B, 0x54, 0x56, 0x4E, 0x5A, 0x20, 0x44, 0x55, 0x4B, 0x45, 0x2B, 0x31,
    0x73, 0x11, 0x63, 0x72, 0x69, 0x64, 0x3A, 0x2F, 0x2F, 0x74, 0x76, 0x6E, 0x7A, 0x2E, 0x63, 0x6F,
    0x2E, 0x6E, 0x7A, 0x30, 0xD4, 0xFC, 0x80, 0x15, 0x48, 0x13, 0x0C, 0x04, 0x54, 0x56, 0x4E, 0x5A,
    0x0C, 0x49, 0x4E, 0x46, 0x4F, 0x20, 0x43, 0x48, 0x41, 0x4E, 0x4E, 0x45, 0x4C, 0xA8, 0x57, 0xB0,
    0xA2,
};

void TestMPEGTables::dvb_sdt_test1(void)
{
    PSIPTable si_table(au_sdt_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                    347U);

    // PSIP generic fields
    ServiceDescriptionTable sdt(si_table);
    QCOMPARE (sdt.SectionSyntaxIndicator(),         true);
    QCOMPARE (sdt.PrivateIndicator(),               true);
    QCOMPARE (sdt.SectionLengthRaw(),               347U);
    QCOMPARE (sdt.TSID(),                          2055U);
    QCOMPARE (sdt.Version(),                         22U);
    QCOMPARE (sdt.IsCurrent(),                      true);
    QCOMPARE (sdt.Section(),                          0U);
    QCOMPARE (sdt.LastSection(),                      0U);

    // SDT specific fields
    QCOMPARE (sdt.OriginalNetworkID(),            12812U);
    QCOMPARE (sdt.ServiceCount(),                     8U);

    QCOMPARE (sdt.ServiceID(0),                    2055U);
    QCOMPARE (sdt.HasEITSchedule(0),                true);
    QCOMPARE (sdt.HasEITPresentFollowing(0),        true);
    QCOMPARE (sdt.RunningStatus(0),                   4U);
    QCOMPARE (sdt.IsEncrypted(0),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(0),       38U);
    QCOMPARE (sdt.ServiceDescriptors(0),  sdt.data()+ 16);
    {
        ServiceDescriptor *sd = sdt.GetServiceDescriptor(0);
        QCOMPARE (sd->ServiceType(),                 1U);
        QCOMPARE (sd->ServiceProviderNameLength(),   3U);
        QCOMPARE (sd->ServiceProviderName(),      "SCA");
        QCOMPARE (sd->ServiceProviderShortName(), "SCA");
        QCOMPARE (sd->ServiceNameLength(),          11U);
        QCOMPARE (sd->ServiceName(),      "10 Canberra");
    }

    QCOMPARE (sdt.ServiceID(1),                    2087U);
    QCOMPARE (sdt.HasEITSchedule(1),                true);
    QCOMPARE (sdt.HasEITPresentFollowing(1),        true);
    QCOMPARE (sdt.RunningStatus(1),                   4U);
    QCOMPARE (sdt.IsEncrypted(1),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(1),       35U);
    QCOMPARE (sdt.ServiceDescriptors(1),  sdt.data()+ 59);
    {
        ServiceDescriptor *sd = sdt.GetServiceDescriptor(1);
        QCOMPARE (sd->ServiceType(),                 1U);
        QCOMPARE (sd->ServiceProviderNameLength(),   3U);
        QCOMPARE (sd->ServiceProviderName(),      "SCA");
        QCOMPARE (sd->ServiceProviderShortName(), "SCA");
        QCOMPARE (sd->ServiceNameLength(),           8U);
        QCOMPARE (sd->ServiceName(),         "10 Peach");
    }

    QCOMPARE (sdt.ServiceDescriptors(2),  sdt.data()+ 99);
    QCOMPARE (sdt.ServiceDescriptors(3),  sdt.data()+138);
    QCOMPARE (sdt.ServiceDescriptors(4),  sdt.data()+173);
    QCOMPARE (sdt.ServiceDescriptors(5),  sdt.data()+222);
    QCOMPARE (sdt.ServiceDescriptors(6),  sdt.data()+265);

    QCOMPARE (sdt.ServiceID(7),                    2279U);
    QCOMPARE (sdt.HasEITSchedule(7),                true);
    QCOMPARE (sdt.HasEITPresentFollowing(7),        true);
    QCOMPARE (sdt.RunningStatus(7),                   4U);
    QCOMPARE (sdt.IsEncrypted(7),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(7),       35U);
    QCOMPARE (sdt.ServiceDescriptors(7),  sdt.data()+311);
    {
        ServiceDescriptor *sd = sdt.GetServiceDescriptor(7);
        QCOMPARE (sd->ServiceType(),                22U);
        QCOMPARE (sd->ServiceProviderNameLength(),   3U);
        QCOMPARE (sd->ServiceProviderName(),      "SCA");
        QCOMPARE (sd->ServiceProviderShortName(), "SCA");
        QCOMPARE (sd->ServiceNameLength(),           8U);
        QCOMPARE (sd->ServiceName(),         "10 SHAKE");
    }
}

void TestMPEGTables::dvb_sdt_test2a(void)
{
    PSIPTable si_table(nz_sdt_actual_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                      152U);

    // PSIP generic fields
    ServiceDescriptionTable sdt(si_table);
    QCOMPARE (sdt.SectionSyntaxIndicator(),           true);
    QCOMPARE (sdt.PrivateIndicator(),                 true);
    QCOMPARE (sdt.SectionLengthRaw(),                 152U);
    QCOMPARE (sdt.TSID(),                              35U);
    QCOMPARE (sdt.Version(),                            6U);
    QCOMPARE (sdt.IsCurrent(),                        true);
    QCOMPARE (sdt.Section(),                            0U);
    QCOMPARE (sdt.LastSection(),                        0U);

    // SDT specific fields
    QCOMPARE (sdt.OriginalNetworkID(),               8746U);
    QCOMPARE (sdt.ServiceCount(),                       3U);

    QCOMPARE (sdt.ServiceID(0),                      1600U);
    QCOMPARE (sdt.HasEITSchedule(0),                  true);
    QCOMPARE (sdt.HasEITPresentFollowing(0),          true);
    QCOMPARE (sdt.RunningStatus(0),                     4U);
    QCOMPARE (sdt.IsEncrypted(0),                    false);
    QCOMPARE (sdt.ServiceDescriptorsLength(0),         55U);
    QCOMPARE (sdt.ServiceDescriptors(0),    sdt.data()+ 16);
    {
        ServiceDescriptor *sd0 = sdt.GetServiceDescriptor(0);
        QCOMPARE (sd0->ServiceType(),                   25U);
        QCOMPARE (sd0->ServiceProviderNameLength(),      6U);
        QCOMPARE (sd0->ServiceProviderName(),      "Kordia");
        QCOMPARE (sd0->ServiceProviderShortName(), "Kordia");
        QCOMPARE (sd0->ServiceNameLength(),             16U);
        QCOMPARE (sd0->ServiceName(),    "Maori Television");
    }

    QCOMPARE (sdt.ServiceID(1),                      1601U);
    QCOMPARE (sdt.HasEITSchedule(1),                  true);
    QCOMPARE (sdt.HasEITPresentFollowing(1),          true);
    QCOMPARE (sdt.RunningStatus(1),                     4U);
    QCOMPARE (sdt.IsEncrypted(1),                    false);
    QCOMPARE (sdt.ServiceDescriptorsLength(1),         45U);
    QCOMPARE (sdt.ServiceDescriptors(1),    sdt.data()+ 76);
    {
        ServiceDescriptor *sd1 = sdt.GetServiceDescriptor(1);
        QCOMPARE (sd1->ServiceType(),                   25U);
        QCOMPARE (sd1->ServiceProviderNameLength(),      6U);
        QCOMPARE (sd1->ServiceProviderName(),      "Kordia");
        QCOMPARE (sd1->ServiceProviderShortName(), "Kordia");
        QCOMPARE (sd1->ServiceNameLength(),              6U);
        QCOMPARE (sd1->ServiceName(),              "Te Reo");
    }

    QCOMPARE (sdt.ServiceID(2),                     16000U);
    QCOMPARE (sdt.HasEITSchedule(2),                 false);
    QCOMPARE (sdt.HasEITPresentFollowing(2),         false);
    QCOMPARE (sdt.RunningStatus(2),                     4U);
    QCOMPARE (sdt.IsEncrypted(2),                    false);
    QCOMPARE (sdt.ServiceDescriptorsLength(2),         25U);
    QCOMPARE (sdt.ServiceDescriptors(2),    sdt.data()+126);
    {
        ServiceDescriptor *sd2 = sdt.GetServiceDescriptor(2);
        QCOMPARE (sd2->ServiceType(),                   22U);
        QCOMPARE (sd2->ServiceProviderNameLength(),      6U);
        QCOMPARE (sd2->ServiceProviderName(),      "Kordia");
        QCOMPARE (sd2->ServiceProviderShortName(), "Kordia");
        QCOMPARE (sd2->ServiceNameLength(),             14U);
        QCOMPARE (sd2->ServiceName(),      "Test Service 1");
    }
}

void TestMPEGTables::dvb_sdt_test2o(void)
{
    PSIPTable si_table(nz_sdt_other_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                    286U);

    // PSIP generic fields
    ServiceDescriptionTable sdt(si_table);
    QCOMPARE (sdt.SectionSyntaxIndicator(),         true);
    QCOMPARE (sdt.PrivateIndicator(),               true);
    QCOMPARE (sdt.SectionLengthRaw(),               286U);
    QCOMPARE (sdt.TSID(),                            27U);
    QCOMPARE (sdt.Version(),                         11U);
    QCOMPARE (sdt.IsCurrent(),                      true);
    QCOMPARE (sdt.Section(),                          0U);
    QCOMPARE (sdt.LastSection(),                      0U);

    // SDT specific fields
    QCOMPARE (sdt.OriginalNetworkID(),             8746U);
    QCOMPARE (sdt.ServiceCount(),                     7U);

    QCOMPARE (sdt.ServiceID(0),                    1200U);
    QCOMPARE (sdt.HasEITSchedule(0),               false);
    QCOMPARE (sdt.HasEITPresentFollowing(0),        true);
    QCOMPARE (sdt.RunningStatus(0),                   4U);
    QCOMPARE (sdt.IsEncrypted(0),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(0),       34U);
    QCOMPARE (sdt.ServiceDescriptors(0),  sdt.data()+ 16);
    {
        ServiceDescriptor *sd0 = sdt.GetServiceDescriptor(0);
        QCOMPARE (sd0->ServiceType(),                 25U);
        QCOMPARE (sd0->ServiceProviderNameLength(),    4U);
        QCOMPARE (sd0->ServiceProviderName(),      "TVNZ");
        QCOMPARE (sd0->ServiceProviderShortName(), "TVNZ");
        QCOMPARE (sd0->ServiceNameLength(),            6U);
        QCOMPARE (sd0->ServiceName(),            "TVNZ 1");
    }

    QCOMPARE (sdt.ServiceID(1),                    1201U);
    QCOMPARE (sdt.HasEITSchedule(1),               false);
    QCOMPARE (sdt.HasEITPresentFollowing(1),        true);
    QCOMPARE (sdt.RunningStatus(1),                   4U);
    QCOMPARE (sdt.IsEncrypted(1),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(1),       34U);
    QCOMPARE (sdt.ServiceDescriptors(1),  sdt.data()+ 55);
    {
        ServiceDescriptor *sd1 = sdt.GetServiceDescriptor(1);
        QCOMPARE (sd1->ServiceType(),                 25U);
        QCOMPARE (sd1->ServiceProviderNameLength(),    4U);
        QCOMPARE (sd1->ServiceProviderName(),      "TVNZ");
        QCOMPARE (sd1->ServiceProviderShortName(), "TVNZ");
        QCOMPARE (sd1->ServiceNameLength(),            6U);
        QCOMPARE (sd1->ServiceName(),            "TVNZ 2");
    }

    QCOMPARE (sdt.ServiceDescriptors(2),  sdt.data()+ 94);
    QCOMPARE (sdt.ServiceDescriptors(3),  sdt.data()+136);
    QCOMPARE (sdt.ServiceDescriptors(4),  sdt.data()+178);
    QCOMPARE (sdt.ServiceDescriptors(5),  sdt.data()+220);

    QCOMPARE (sdt.ServiceID(6),                   12500U);
    QCOMPARE (sdt.HasEITSchedule(6),               false);
    QCOMPARE (sdt.HasEITPresentFollowing(6),       false);
    QCOMPARE (sdt.RunningStatus(6),                   4U);
    QCOMPARE (sdt.IsEncrypted(6),                  false);
    QCOMPARE (sdt.ServiceDescriptorsLength(6),       21U);
    QCOMPARE (sdt.ServiceDescriptors(6),  sdt.data()+264);
    {
        ServiceDescriptor *sd6 = sdt.GetServiceDescriptor(6);
        QCOMPARE (sd6->ServiceType(),                 12U);
        QCOMPARE (sd6->ServiceProviderNameLength(),    4U);
        QCOMPARE (sd6->ServiceProviderName(),      "TVNZ");
        QCOMPARE (sd6->ServiceProviderShortName(), "TVNZ");
        QCOMPARE (sd6->ServiceNameLength(),           12U);
        QCOMPARE (sd6->ServiceName(),      "INFO CHANNEL");
    }
}

//
// DVB EventInformationTable Tests
//

const std::vector<uint8_t> nz_dvb_eit_data {
    0x4E, 0xF0, 0xFD, 0x06, 0x41, 0xC1, 0x00, 0x01, 0x00, 0x23, 0x22, 0x2A, 0x01, 0x4E, 0x00, 0xB5,
    0xE9, 0x7A, 0x11, 0x00, 0x00, 0x12, 0x30, 0x00, 0x80, 0xE2, 0x54, 0x02, 0x30, 0x00, 0x4D, 0xC6,
    0x65, 0x6E, 0x67, 0x07, 0x05, 0x54, 0x65, 0x20, 0x52, 0x65, 0x6F, 0xBA, 0x05, 0x4D, 0x61, 0x6F,
    0x72, 0x69, 0x20, 0x54, 0x65, 0x6C, 0x65, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x70,
    0x65, 0x63, 0x69, 0x61, 0x6C, 0x69, 0x73, 0x65, 0x73, 0x20, 0x69, 0x6E, 0x20, 0x74, 0x68, 0x65,
    0x20, 0x73, 0x74, 0x6F, 0x72, 0x69, 0x65, 0x73, 0x20, 0x6F, 0x66, 0x20, 0x41, 0x6F, 0x74, 0x65,
    0x61, 0x72, 0x6F, 0x61, 0x2E, 0x20, 0x42, 0x65, 0x20, 0x65, 0x6E, 0x74, 0x65, 0x72, 0x74, 0x61,
    0x69, 0x6E, 0x65, 0x64, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x65,
    0x64, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x6E, 0x65, 0x77, 0x73, 0x2C, 0x20, 0x63, 0x75, 0x72,
    0x72, 0x65, 0x6E, 0x74, 0x20, 0x61, 0x66, 0x66, 0x61, 0x69, 0x72, 0x73, 0x2C, 0x20, 0x73, 0x70,
    0x6F, 0x72, 0x74, 0x73, 0x2C, 0x20, 0x64, 0x72, 0x61, 0x6D, 0x61, 0x2C, 0x20, 0x64, 0x6F, 0x63,
    0x75, 0x6D, 0x65, 0x6E, 0x74, 0x61, 0x72, 0x69, 0x65, 0x73, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x6D,
    0x75, 0x63, 0x68, 0x20, 0x6D, 0x6F, 0x72, 0x65, 0x2E, 0x20, 0x47, 0x6F, 0x20, 0x74, 0x6F, 0x20,
    0x77, 0x77, 0x77, 0x2E, 0x4D, 0x61, 0x6F, 0x72, 0x69, 0x74, 0x65, 0x6C, 0x65, 0x76, 0x69, 0x73,
    0x69, 0x6F, 0x6E, 0x2E, 0x2E, 0x2E, 0x55, 0x04, 0x4E, 0x5A, 0x4C, 0x00, 0x50, 0x06, 0xF6, 0x03,
    0x14, 0x6D, 0x61, 0x6F, 0x50, 0x06, 0xF5, 0x0B, 0x01, 0x6D, 0x61, 0x6F, 0x76, 0x8A, 0x5F, 0x20,
};

void TestMPEGTables::dvb_eit_test1(void)
{
    PSIPTable si_table(nz_dvb_eit_data);
    QVERIFY  (si_table.IsGood());
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),                  253U);

    // PSIP generic fields
    DVBEventInformationTable eit(si_table);
    QCOMPARE (eit.SectionSyntaxIndicator(),       true);
    QCOMPARE (eit.PrivateIndicator(),             true);
    QCOMPARE (eit.SectionLengthRaw(),             253U);
    QCOMPARE (eit.ServiceID(),                   1601U);
    QCOMPARE (eit.Version(),                        0U);
    QCOMPARE (eit.IsCurrent(),                    true);
    QCOMPARE (eit.Section(),                        0U);
    QCOMPARE (eit.LastSection(),                    1U);

    // EIT specific fields
    QCOMPARE (eit.TSID(),                          35U);
    QCOMPARE (eit.OriginalNetworkID(),           8746U);
    QCOMPARE (eit.SegmentLastSectionNumber(),       1U);
    QCOMPARE (eit.LastTableID(),                   78U);

    // EIT table data
    QDateTime expectedDT {QDate(2022,07,10), QTime(11,00,00), Qt::UTC};
    QCOMPARE (eit.EventID(0),                     181U);
    QCOMPARE (eit.StartTimeUTC(0),          expectedDT);
    QCOMPARE (eit.StartTimeUnixUTC(0),     1657450800U);
    QCOMPARE (eit.EndTimeUnixUTC(0),       1657495800U);
    QCOMPARE (eit.DurationInSeconds(0),         45000U);
    QCOMPARE (eit.RunningStatus(0),                 4U);
    QCOMPARE (eit.IsScrambled(0),                false);
    QCOMPARE (eit.DescriptorsLength(0),           226U);
    QCOMPARE (eit.Descriptors(0),       eit.data()+26U);
}

//
// DVB TimeDateTable Tests
//

void TestMPEGTables::dvb_tdt_test1(void)
{
    const std::vector<uint8_t> si_data {
        0x70, 0x70, 0x05, 0xE9, 0x69, 0x03, 0x46, 0x24,
    };
    QDateTime expectedDT { QDate(2022,06,23), QTime(03,46,24), Qt::UTC };
    PSIPTable si_table(si_data);
    QVERIFY  (!si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),               5U);

    // PSIP generic fields
    TimeDateTable tdt(si_table);
    QCOMPARE (tdt.SectionSyntaxIndicator(), false);
    QCOMPARE (tdt.SectionLengthRaw(),          5U);

    // TDT specific fields
    QCOMPARE (tdt.UTC(),               expectedDT);
}

void TestMPEGTables::dvb_tdt_test2(void)
{
    const std::vector<uint8_t> nz_tdt_data {
        0x70, 0x70, 0x05, 0xE9, 0x7A, 0x12, 0x23, 0x52,
    };
    QDateTime expectedDT { QDate(2022,07,10), QTime(12,23,52), Qt::UTC };
    PSIPTable si_table(nz_tdt_data);
    QVERIFY  (!si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),               5U);

    // PSIP generic fields
    TimeDateTable tdt(si_table);
    QCOMPARE (tdt.SectionSyntaxIndicator(), false);
    QCOMPARE (tdt.SectionLengthRaw(),          5U);

    // TDT specific fields
    QCOMPARE (tdt.UTC(),               expectedDT);
    QCOMPARE (tdt.UTCUnix(),          1657455832U);
}

void TestMPEGTables::dvb_tot_test2(void)
{
    const std::vector<uint8_t> nz_tot_data {
        0x73, 0x70, 0x1A, 0xE9, 0x7A, 0x12, 0x23, 0x52, 0xF0, 0x0F, 0x58, 0x0D, 0x4E, 0x5A, 0x4C, 0x02,
        0x12, 0x00, 0xE9, 0x17, 0x14, 0x00, 0x00, 0x12, 0x00, 0xF2, 0x8F, 0xF4, 0x04,
    };
    QDateTime expectedDT { QDate(2022,07,10), QTime(12,23,52), Qt::UTC };
    PSIPTable si_table(nz_tot_data);
    QVERIFY  (si_table.HasCRC());
    QVERIFY  (si_table.VerifyCRC());
    QCOMPARE (si_table.Length(),              26U);

    // PSIP generic fields
    TimeOffsetTable tot(si_table);
    QCOMPARE (tot.SectionSyntaxIndicator(), false);
    QCOMPARE (tot.SectionLengthRaw(),         26U);

    // TOT specific fields
    QCOMPARE (tot.UTC(),               expectedDT);
    QCOMPARE (tot.UTCUnix(),          1657455832U);

    QCOMPARE (tot.DescriptorsLength(),        15U);
    QCOMPARE (tot.Descriptors(),   tot.data()+10U);

    QDateTime ToCexpectedDT { QDate(2022,04,02), QTime(14,00,00), Qt::UTC };
    LocalTimeOffsetDescriptor lto(tot.Descriptors(), tot.DescriptorsLength());
    QCOMPARE (lto.Count(),                       1U);
    QCOMPARE (lto.CountryCode(0),         0x4E5A4CU);
    QCOMPARE (lto.CountryCodeString(0),       "NZL");
    QCOMPARE (lto.CountryRegionId(0),            0U);
    QCOMPARE (lto.LocalTimeOffsetPolarity(0), false);
    QCOMPARE (lto.LocalTimeOffset(0),          720U);
    QCOMPARE (lto.TimeOfChange(0),    ToCexpectedDT);
    QCOMPARE (lto.TimeOfChangeUnix(0),  1648908000U);
    QCOMPARE (lto.NextTimeOffset(0),           720U);
}

void TestMPEGTables::dvbdate(void)
{
    const std::array<uint8_t,5> dvbdate_data {
        0xdc, 0xa9, 0x12, 0x33, 0x37 /* day 0xdca9, 12:33:37 UTC */
    };

    QCOMPARE (dvbdate2unix (dvbdate_data), (time_t) 1373978017);
    QCOMPARE (dvbdate2qt (dvbdate_data), MythDate::fromString("2013-07-16 12:33:37Z"));
}

void TestMPEGTables::tdt_test(void)
{
    const std::vector<uint8_t> si_data {
        0x70, 0x70, 0x05, 0xdc, 0xa9, 0x12, 0x33, 0x37                                                    /* pp....37 */
    };

    PSIPTable si_table(si_data);

    // test for the values needed for the copy constructor, especially the length
    // QCOMPARE (si_table._pesdataSize, 8); // is protected, so use TSSizeInBuffer() instead
    QCOMPARE (si_table.TSSizeInBuffer(), (unsigned int) 8);

    TimeDateTable tdt(si_table);

    QVERIFY  (tdt.IsGood());

    QVERIFY  (!tdt.HasCRC());
    QVERIFY  (tdt.VerifyCRC());

    for (size_t i=0; i<5; i++) {
        QCOMPARE (si_table.pesdata()[i+3], (uint8_t) si_data[i+3]);
    }

    const unsigned char *dvbDateBuf = tdt.UTCdata();

    for (size_t i=0; i<5; i++) {
        QCOMPARE (dvbDateBuf[i], (uint8_t) si_data[i+3]);
    }

    // actual is 2013-03-30 01:00:00 UTC, 24 hours before the switch to DST in europe
    QCOMPARE (tdt.UTCUnix(), (time_t) 1373978017);
    QCOMPARE (tdt.UTC(), MythDate::fromString("2013-07-16 12:33:37Z"));
}

void TestMPEGTables::ContentIdentifierDescriptor_test(void)
{
    const std::vector<uint8_t> eit_data {
        0x4f, 0xf2, 0x17, 0x42, 0xd8, 0xdb, 0x00, 0x01,  0x00, 0xab, 0x27, 0x0f, 0x01, 0x4f, 0x30, 0x17,  /* O..B......'..O0. */
        0xdc, 0xc9, 0x07, 0x15, 0x00, 0x00, 0x25, 0x00,  0x81, 0xfc, 0x4d, 0xb2, 0x65, 0x6e, 0x67, 0x0d,  /* ......%...M.eng. */
        0x05, 0x4d, 0x6f, 0x6e, 0x65, 0x79, 0x62, 0x72,  0x6f, 0x74, 0x68, 0x65, 0x72, 0xa0, 0x05, 0x44,  /* .Moneybrother..D */
        0x6f, 0x63, 0x75, 0x6d, 0x65, 0x6e, 0x74, 0x61,  0x72, 0x79, 0x20, 0x73, 0x65, 0x72, 0x69, 0x65,  /* ocumentary serie */
        0x73, 0x20, 0x6f, 0x6e, 0x20, 0x53, 0x77, 0x65,  0x64, 0x65, 0x6e, 0x27, 0x73, 0x20, 0x41, 0x6e,  /* s on Sweden's An */
        0x64, 0x65, 0x72, 0x73, 0x20, 0x57, 0x65, 0x6e,  0x64, 0x69, 0x6e, 0x20, 0x61, 0x6e, 0x64, 0x20,  /* ders Wendin and  */
        0x74, 0x68, 0x65, 0x20, 0x6d, 0x61, 0x6b, 0x69,  0x6e, 0x67, 0x20, 0x6f, 0x66, 0x20, 0x68, 0x69,  /* the making of hi */
        0x73, 0x20, 0x6e, 0x65, 0x77, 0x20, 0x61, 0x6c,  0x62, 0x75, 0x6d, 0x2e, 0x20, 0x54, 0x68, 0x69,  /* s new album. Thi */
        0x73, 0x20, 0x65, 0x70, 0x69, 0x73, 0x6f, 0x64,  0x65, 0x20, 0x73, 0x68, 0x6f, 0x77, 0x73, 0x20,  /* s episode shows  */
        0x74, 0x68, 0x65, 0x20, 0x70, 0x72, 0x65, 0x70,  0x61, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20,  /* the preparation  */
        0x69, 0x6e, 0x20, 0x53, 0x74, 0x6f, 0x63, 0x6b,  0x68, 0x6f, 0x6c, 0x6d, 0x20, 0x61, 0x6e, 0x64,  /* in Stockholm and */
        0x20, 0x72, 0x65, 0x63, 0x6f, 0x72, 0x64, 0x69,  0x6e, 0x67, 0x73, 0x20, 0x69, 0x6e, 0x20, 0x43,  /*  recordings in C */
        0x68, 0x69, 0x63, 0x61, 0x67, 0x6f, 0x20, 0x61,  0x6e, 0x64, 0x20, 0x4c, 0x41, 0x2e, 0x4d, 0xc7,  /* hicago and LA.M. */
        0x67, 0x65, 0x72, 0x0d, 0x05, 0x4d, 0x6f, 0x6e,  0x65, 0x79, 0x62, 0x72, 0x6f, 0x74, 0x68, 0x65,  /* ger..Moneybrothe */
        0x72, 0xb5, 0x05, 0x44, 0x69, 0x65, 0x73, 0x65,  0x20, 0x46, 0x6f, 0x6c, 0x67, 0x65, 0x20, 0x64,  /* r..Diese Folge d */
        0x65, 0x73, 0x20, 0x4d, 0x75, 0x73, 0x69, 0x6b,  0x6d, 0x61, 0x67, 0x61, 0x7a, 0x69, 0x6e, 0x73,  /* es Musikmagazins */
        0x20, 0x6d, 0x69, 0x74, 0x20, 0x64, 0x65, 0x6d,  0x20, 0x73, 0x63, 0x68, 0x77, 0x65, 0x64, 0x69,  /*  mit dem schwedi */
        0x73, 0x63, 0x68, 0x65, 0x6e, 0x20, 0x4d, 0x75,  0x73, 0x69, 0x6b, 0x65, 0x72, 0x20, 0x41, 0x6e,  /* schen Musiker An */
        0x64, 0x65, 0x72, 0x73, 0x20, 0x57, 0x65, 0x6e,  0x64, 0x69, 0x6e, 0x20, 0x61, 0x2e, 0x6b, 0x2e,  /* ders Wendin a.k. */
        0x61, 0x2e, 0x20, 0x4d, 0x6f, 0x6e, 0x65, 0x79,  0x62, 0x72, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20,  /* a. Moneybrother  */
        0x7a, 0x65, 0x69, 0x67, 0x74, 0x20, 0x64, 0x69,  0x65, 0x20, 0x56, 0x6f, 0x72, 0x62, 0x65, 0x72,  /* zeigt die Vorber */
        0x65, 0x69, 0x74, 0x75, 0x6e, 0x67, 0x65, 0x6e,  0x20, 0x69, 0x6e, 0x20, 0x53, 0x74, 0x6f, 0x63,  /* eitungen in Stoc */
        0x6b, 0x68, 0x6f, 0x6c, 0x6d, 0x20, 0x75, 0x6e,  0x64, 0x20, 0x64, 0x69, 0x65, 0x20, 0x65, 0x72,  /* kholm und die er */
        0x73, 0x74, 0x65, 0x6e, 0x20, 0x41, 0x75, 0x66,  0x6e, 0x61, 0x68, 0x6d, 0x65, 0x6e, 0x20, 0x43,  /* sten Aufnahmen C */
        0x68, 0x69, 0x63, 0x61, 0x67, 0x6f, 0x20, 0x75,  0x6e, 0x64, 0x20, 0x4c, 0x6f, 0x73, 0x20, 0x41,  /* hicago und Los A */
        0x6e, 0x67, 0x65, 0x6c, 0x65, 0x73, 0x2e, 0x76,  0x73, 0x04, 0x40, 0x65, 0x76, 0x65, 0x6e, 0x74,  /* ngeles.vs.@event */
        0x69, 0x73, 0x2e, 0x6e, 0x6c, 0x2f, 0x30, 0x30,  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30,  /* is.nl/00000000-0 */
        0x30, 0x30, 0x30, 0x2d, 0x31, 0x30, 0x30, 0x30,  0x2d, 0x30, 0x36, 0x30, 0x34, 0x2d, 0x30, 0x30,  /* 000-1000-0604-00 */
        0x30, 0x30, 0x30, 0x30, 0x30, 0x45, 0x30, 0x37,  0x31, 0x31, 0x23, 0x30, 0x30, 0x31, 0x30, 0x33,  /* 00000E0711#00103 */
        0x38, 0x39, 0x39, 0x30, 0x30, 0x30, 0x30, 0x32,  0x30, 0x31, 0x37, 0x08, 0x2f, 0x65, 0x76, 0x65,  /* 89900002017./eve */
        0x6e, 0x74, 0x69, 0x73, 0x2e, 0x6e, 0x6c, 0x2f,  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,  /* ntis.nl/00000000 */
        0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x31, 0x30,  0x30, 0x30, 0x2d, 0x30, 0x36, 0x30, 0x38, 0x2d,  /* -0000-1000-0608- */
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,  0x33, 0x46, 0x39, 0x43, 0x55, 0x04, 0x44, 0x45,  /* 000000003F9CU.DE */
        0x55, 0x00, 0x54, 0x02, 0x23, 0x00, 0x3b, 0xf9,  0x94, 0xa5                                       /* U.T.#.;...       */
    };

    /* pick just the ContentIdentifierDescriptor from the event_information_section */
    DVBContentIdentifierDescriptor descriptor(&eit_data[407]);

    QCOMPARE (descriptor.CRIDCount(),  (size_t) 2);
    /* CRID with IMI appended, the separator and IMI are properly removed from the CRID
     * see ETSI TS 102 323 V1.5.1 page 102
     * The content identifier is composed of the CRID authority "eventis.nl",
     * the separator "/",
     * the CRID data "00000000-0000-1000-0604-0000000E0711",
     * the separator "#", and
     * the IMI "0010389900002017"
     */
    QCOMPARE (descriptor.ContentId(),  QString("eventis.nl/00000000-0000-1000-0604-0000000E0711"));
    /* there is a second content_id in the same descriptor */
    QCOMPARE (descriptor.ContentId(1), QString("eventis.nl/00000000-0000-1000-0608-000000003F9C"));
}

void TestMPEGTables::clone_test(void)
{
    auto *si_data = new unsigned char[8];
    si_data[0] = 0x70; /* pp....37 */
    si_data[1] = 0x70;
    si_data[2] = 0x05;
    si_data[3] = 0xdc;
    si_data[4] = 0xa9;
    si_data[5] = 0x12;
    si_data[6] = 0x33;
    si_data[7] = 0x37;

    const PSIPTable si_table(si_data);

    QVERIFY (!si_table.IsClone());
}

void TestMPEGTables::PrivateDataSpecifierDescriptor_test (void)
{
    /* from https://code.mythtv.org/trac/ticket/12091 */
    const std::vector<uint8_t> si_data {
        0x5f, 0x04, 0x00, 0x00, 0x06, 0x00
    };
    PrivateDataSpecifierDescriptor desc(si_data);
    QCOMPARE (desc.IsValid(), true);
    if (!desc.IsValid())
        return;
    QCOMPARE (desc.PrivateDataSpecifier(), (uint32_t) PrivateDataSpecifierID::UPC1);
}

void TestMPEGTables::PrivateUPCCablecomEpisodetitleDescriptor_test (void)
{
    const std::vector<uint8_t> si_data {
        0xa7, 0x13, 0x67, 0x65, 0x72, 0x05, 0x4b, 0x72,  0x61, 0x6e, 0x6b, 0x20, 0x76, 0x6f, 0x72, 0x20,  /* ..ger.Krank vor  */
        0x4c, 0x69, 0x65, 0x62, 0x65                                                                      /* Liebe            */
    };

    PrivateUPCCablecomEpisodeTitleDescriptor descriptor(si_data);
    QCOMPARE (descriptor.IsValid(), true);
    if (!descriptor.IsValid())
        return;
    QCOMPARE (descriptor.CanonicalLanguageString(), QString("ger"));
    QCOMPARE (descriptor.TextLength(), (uint) 16);
    QCOMPARE (descriptor.Text(), QString("Krank vor Liebe"));
}

void TestMPEGTables::ItemList_test (void)
{
    ShortEventDescriptor descriptor(&eit_data_0000[26]);
    if (!descriptor.IsValid()) {
        QFAIL("The eit_data_0000 descriptor is invalid");
        return;
    }
    QCOMPARE (descriptor.DescriptorTag(), (unsigned int) DescriptorID::short_event);
    QCOMPARE (descriptor.size(), (unsigned int) 194);
    QCOMPARE (descriptor.LanguageString(), QString("ger"));
    QVERIFY  (descriptor.Text().startsWith(QString("Krimiserie. ")));

    ExtendedEventDescriptor descriptor2(&eit_data_0000[26+descriptor.size()]);
    if (!descriptor2.IsValid()) {
        QFAIL("The eit_data_0000 descriptor2 is invalid");
        return;
    }
    QCOMPARE (descriptor2.DescriptorTag(), (unsigned int) DescriptorID::extended_event);
    /* tests for items start here */
    QCOMPARE (descriptor2.LengthOfItems(), (uint) 139);
}

void TestMPEGTables::TestUCS2 (void)
{
    std::array<uint8_t,24> ucs2_data {
        0x17, 0x11, 0x80, 0x06, 0x5e, 0xb7, 0x67, 0x03,  0x54, 0x48, 0x73, 0x7b, 0x00, 0x3a, 0x95, 0x8b,
        0xc3, 0x80, 0x01, 0x53, 0xcb, 0x8a, 0x18, 0xbf
    };

    std::array<wchar_t,12> wchar_data { L"\u8006\u5eb7\u6703\u5448\u737b\u003a\u958b\uc380\u0153\ucb8a\u18bf"};

    QCOMPARE (sizeof (QChar), (size_t) 2);
    QCOMPARE (sizeof (ucs2_data) - 1, (size_t) ucs2_data[0]);
    QString ucs2 = dvb_decode_text (&ucs2_data[1], ucs2_data[0], {});
    QCOMPARE (ucs2.length(), (int) (ucs2_data[0] - 1) / 2);
    QCOMPARE (ucs2, QString::fromWCharArray (wchar_data.data()));
}

void TestMPEGTables::TestISO8859_data (void)
{
    QTest::addColumn<int>("iso");
    QTest::addColumn<QString>("expected");

    QTest::newRow("iso-8859-1") << 1 <<
        QStringLiteral(u" ¡¢£¤¥¦§¨©ª«¬­®¯" \
                        "°±²³´µ¶·¸¹º»¼½¾¿" \
                        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ" \
                        "ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß" \
                        "àáâãäåæçèéêëìíîï" \
                        "ðñòóôõö÷øùúûüýþÿ" );
    QTest::newRow("iso-8859-2") << 2 <<
        QStringLiteral(u" Ą˘Ł¤ĽŚ§¨ŠŞŤŹ­ŽŻ" \
                        "°ą˛ł´ľśˇ¸šşťź˝žż" \
                        "ŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎ" \
                        "ĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢß" \
                        "ŕáâăäĺćçčéęëěíîď" \
                        "đńňóôőö÷řůúűüýţ˙" );
    QTest::newRow("iso-8859-3") << 3 <<
        QStringLiteral(u" Ħ˘£¤�Ĥ§¨İŞĞĴ­�Ż" \
                        "°ħ²³´µĥ·¸ışğĵ½�ż" \
                        "ÀÁÂ�ÄĊĈÇÈÉÊËÌÍÎÏ" \
                        "�ÑÒÓÔĠÖ×ĜÙÚÛÜŬŜß" \
                        "àáâ�äċĉçèéêëìíîï" \
                        "�ñòóôġö÷ĝùúûüŭŝ˙" );
    QTest::newRow("iso-8859-4") << 4 <<
        QStringLiteral(u" ĄĸŖ¤ĨĻ§¨ŠĒĢŦ­Ž¯" \
                        "°ą˛ŗ´ĩļˇ¸šēģŧŊžŋ" \
                        "ĀÁÂÃÄÅÆĮČÉĘËĖÍÎĪ" \
                        "ĐŅŌĶÔÕÖ×ØŲÚÛÜŨŪß" \
                        "āáâãäåæįčéęëėíîī" \
                        "đņōķôõö÷øųúûüũū˙" );
    QTest::newRow("iso-8859-5") << 5 <<
        QStringLiteral(u" ЁЂЃЄЅІЇЈЉЊЋЌ­ЎЏ" \
                        "АБВГДЕЖЗИЙКЛМНОП" \
                        "РСТУФХЦЧШЩЪЫЬЭЮЯ" \
                        "абвгдежзийклмноп" \
                        "рстуфхцчшщъыьэюя" \
                        "№ёђѓєѕіїјљњћќ§ўџ" );
    // iso-8859-6: latin/arabic
    QTest::newRow("iso-8859-7") << 7 <<
        QStringLiteral(u" ‘’£€₯¦§¨©ͺ«¬­�―" \
                        "°±²³΄΅Ά·ΈΉΊ»Ό½ΎΏ" \
                        "ΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟ" \
                        "ΠΡ�ΣΤΥΦΧΨΩΪΫάέήί" \
                        "ΰαβγδεζηθικλμνξο" \
                        "πρςστυφχψωϊϋόύώ�" );
    // iso-8859-6: latin/hebrew
    QTest::newRow("iso-8859-9") << 9 <<
        QStringLiteral(u" ¡¢£¤¥¦§¨©ª«¬­®¯" \
                        "°±²³´µ¶·¸¹º»¼½¾¿" \
                        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ" \
                        "ĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß" \
                        "àáâãäåæçèéêëìíîï" \
                        "ğñòóôõö÷øùúûüışÿ" );
    QTest::newRow("iso-8859-10") << 10 <<
        QStringLiteral(u" ĄĒĢĪĨĶ§ĻĐŠŦŽ­ŪŊ" \
                        "°ąēģīĩķ·ļđšŧž―ūŋ" \
                        "ĀÁÂÃÄÅÆĮČÉĘËĖÍÎÏ" \
                        "ÐŅŌÓÔÕÖŨØŲÚÛÜÝÞß" \
                        "āáâãäåæįčéęëėíîï" \
                        "ðņōóôõöũøųúûüýþĸ" );
    QTest::newRow("iso-8859-11") << 11 <<
        QStringLiteral(u" กขฃคฅฆงจฉชซฌญฎฏ" \
                        "ฐฑฒณดตถทธนบปผฝพฟ" \
                        "ภมยรฤลฦวศษสหฬอฮฯ" \
                        "ะัาำิีึืฺุู����฿" \
                        "เแโใไๅๆ็่้๊๋์ํ๎๏" \
                        "๐๑๒๓๔๕๖๗๘๙๚๛����" );
    // iso-8859-12 was abandoned
    QTest::newRow("iso-8859-13") << 13 <<
        QStringLiteral(u" ”¢£¤„¦§Ø©Ŗ«¬­®Æ" \
                        "°±²³“µ¶·ø¹ŗ»¼½¾æ" \
                        "ĄĮĀĆÄÅĘĒČÉŹĖĢĶĪĻ" \
                        "ŠŃŅÓŌÕÖ×ŲŁŚŪÜŻŽß" \
                        "ąįāćäåęēčéźėģķīļ" \
                        "šńņóōõö÷ųłśūüżž’" );
    QTest::newRow("iso-8859-14") << 14 <<
        QStringLiteral(u" Ḃḃ£ĊċḊ§Ẁ©ẂḋỲ­®Ÿ" \
                        "ḞḟĠġṀṁ¶ṖẁṗẃṠỳẄẅṡ" \
                        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ" \
                        "ŴÑÒÓÔÕÖṪØÙÚÛÜÝŶß" \
                        "àáâãäåæçèéêëìíîï" \
                        "ŵñòóôõöṫøùúûüýŷÿ" );
    QTest::newRow("iso-8859-15") << 15 <<
        QStringLiteral(u" ¡¢£€¥Š§š©ª«¬­®¯" \
                        "°±²³Žµ¶·ž¹º»ŒœŸ¿" \
                        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ" \
                        "ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß" \
                        "àáâãäåæçèéêëìíîï" \
                        "ðñòóôõö÷øùúûüýþÿ" );
}

void TestMPEGTables::TestISO8859 (void)
{
    QFETCH(int, iso);
    QFETCH(QString, expected);

    high8[2] = iso;
    QString actual = dvb_decode_text(high8.data(), high8.size());
    QCOMPARE (actual, expected);
}

void TestMPEGTables::ParentalRatingDescriptor_test (void)
{
    /* from https://forum.mythtv.org/viewtopic.php?p=4376 / #12553 */
    const std::vector<uint8_t> si_data {
        0x55, 0x04, 0x47, 0x42, 0x52, 0x0B
    };
    ParentalRatingDescriptor desc(si_data);
    QCOMPARE (desc.IsValid(), true);
    if (!desc.IsValid())
        return;
    QCOMPARE (desc.Count(), 1U);
    QCOMPARE (desc.CountryCodeString(0), QString("GBR"));
    QCOMPARE (desc.Rating(0), 14);
}

void TestMPEGTables::ExtendedEventDescriptor_test (void)
{
    ExtendedEventDescriptor desc(&eit_data_0000[16*13+12]);
    if (!desc.IsValid()) {
        QFAIL("The eit_data_0000 descriptor is invalid");
        return;
    }
    QCOMPARE (desc.LengthOfItems(), 139U);
    QMultiMap<QString,QString> items = desc.Items();
    QCOMPARE (items.count(), 5);
    QVERIFY (items.contains (QString ("Role Player")));
    QCOMPARE (items.count (QString ("Role Player")), 5);
    QCOMPARE (items.count (QString ("Role Player"), QString ("Nathan Fillion")), 1);
}

void TestMPEGTables::OTAChannelName_test (void)
{
    /* manually crafted according to A65/2013
     * http://atsc.org/wp-content/uploads/2015/03/Program-System-Information-Protocol-for-Terrestrial-Broadcast-and-Cable.pdf
     */
    const std::vector<uint8_t> tvct_data {
        0xc8, 0xf0, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x01, 0x00,  'A', 0x00,  'B', 0x00,  'C',
        0x00,  'D', 0x00,  'E', 0x00,  'F', 0x00, '\0',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x60, 0xc4, 0x82, 0xb9
    };

    PSIPTable psip = PSIPTable(tvct_data.data());
    TerrestrialVirtualChannelTable table(psip);

    QVERIFY (table.HasCRC());
    QCOMPARE (table.CalcCRC(), table.CRC());
    QVERIFY (table.VerifyCRC());

    QCOMPARE (table.SectionLength(), static_cast<uint>(tvct_data.size()));

    QCOMPARE (table.ChannelCount(), 1U);
    QCOMPARE (table.ShortChannelName(0), QString("ABCDEF"));
    QCOMPARE (table.ShortChannelName(1), QString());

    PSIPTable psip2 = PSIPTable(tvct_data_0000.data());
    TerrestrialVirtualChannelTable tvct(psip2);
    QVERIFY (tvct.VerifyCRC());
    QVERIFY (tvct.VerifyPSIP(false));

    QCOMPARE (tvct.ChannelCount(), 6U);
    /*
     * ShortChannelName is fixed width 7-8 characters.
     * A65/2013 says to fill trailing characters with \0
     * but here the space is used
     */
    QCOMPARE (tvct.ShortChannelName(0), QString("KYNM-HD"));
    QCOMPARE (tvct.ShortChannelName(1), QString("TUFF-TV"));
    QCOMPARE (tvct.ShortChannelName(2), QString("Retro"));
    QCOMPARE (tvct.ShortChannelName(3), QString("REV'N"));
    QCOMPARE (tvct.ShortChannelName(4), QString("QVC"));
    QCOMPARE (tvct.ShortChannelName(5), QString("Antenna"));
    QCOMPARE (tvct.ShortChannelName(6), QString(""));
    QCOMPARE (tvct.ShortChannelName(999), QString());
    /*
     * ExtendedChannelName has a \0 terminated string inside
     * strings with length. That's uncommon.
     */
    QCOMPARE (tvct.GetExtendedChannelName(0), QString());
    QCOMPARE (tvct.GetExtendedChannelName(1), QString("KYNM TUFF-T"));
    QCOMPARE (tvct.GetExtendedChannelName(2), QString("KYNM Albuquerque, N"));
    QCOMPARE (tvct.GetExtendedChannelName(3), QString("KYNM PBJ-T"));
    QCOMPARE (tvct.GetExtendedChannelName(4), QString("KYNM QV"));
    QCOMPARE (tvct.GetExtendedChannelName(5), QString());
    QCOMPARE (tvct.GetExtendedChannelName(6), QString());
    QCOMPARE (tvct.GetExtendedChannelName(999), QString());
}

void TestMPEGTables::atsc_huffman_test_data (void)
{
    QTest::addColumn<QString>("encoding");
    QTest::addColumn<QByteArray>("compressed");
    QTest::addColumn<QString>("e_uncompressed");

    // This is the only example I could find online.
    const std::array<uint8_t,5> example1
        {0b01000011, 0b00101000, 0b11011100, 0b10000100, 0b11010100};
    QTest::newRow("Title")
        << "C5"
        << QByteArray((char *)example1.data(), example1.size())
        << "The next";

    // M = 1010, y = 011, t = 1101001, h = 111, 27 = 1110001,
    // T = 01010100, V = 111100, ' ' = 01010, i = 010010, s = 0011,
    // ' ' = 10, c = 01000000, o = 1001, o = 0011, l = 0100,
    // 27 = 0111001, '!' = 00100001, END = 1
    const std::array<uint8_t,12> example2
        { 0b10100111, 0b10100111, 0b11110001, 0b01010100,
          0b11110001, 0b01001001, 0b00111010, 0b10000001,
          0b00100110, 0b10001110, 0b01001000, 0b01100000};
    QTest::newRow("myth title")
        << "C5"
        << QByteArray((char *)example2.data(), example2.size())
        << "MythTV is cool!";

    // M = 1111, 27 = 11010, y = 0111 1001, 27 = 01010, t = 0111 0100,
    // h = 00, 27 = 1011100, T = 0101 0100, V = 1000, ' ' = 10,
    // i = 10101, s = 101, ' ' = 0, c = 10011, o = 101, o = 10100,
    // l = 0101, '.' = 00100, END = 1
    const std::array<uint8_t,11> example3
        { 0b11111101, 0b00111100, 0b10101001, 0b11010000,
          0b10111000, 0b10101001, 0b00010101, 0b01101010,
          0b01110110, 0b10001010, 0b01001000};
    QTest::newRow("myth descr")
        << "C7"
        << QByteArray((char *)example3.data(), example3.size())
        << "MythTV is cool.";
}

void TestMPEGTables::atsc_huffman_test (void)
{
    QFETCH(QString,    encoding);
    QFETCH(QByteArray, compressed);
    QFETCH(QString,    e_uncompressed);

    QString uncompressed {};
    if (encoding == "C5") {
        uncompressed = atsc_huffman1_to_string((uchar *)compressed.data(),
                                               compressed.size(), 1);
    } else if (encoding == "C7") {
        uncompressed = atsc_huffman1_to_string((uchar *)compressed.data(),
                                               compressed.size(), 2);
    }
    QCOMPARE(uncompressed.trimmed(), e_uncompressed);
}

QTEST_APPLESS_MAIN(TestMPEGTables)

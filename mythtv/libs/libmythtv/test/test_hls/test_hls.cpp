/*
 *  Class TestHls
 *
 *  Copyright (c) David Hampton 2025
 *
 * See the file LICENSE for licensing information.
 */
#include "test_hls.h"

#include "libmythtv/HLS/httplivestreambuffer.h"

// Examples from draft-pantos-http-live-streaming-04

const QByteArray example1 =
    "#EXTM3U\n" \
    "#EXT-X-TARGETDURATION:5220\n" \
    "#EXTINF:5220,\n" \
    "http://media.example.com/entire.ts\n" \
    "#EXT-X-ENDLIST\n";

const QByteArray example2 =
    "#EXTM3U\n" \
    "#EXT-X-TARGETDURATION:8\n" \
    "#EXT-X-MEDIA-SEQUENCE:2680\n" \
    "\n" \
    "#EXTINF:8,\n" \
    "https://priv.example.com/fileSequence2680.ts\n" \
    "#EXTINF:8,\n" \
    "https://priv.example.com/fileSequence2681.ts\n" \
    "#EXTINF:8,\n" \
    "https://priv.example.com/fileSequence2682.ts";

const QByteArray example3 =
    "#EXTM3U\n" \
    "#EXT-X-MEDIA-SEQUENCE:7794\n" \
    "#EXT-X-TARGETDURATION:15\n" \
    "\n" \
    "#EXT-X-KEY:METHOD=AES-128,URI=\"https://priv.example.com/key.php?r=52\"\n" \
    "\n" \
    "#EXTINF:15,\n" \
    "http://media.example.com/fileSequence52-1.ts\n" \
    "#EXTINF:15,\n" \
    "http://media.example.com/fileSequence52-2.ts\n" \
    "#EXTINF:15,\n" \
    "http://media.example.com/fileSequence52-3.ts\n" \
    "\n" \
    "#EXT-X-KEY:METHOD=AES-128,URI=\"https://priv.example.com/key.php?r=53\"\n" \
    "\n" \
    "#EXTINF:15,\n" \
    "http://media.example.com/fileSequence53-1.ts\n";

#ifdef REQUIRES_NETWORK_ACCESS
const QByteArray example4 =
    "#EXTM3U\n" \
    "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=1280000\n" \
    "http://example.com/low.m3u8\n" \
    "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=2560000\n" \
    "http://example.com/mid.m3u8\n" \
    "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=7680000\n" \
    "http://example.com/hi.m3u8\n" \
    "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=65000,CODECS=\"mp4a.40.5\"\n" \
    "http://example.com/audio-only.m3u8";
#endif

void TestHls::initTestCase(void)
{
}

void TestHls::test_parsem3u8(void)
{
    auto *rb = new HLSRingBuffer("/tmp/dumy", false);

    int res = rb->ParseM3U8(&example1);
    QCOMPARE(res, 0);

    res = rb->ParseM3U8(&example2);
    QCOMPARE(res, 0);

    res = rb->ParseM3U8(&example3);
    QCOMPARE(res, 0);

#ifdef REQUIRES_NETWORK_ACCESS
    res = rb->ParseM3U8(&example4);
    QCOMPARE(res, 0);
#endif
}

void TestHls::cleanupTestCase()
{
}

QTEST_APPLESS_MAIN(TestHls)

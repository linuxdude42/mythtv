
#include "mythcoreutil.h"

// POSIX
#include <unistd.h>
#include <fcntl.h>

// System specific C headers
#include "compat.h"

#include "zlib.h"
#undef Z_NULL
#define Z_NULL nullptr
#include "zip.h"

#ifdef linux
#include <sys/vfs.h>
#include <sys/sysinfo.h>
#endif

#if CONFIG_DARWIN
#include <mach/mach.h>
#endif

#ifdef BSD
#include <sys/mount.h>  // for struct statfs
#include <sys/sysctl.h>
#endif

// Qt headers
#include <QByteArray>
#include <QStringList>
#include <QDir>
#include <QFile>

// libmythbase headers
#include "mythcorecontext.h"
#include "mythdate.h"
#include "mythlogging.h"

#include "version.h"
#include "mythversion.h"

#define ZIP_ATTR_FILE_TYPE_MASK      0xFE000000
#define   ZIP_ATTR_FILE_TYPE_SYMLINK 0xA0000000
#define   ZIP_ATTR_FILE_TYPE_NORMAL  0x80000000
#define ZIP_ATTR_USER_PERM_MASK      0x01C00000
#define ZIP_ATTR_GROUP_PERM_MASK     0x03800000
#define ZIP_ATTR_OTHER_PERM_MASK     0x00700000
#define ZIP_ATTR_USER_PERM_SHIFT     22
#define ZIP_ATTR_GROUP_PERM_SHIFT    19
#define ZIP_ATTR_OTHER_PERM_SHIFT    16

/** \fn getDiskSpace(const QString&,long long&,long long&)
 *  \brief Returns free space on disk containing file in KiB,
 *          or -1 if it does not succeed.
 *  \param file_on_disk file on the file system we wish to stat.
 */
int64_t getDiskSpace(const QString &file_on_disk,
                     int64_t &total, int64_t &used)
{
    struct statfs statbuf {};
    int64_t freespace = -1;
    QByteArray cstr = file_on_disk.toLocal8Bit();

    total = used = -1;

    // there are cases where statfs will return 0 (good), but f_blocks and
    // others are invalid and set to 0 (such as when an automounted directory
    // is not mounted but still visible because --ghost was used),
    // so check to make sure we can have a total size > 0
    if ((statfs(cstr.constData(), &statbuf) == 0) &&
        (statbuf.f_blocks > 0) &&
        (statbuf.f_bsize > 0))
    {
        total      = statbuf.f_blocks;
        total     *= statbuf.f_bsize;
        total      = total >> 10;

        freespace  = statbuf.f_bavail;
        freespace *= statbuf.f_bsize;
        freespace  = freespace >> 10;

        used       = total - freespace;
    }

    return freespace;
}

// NOLINTNEXTLINE(readability-uppercase-literal-suffix)
static constexpr uint64_t STATS_REQUIRED {
    ZIP_STAT_NAME | ZIP_STAT_INDEX | ZIP_STAT_SIZE |
    ZIP_STAT_MTIME | ZIP_STAT_ENCRYPTION_METHOD};

bool zipCreateDirectory(const QFileInfo & fi);
bool zipCreateDirectory(const QFileInfo & fi)
{
    QDir dir = fi.absoluteDir();
    if (dir.exists())
        return true;
    if (dir.mkpath(dir.absolutePath()))
        return true;
    LOG(VB_GENERAL, LOG_ERR,
        QString("extractZIP(): Failed to create directory %1")
        .arg(dir.absolutePath()));
    return false;
}

bool zipValidateFilename(const QFileInfo& fi, const QString& outDirAbsPath);
bool zipValidateFilename(const QFileInfo& fi, const QString& outDirAbsPath)
{
    if (fi.absoluteFilePath().startsWith(outDirAbsPath))
        return true;
    LOG(VB_GENERAL, LOG_ERR,
        QString("extractZIP(): Attempt to write outside destination directory. File: %1")
        .arg(QString(fi.fileName())));
    return false;
}

void zipSetFileAttributes(QFile& outfile, const zip_stat_t& stats, zip_uint32_t attributes);
void zipSetFileAttributes(QFile& outfile, const zip_stat_t& stats, zip_uint32_t attributes)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    // Set times
    auto dateTime = MythDate::fromSecsSinceEpoch(stats.mtime);

    outfile.setFileTime(dateTime, QFileDevice::FileAccessTime);
    outfile.setFileTime(dateTime, QFileDevice::FileBirthTime);
    outfile.setFileTime(dateTime, QFileDevice::FileMetadataChangeTime);
    outfile.setFileTime(dateTime, QFileDevice::FileModificationTime);
#endif

    if (attributes == 0)
        return;
    int32_t user  = (attributes & ZIP_ATTR_USER_PERM_MASK)  >> ZIP_ATTR_USER_PERM_SHIFT;
    int32_t group = (attributes & ZIP_ATTR_GROUP_PERM_MASK) >> ZIP_ATTR_GROUP_PERM_SHIFT;
    int32_t other = (attributes & ZIP_ATTR_OTHER_PERM_MASK) >> ZIP_ATTR_OTHER_PERM_SHIFT;
    int32_t qt_perms = (user << 12) | (user < 8) | (group < 4) | other;
    outfile.setPermissions(static_cast<QFileDevice::Permission>(qt_perms));
}

bool zipCreateSymlink(zip_t* zip, zip_stat_t& stats, const QFileInfo& fi, const QString& outDirAbsPath);
bool zipCreateSymlink(zip_t* zip, zip_stat_t& stats, const QFileInfo& fi, const QString& outDirAbsPath)
{
    zip_file_t *infile = zip_fopen_index(zip, stats.index, 0);
    if (infile == nullptr)
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Unable to open file %1 in %2")
            .arg(stats.index).arg("abcdefg"));
        return false;
    }

    int64_t  readLen      {0};
    static constexpr int BLOCK_SIZE { 4096 };
    QByteArray data; data.resize(BLOCK_SIZE);
    readLen = zip_fread(infile, data.data(), BLOCK_SIZE);
    if (readLen < 1)
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Symlink name for file %1 is too short")
            .arg(stats.index));
        return false;
    }
    data.resize(readLen);

    QFileInfo target = fi.absolutePath() + "/" + data;
    if (!zipValidateFilename(target, outDirAbsPath))
        return false;
    if (!QFile::link(target.absoluteFilePath(), fi.absoluteFilePath()))
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Failed to create link from %1 pointing to %2")
            .arg(fi.absoluteFilePath())
            .arg(target.absoluteFilePath()));
        return false;
    }
    return true;
}

bool zipWriteOneFile(zip_t* zip, zip_stat_t& stats, const QFileInfo& fi, zip_uint32_t attributes);
bool zipWriteOneFile(zip_t* zip, zip_stat_t& stats, const QFileInfo& fi, zip_uint32_t attributes)
{
    zip_file_t *infile = zip_fopen_index(zip, stats.index, 0);
    if (infile == nullptr)
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Unable to open file %1 in %2")
            .arg(stats.index).arg("abcdefg"));
        return false;
    }

    auto outfile = QFile(fi.absoluteFilePath());
    if (!outfile.open(QIODevice::Truncate|QIODevice::WriteOnly))
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Failed to open output file %1")
            .arg(fi.absoluteFilePath()));
        return false;
    }

    int64_t  readLen      {0};
    uint64_t bytesRead    {0};
    uint64_t bytesWritten {0};
    static constexpr int BLOCK_SIZE { 4096 };
    QByteArray data; data.resize(BLOCK_SIZE);
    while ((readLen = zip_fread(infile, data.data(), BLOCK_SIZE)) > 0)
    {
        bytesRead += readLen;
        int64_t writeLen = outfile.write(data.data(), readLen);
        if (writeLen < 0)
        {
            LOG(VB_GENERAL, LOG_ERR,
                QString("extractZIP(): Failed to write %1/%2 bytes to output file %3")
                .arg(writeLen).arg(readLen).arg(fi.absoluteFilePath()));
            return false;
        }
        bytesWritten += writeLen;
    }

    if ((stats.size != bytesRead) ||
        (stats.size != bytesWritten))
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Failed to copy file %1. Read %2 and wrote %3 of %4.")
            .arg(fi.fileName()).arg(bytesRead).arg(bytesWritten)
            .arg(stats.size));
        return false;
    }

    outfile.flush();
    zipSetFileAttributes(outfile, stats, attributes);
    outfile.close();
    if (zip_fclose(infile) == -1)
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Failed to close current file %1")
            .arg(fi.fileName()));
        return false;
    }

    return true;
}

bool extractZIP(const QString &zipFileName, const QString &outDirName)
{
    auto outDir = QDir(outDirName);
    if (!outDir.exists())
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Target directory %1 doesn't exist")
            .arg(outDirName));
        return false;
    }
    auto outDirAbsPath = outDir.absolutePath();

    int err { ZIP_ER_OK };
    zip_t *zip = zip_open(qPrintable(zipFileName), 0, &err);
    if (zip == nullptr)
    {
        LOG(VB_GENERAL, LOG_ERR,
                QString("extractZIP(): Unable to open ZIP file %1")
                        .arg(zipFileName));
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
    if (numEntries == -1)
    {
        LOG(VB_GENERAL, LOG_ERR,
                QString("extractZIP(): Zip archive %1 is empty")
                        .arg(zipFileName));
        return false;
    }

    bool ok { true };
    for (int index = 0; ok && (index < numEntries); index++)
    {
        zip_stat_t stats;
        zip_stat_init(&stats);
        if (-1 == zip_stat_index(zip, index, 0, &stats))
        {
            LOG(VB_GENERAL, LOG_ERR,
                QString("extractZIP(): Unable to get info for file %1 in %2")
                .arg(index).arg(zipFileName));
            return false;
        }
        if ((stats.valid & STATS_REQUIRED) != STATS_REQUIRED)
        {
            LOG(VB_GENERAL, LOG_ERR,
                QString("extractZIP(): Invalid status information for file %1 in %2")
                .arg(index).arg(zipFileName));
            return false;
        }
        if (stats.encryption_method > 0)
        {
            LOG(VB_GENERAL, LOG_WARNING,
                QString("extractZIP(): Skipping encryped file %1 in %2")
                .arg(index).arg(zipFileName));
            continue;
        }

        zip_uint8_t  opsys      {ZIP_OPSYS_UNIX};
        zip_uint32_t attributes {0};
        zip_file_get_external_attributes(zip, index, ZIP_FL_UNCHANGED, &opsys, &attributes);

        auto fi = QFileInfo(outDirName + '/' + stats.name);
        ok = zipValidateFilename(fi, outDirAbsPath);
        if (ok)
            ok = zipCreateDirectory(fi);
        if (ok && (stats.size > 0))
        {
            switch (attributes & ZIP_ATTR_FILE_TYPE_MASK)
            {
              case ZIP_ATTR_FILE_TYPE_SYMLINK:
                  ok = zipCreateSymlink(zip, stats, fi, outDirAbsPath);
                break;
              default:
                  ok = zipWriteOneFile(zip, stats, fi, attributes);
                break;
            }
        }
    }

    if (zip_close(zip) == -1)
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("extractZIP(): Error closing ZIP file %1")
            .arg(zipFileName));
        return false;
    }
    return ok;
}

bool gzipFile(const QString &inFilename, const QString &gzipFilename)
{
    QFile infile(inFilename);
    QFile outfile(gzipFilename);

    if (!infile.open(QIODevice::ReadOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gzipFile(): Error opening file for reading '%1'").arg(inFilename));
        return false;
    }

    if (!outfile.open(QIODevice::WriteOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gzipFile(): Error opening file for writing '%1'").arg(gzipFilename));
        infile.close();
        return false;
    }

    QByteArray uncompressedData = infile.readAll();
    QByteArray compressedData = gzipCompress(uncompressedData);

    if (!outfile.write(compressedData))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gzipFile(): Error while writing to '%1'").arg(gzipFilename));
        infile.close();
        outfile.close();
        return false;
    }

    infile.close();
    outfile.close();

    return true;
}

bool gunzipFile(const QString &gzipFilename, const QString &outFilename)
{
    QFile infile(gzipFilename);
    QFile outfile(outFilename);

    if (!infile.open(QIODevice::ReadOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gunzipFile(): Error opening file for reading '%1'").arg(gzipFilename));
        return false;
    }

    if (!outfile.open(QIODevice::WriteOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gunzipFile(): Error opening file for writing '%1'").arg(outFilename));
        infile.close();
        return false;
    }

    QByteArray compressedData = infile.readAll();
    QByteArray uncompressedData = gzipUncompress(compressedData);

    if (outfile.write(uncompressedData) < uncompressedData.size())
    {
        LOG(VB_GENERAL, LOG_ERR, QString("gunzipFile(): Error while writing to '%1'").arg(outFilename));
        infile.close();
        outfile.close();
        return false;
    }

    infile.close();
    outfile.close();

    return true;
}

QByteArray gzipCompress(const QByteArray& data)
{
    if (data.length() == 0)
        return QByteArray();

    std::array <char,1024> out {};

    // allocate inflate state
    z_stream strm;

    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = data.length();
    strm.next_in  = (Bytef*)(data.data());

    int ret = deflateInit2(&strm,
                           Z_DEFAULT_COMPRESSION,
                           Z_DEFLATED,
                           15 + 16,
                           8,
                           Z_DEFAULT_STRATEGY ); // gzip encoding
    if (ret != Z_OK)
        return QByteArray();

    QByteArray result;

    // run deflate()
    do
    {
        strm.avail_out = out.size();
        strm.next_out  = (Bytef*)(out.data());

        ret = deflate(&strm, Z_FINISH);

        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret)
        {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)deflateEnd(&strm);
                return QByteArray();
        }

        result.append(out.data(), out.size() - strm.avail_out);
    }
    while (strm.avail_out == 0);

    // clean up and return

    deflateEnd(&strm);

    return result;
}

QByteArray gzipUncompress(const QByteArray &data)
{
    if (data.length() == 0)
        return QByteArray();

    std::array<char,1024> out {};

    // allocate inflate state
    z_stream strm;
    strm.total_in = 0;
    strm.total_out = 0;
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = data.length();
    strm.next_in  = (Bytef*)(data.data());

    int ret = inflateInit2(&strm, 15 + 16);

    if (ret != Z_OK)
        return QByteArray();

    QByteArray result;

    do
    {
        strm.avail_out = out.size();
        strm.next_out = (Bytef*)out.data();
        ret = inflate(&strm, Z_NO_FLUSH);

        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret)
        {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void) deflateEnd(&strm);
                return QByteArray();
        }

        result.append(out.data(), out.size() - strm.avail_out);
    }
    while (strm.avail_out == 0);

    (void) inflateEnd(& strm);

    return result;
}

static QString downloadRemoteFile(const QString &cmd, const QString &url,
                                  const QString &storageGroup,
                                  const QString &filename)
{
    QStringList strlist(cmd);
    strlist << url;
    strlist << storageGroup;
    strlist << filename;

    bool ok = gCoreContext->SendReceiveStringList(strlist);

    if (!ok || strlist.size() < 2 || strlist[0] != "OK")
    {
        LOG(VB_GENERAL, LOG_ERR,
            "downloadRemoteFile(): " + cmd + " returned ERROR!");
        return QString();
    }

    return strlist[1];
}

QString RemoteDownloadFile(const QString &url,
                           const QString &storageGroup,
                           const QString &filename)
{
    return downloadRemoteFile("DOWNLOAD_FILE", url, storageGroup, filename);
}

QString RemoteDownloadFileNow(const QString &url,
                              const QString &storageGroup,
                              const QString &filename)
{
    return downloadRemoteFile("DOWNLOAD_FILE_NOW", url, storageGroup, filename);
}

const char *GetMythSourceVersion()
{
    return MYTH_SOURCE_VERSION;
}

const char *GetMythSourcePath()
{
    return MYTH_SOURCE_PATH;
}

/* vim: set expandtab tabstop=4 shiftwidth=4: */

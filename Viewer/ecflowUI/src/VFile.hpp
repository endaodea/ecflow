//============================================================================
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#ifndef VFILE_INC__
#define VFILE_INC__

#include <memory>
#include <string>
#include <vector>

#include <QDateTime>

class VFile;
typedef std::shared_ptr<VFile> VFile_ptr;

class VFile : public std::enable_shared_from_this<VFile> {
public:
    virtual ~VFile();

    enum StorageMode { MemoryStorage, DiskStorage };
    enum FetchMode { NoFetchMode, LocalFetchMode, ServerFetchMode, LogServerFetchMode, TransferFetchMode };

    const std::string& path() const { return path_; }
    const std::string& sourcePath() const { return sourcePath_; }
    void setSourcePath(const std::string& p) { sourcePath_ = p; }
    unsigned int sourceModTime() const { return sourceModTime_; }
    void setSourceModTime(unsigned int t) { sourceModTime_ = t; }
    const std::string& sourceCheckSum() const { return sourceCheckSum_; }
    void setSourceCheckSum(const std::string& c) { sourceCheckSum_ = c; }

    void setContents(const std::string);
    bool exists() const;
    bool isEmpty() const;

    StorageMode storageMode() const { return storageMode_; }
    void setStorageMode(StorageMode);

    static size_t maxDataSize() { return maxDataSize_; }
    size_t dataSize() const { return dataSize_; }
    const char* data() const { return data_; }

    size_t fileSize() const;
    size_t sizeInBytes() const;

    void setTransferDuration(unsigned int d) { transferDuration_ = d; }
    unsigned int transferDuration() const { return transferDuration_; }
    void setFetchDate(QDateTime d) { fetchDate_ = d; }
    QDateTime fetchDate() const { return fetchDate_; }
    void setFetchMode(FetchMode m) { fetchMode_ = m; }
    FetchMode fetchMode() const { return fetchMode_; }
    void setFetchModeStr(const std::string& fetchMethod) { fetchModeStr_ = fetchMethod; }
    const std::string& fetchModeStr() const { return fetchModeStr_; }
    int truncatedTo() const { return truncatedTo_; }
    void setTruncatedTo(int t) { truncatedTo_ = t; }
    void setCached(bool b) { cached_ = b; }
    bool cached() const { return cached_; }
    void setLog(const std::vector<std::string>& log) { log_ = log; }
    void addToLog(const std::string& s) { log_.push_back(s); }
    const std::vector<std::string>& log() const { return log_; }
    int numberOfLines() const;

    bool hasDeltaContents() const { return deltaContents_; }
    void setDeltaContents(bool b) { deltaContents_ = b; }

    bool append(VFile_ptr);
    bool write(const char* buf, size_t len, std::string& err);
    bool write(const std::string& buf, std::string& err);

    void close();
    void print();

    static VFile_ptr create(const std::string& path, const std::string& contents, bool deleteFile = true);
    static VFile_ptr create(const std::string& path, bool deleteFile = true);
    static VFile_ptr create(bool deleteFile = true);
    static VFile_ptr createTmpFile(bool deleteFile = true);

protected:
    VFile(const std::string& name, const std::string& str, bool deleteFile = true);
    explicit VFile(const std::string& str, bool deleteFile = true);
    explicit VFile(bool deleteFile = true);
    bool appendContentsTo(FILE* fpTarget) const;

    std::string path_;
    std::string sourcePath_;
    unsigned int sourceModTime_{0};
    std::string sourceCheckSum_;
    bool deleteFile_;

    StorageMode storageMode_{MemoryStorage};
    static const size_t maxDataSize_;
    char* data_{nullptr};
    size_t dataSize_{0};
    FILE* fp_{nullptr};
    bool deltaContents_{false};

    FetchMode fetchMode_{NoFetchMode};
    std::string fetchModeStr_;
    QDateTime fetchDate_;
    unsigned int transferDuration_{0};
    int truncatedTo_{0};

    bool cached_{false};
    std::vector<std::string> log_;
};

#endif

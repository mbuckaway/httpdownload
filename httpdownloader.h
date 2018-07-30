//********************************************************************
//    created:    2018-07-25 10:57 PM
//    file:       httpdownloader.h
//    author:     Mark Buckaway
//    purpose:    Header for Downloader
//
//*********************************************************************


#ifndef HTTPDOWNLOAD_HTTPDOWNLOADER_H
#define HTTPDOWNLOAD_HTTPDOWNLOADER_H

#include <string>
#include "Poco/URI.h"

namespace HTTPDownload
{
    /// \brief HTTPDownloader class does the heavy lifting of opening the http connection
    /// and downloads the file. It has the ability to download the file as a whole, or in 1MiB
    /// chunks or blocks. It is the responsibility of the caller to re-assemble the chunks.
    class HTTPDownloader
    {
    public:
        /// \brief Constructor
        /// \param uri Fully qualifed URL
        /// \param outfileFilename Name of the file to save the download, or use the URL to get the filename
        /// if not specified
        /// \param verbose be verbose or quiet
        /// \param chunkSize Size of the chunks to download in bytes
        HTTPDownloader(const Poco::URI &uri, const std::string& outfileFilename, bool verbose, long chunkSize) :
                    _uri(uri)
                    , _outputFilename(outfileFilename)
                    ,_verbose(verbose)
                    ,_chunkSize(chunkSize)
                    , _fileSize(0L)
                    {}

        /// \brief Check the remote file for existance and file size
        /// Also verify the command line options won't download invalid number of chunks
        /// \param numberChunks Number of chunks requested
        /// \param chunkSize Size of the chunks
        void CheckRemoteFile(int numberChunks);

        /// \brief Main download routine. Connects to the remove host and downloads the file
        /// to the current path
        /// \param Chunk number (1 MiB block) or zero for the entire file (0 is default)
        /// \throws BadParameterException if the URL is not fully qualified
        /// \throws FailedDownloadException If there is a download error
        void Download(int chunkNumber = 0);

        /// \brief Log a message to the user if verbose is enabled
        void sendMessage(const std::string& message);

        virtual ~HTTPDownloader() {}

        /// \brief Getting for FileURL
        const Poco::URI& getFileURL() { return _uri; }

    private:
        Poco::URI _uri;
        bool _verbose;
        std::string _outputFilename;
        long _chunkSize;
        long _fileSize;
    };

}
#endif //HTTPDOWNLOAD_HTTPDOWNLOADER_H

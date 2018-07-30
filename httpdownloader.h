//********************************************************************
//    created:    2018-07-25 10:57 PM
//    file:       httpdownloader.h
//    author:     Mark Buckaway
//    purpose:
//
//*********************************************************************


#ifndef HTTPDOWNLOAD_HTTPDOWNLOADER_H
#define HTTPDOWNLOAD_HTTPDOWNLOADER_H

#include <string>
#include "Poco/URI.h"

namespace HTTPDownload
{
    /// \brief HTTPDownloader class does the heavy lifting of opening the http connection
    /// and downloads the file
    class HTTPDownloader
    {
    public:
        /// \brief Constructor
        /// \param fileURL Fully qualifed URL
        HTTPDownloader(const Poco::URI &uri, bool verbose) :
                    _uri(uri)
                    ,_verbose(verbose)
                    {}

        /// \brief Main download routine. Connects to the remove host and downloads the file
        /// to the current path
        /// \throws BadParameterException if the URL is not fully qualified
        void Download(int chunkNumber, const std::string& filename);

        /// \brief Log a message to the user if verbose is enabled
        void sendMessage(const std::string& message);

        virtual ~HTTPDownloader() {}

        /// \brief Getting for FileURL
        const Poco::URI& getFileURL() { return _uri; }

    private:
        Poco::URI _uri;
        bool _verbose;

        static long const MiB = 1024*1024;
    };

}
#endif //HTTPDOWNLOAD_HTTPDOWNLOADER_H

//********************************************************************
//    created:    2018-07-25 10:57 PM
//    file:       httpdownloader.cpp
//    author:     Mark Buckaway
//    purpose:    Download a file from HTTP as a whole or in chunks
//
//*********************************************************************


#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/Format.h"
#include "Poco/NumberParser.h"
#include "httpdownloader.h"
#include "httpdownloadexceptions.h"


namespace HTTPDownload
{

    void HTTPDownloader::CheckRemoteFile(int numberChunks)
    {
        std::string rangeType = "";

        try
        {

            Poco::Net::HTTPClientSession session(_uri.getHost(), _uri.getPort());
            std::string path(_uri.getPathAndQuery());
            if (path.empty()) path = "/";

            // Use the HEAD request to find out if ranges are support and to get our file size
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_HEAD, path, Poco::Net::HTTPMessage::HTTP_1_1);
            Poco::Net::HTTPResponse response;

            session.sendRequest(request);
            session.receiveResponse(response);

            // Check if the request failed
            if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
            {
                throw FailedDownloadException(Poco::format("HTTP server error: %d: %s", (int) response.getStatus()
                                                           , response.getReason()));
            }

            // Check if we can accept the Range header for chunked downloads
            if (!response.has("Accept-Ranges"))
            {
                throw FailedDownloadException("HTTP Server does not accept range requests");
            }
            // Make sure the server supports ranged
            if (response.has("Accept-Ranges"))
            {
                rangeType = response.get("Accept-Ranges");
                if (rangeType != "bytes")
                {
                    // For now, we don't want to deal with anything by bytes
                    throw FailedDownloadException("Server reported a range type of something other than bytes");
                }
            }
            // Get the file size
            _fileSize = response.getContentLength64();

            sendMessage(
                    Poco::format("Range request available with type %s and with a total file size of %?d", rangeType
                                 , _fileSize));

            // Check if we will eventually reach over the length of the file
            // While this isn't necessary if we want the full file, the above checks for file existance
            if ((numberChunks * _chunkSize) > _fileSize)
            {
                throw BadParameterException("Number of chunks along with chuck size is larger than the size of the file");
            }
        }
        catch (BadParameterException& e)
        {
            throw e;
        }
        catch (Poco::Exception& e)
        {
            throw FailedDownloadException(Poco::format("Error checking file from %s: %s", _uri.toString(), e.displayText()));
        }
    }

    void HTTPDownload::HTTPDownloader::Download(int chunkNumber)
    {
        // Initialize out locals. OutputFilename is changes if chunkNumber!=0
        std::string outputFilename = _outputFilename;

        // Downloading a chunk? Append the chunk number to the filename
        // The caller is assumed to understand that in chunked mode, the filename is changed
        if (chunkNumber!=0)
        {
            outputFilename = Poco::format("%s-%d", outputFilename, chunkNumber);
        }
        try
        {

            // Setup our connection. We assume we are direct and not through a proxy
            // The URI class deals with encoding the URL if any odd chars appear
            // We assume that authentication isn't required
            // We could pool the sessions, but use a new one each time for easy multi-thread support
            Poco::Net::HTTPClientSession session(_uri.getHost(), _uri.getPort());
            std::string path(_uri.getPathAndQuery());
            if (path.empty()) path = "/";

            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
            Poco::Net::HTTPResponse response;

            if (chunkNumber==0)
            {
                sendMessage("Request for complete file made...");
            }
            else
            {
                // Setup the download of a partial file
                long startPosition = (_chunkSize*(chunkNumber-1));
                long endPosition = (_chunkSize*chunkNumber);
                request.set("Range", Poco::format("bytes=%?d-%?d", startPosition, endPosition-1));
                // Content-Length is supposed to be provided, however, the pravala.com server gives 403 forbidden, yet it works elsewhere.
                // Without it the request does work
                //request.setContentLength64(endPosition-startPosition);

                sendMessage(Poco::format("Downloading chunk number %d for %?d bytes from position %?d to %?d", chunkNumber, endPosition-startPosition, startPosition, endPosition));
            }
            session.sendRequest(request);
            std::istream& responseStream = session.receiveResponse(response);
            Poco::FileOutputStream file(outputFilename);

            sendMessage(Poco::format("Downloading file: %s to %s...",_uri.toString(), outputFilename));

            // Make we have a valid response
            if ((response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) && (response.getStatus() != Poco::Net::HTTPResponse::HTTP_PARTIAL_CONTENT))
            {
                throw FailedDownloadException(Poco::format("HTTP server error: %d: %s", (int)response.getStatus() , response.getReason()));
            }

            // Download the file
            Poco::StreamCopier::copyStream(responseStream, file);

            // destructors for file and responseStream close streams automatically
        }
        catch (Poco::Exception& e)
        {
            throw FailedDownloadException(Poco::format("Error retrieving file from %s: %s", _uri.toString(), e.displayText()));
        }
    }

    void HTTPDownloader::sendMessage(const std::string &message)
    {
        if (_verbose)
        {
            std::cout << message << std::endl;
        }

    }


}


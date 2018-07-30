//********************************************************************
//    created:    2018-07-25 10:57 PM
//    file:       httpdownloader.cpp
//    author:     Mark Buckaway
//    purpose:
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

    void HTTPDownload::HTTPDownloader::Download(int chunkNumber, const std::string& filename)
    {
        std::string outputFilename = filename;
        std::string rangeType = "";

        // Downloading a chunk? Append the chunk number to the filename
        if (chunkNumber!=0)
        {
            outputFilename = Poco::format("%s-%d", outputFilename, chunkNumber);
        }
        try
        {

            // Setup our connection. We assume we are direct and not through a proxy
            // The URI class deals with encoding the URL if any odd chars appear
            // We assume that authentication isn't required
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
                throw FailedDownloadException(Poco::format("HTTP server error: %d: %s", response.getStatus(), response.getReason()));
            }

            // Check if we can accept the Range header
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
            int fileSize = response.getContentLength64();

            sendMessage(Poco::format("Range request available with type %s and with a total file size of %?d", rangeType, fileSize));

            // Reset for the next request
            session.reset();
            request.clear();
            response.clear();

            // Set out GET request
            request.setMethod(Poco::Net::HTTPRequest::HTTP_GET);
            if (chunkNumber==0)
            {
                sendMessage("Request for complete file made...");
            }
            else
            {
                // Setup the download of a partial file
                long startPosition = (MiB*(chunkNumber-1));
                long endPosition = (MiB*chunkNumber);
                request.set("Range", Poco::format("bytes=%?d-%?d", startPosition, endPosition-1));
                request.setContentLength64(endPosition-startPosition);

                sendMessage(Poco::format("Downloading chunk number %d for %?d bytes from position %?d to %?d", chunkNumber, endPosition-startPosition, startPosition, endPosition));
            }
            session.sendRequest(request);
            std::istream& responseStream = session.receiveResponse(response);
            Poco::FileOutputStream file(outputFilename);

            sendMessage(Poco::format("Downloading file: %s to %s...",_uri.toString(), outputFilename));

            // Make we have a valid response
            if ((response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) && (response.getStatus() != Poco::Net::HTTPResponse::HTTP_PARTIAL_CONTENT))
            {
                throw FailedDownloadException(Poco::format("HTTP server error: %d: %s", response.getStatus() , response.getReason()));
            }

            // Download the file
            Poco::StreamCopier::copyStream(responseStream, file);
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


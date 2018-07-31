//********************************************************************
//    created:    2018-07-25 9:16 PM
//    file:       httpapplicaton.cpp
//    author:     Mark Buckaway
//    purpose:    Main application code for downloader
//
//*********************************************************************

#include <Poco/NumberParser.h>
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Logger.h"
#include "Poco/URI.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/File.h"

#include "httpapplicaton.h"
#include "version.h"
#include "httpdownloader.h"
#include "httpdownloadexceptions.h"

namespace HTTPDownload
{
    HTTPApplication::HTTPApplication() :
                _displayHelp(false) // Default is no help
                , _verbose(false)   // default is not to use verbose mode (be quiet)
                , _fullFile(false)  // default is to download chunks
                , _numberChunks(4)  /// default is four chunks
                , _chunkSize(ONEMiB)
    {
        Application::setUnixOptions(true);
    }

    HTTPApplication::~HTTPApplication()
    {
    }

    void HTTPApplication::initialize(Application& self)
    {
#ifdef USE_LOGGING
        loadConfiguration(); // Add logging support. Required .properties file
#endif
        // Configure the subsystems including logging
        Application::initialize(self);

#ifdef USE_LOGGING
        Poco::Logger::root().information("HTTP Download Application started version %s", std::string(BUILDVERSION));
#endif
    }

    void HTTPApplication::uninitialize()
    {
        Application::uninitialize();
    }

    void HTTPApplication::defineOptions(Poco::Util::OptionSet& options)
    {
        Application::defineOptions(options);

        // Commands
        options.addOption(
                Poco::Util::Option("help", "h", "display argument help information")
                        .required(false)
                        .repeatable(false)
                        .callback(
                                Poco::Util::OptionCallback<HTTPApplication>(this, &HTTPApplication::handleHelp)));

        options.addOption(
                Poco::Util::Option("url", "u", "Remote URL of the file to download")
                        .required(false)
                        .repeatable(false)
                        .argument("url"));

        options.addOption(
                Poco::Util::Option("file", "f", "Specify the output file (default file filename from URL)")
                        .required(false)
                        .repeatable(false)
                        .argument("file"));

        options.addOption(
                Poco::Util::Option("verbose", "v", "Verbose operation")
                        .required(false)
                        .repeatable(false));

        options.addOption(
                Poco::Util::Option("full", "F", "Download the entire file rather than chunks")
                        .required(false)
                        .repeatable(false));

        options.addOption(
                Poco::Util::Option("chunks", "c", "Number of chunks to use (default is 4)")
                        .required(false)
                        .repeatable(false)
                        .argument("chunks"));

        options.addOption(
                Poco::Util::Option("size", "s", "Chunk size in bytes (default is 1MIB or 1048576)")
                        .required(false)
                        .repeatable(false)
                        .argument("size"));

    }

    void HTTPApplication::handleOption(const std::string &name, const std::string &value)
    {
        Application::handleOption(name, value);

        if (name == "url")
        {
            _url = value;
        }
        else if (name == "verbose")
        {
            _verbose = true;
        }
        else if (name == "file")
        {
            _outputFilename = value;
        }
        else if (name == "full")
        {
            _fullFile = true;
        }
        else if (name == "chunks")
        {
            // Get the number of chunks and ignore user error
            try
            {
                _numberChunks = Poco::NumberParser::parse(value);
                // Save std::cout for verbose info
                std::cerr << "Number of chunks specified as " << _numberChunks << std::endl;

            }
            catch (Poco::Exception& e)
            {
                std::cerr << "Invalid number of chunks ignored." << std::endl;
            }
        }
        else if (name == "size")
        {
            // Get size of the chunks. Must be 1 or larger
            try
            {
                _chunkSize = Poco::NumberParser::parse64(value);
                std::cerr << "Chunk size specified as " << _chunkSize << std::endl;
            }
            catch (Poco::Exception& e)
            {
                std::cerr << "Invalid number of chunks ignored." << std::endl;
            }
        }
    }

    void HTTPApplication::displayHelp()
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setUnixStyle(true);
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader(Poco::format("HTTP MultiGET Download Utility V%s",std::string(BUILDVERSION)));
        helpFormatter.format(std::cout);

    }

    void HTTPApplication::handleHelp(const std::string& name, const std::string& value)
    {
        _displayHelp = true;
        stopOptionsProcessing();
    }

    int HTTPApplication::main(const std::vector<std::string>& args)
    {
        int result = Application::EXIT_OK;
        if (_displayHelp || _url.empty())
        {
            displayHelp();
        }
        else
        {
            try
            {
                Poco::URI uri(_url);
                if (uri.isRelative())
                {
                    throw BadParameterException("Fully qualified URL is required");
                }
                if (_outputFilename.empty())
                {
                    // Get the output filename from the last item in the URL
                    std::vector<std::string> segments;
                    uri.getPathSegments(segments);
                    _outputFilename = segments[segments.size() - 1];
                }

                // Setup out download
                HTTPDownload::HTTPDownloader downloader(uri, _outputFilename, _verbose, _chunkSize);

                // Verify our download can succeed. Check file existance, server support, and
                // if the command line options are valid
                downloader.CheckRemoteFile(_numberChunks);

                // If we have the file file, we download everything, otherwise chunk it
                if (_fullFile)
                {
                    downloader.Download();
                }
                else
                {
                    // Download the chunks first, in case there are errors
                    // An improvement would be to store the temp files on /tmp or simular, but we use the
                    // directory of the output file for now. We could also setup a POCO ThreadPool
                    // to download these in parallel.

                    // We start at 1 because 0 means download the full file
                    for (int count = 1; count <= _numberChunks; count++)
                    {
                        downloader.Download(count);
                    }

                    // Rebuild the file after a successfull download
                    Poco::FileOutputStream outfile(_outputFilename);
                    for (int fileno = 1; fileno <= _numberChunks; fileno++)
                    {
                        std::string infilename = Poco::format("%s-%d", _outputFilename, fileno);
                        Poco::FileInputStream infile(infilename);
                        Poco::StreamCopier::copyStream(infile, outfile);
                        infile.close();
                        Poco::File tmpfile(infilename);
                        tmpfile.remove();
                    }
                    outfile.close();
                    // We could use POCO logging to log to the console instead of verbose checks and
                    // SendMessage in HTTPDownloader as an improvement. However, sending text
                    // to the console is good enough for now
                    if (_verbose)
                    {
                        std::cout << "Chunk file downloaded and stored in " << _outputFilename << std::endl;
                    }
                }
            }
            catch (Poco::Exception &e)
            {
                // On error, we do not clean up chunk files for now, but an improvement would be to
                // remove any temporary files
                std::cerr << "Downloader returned an error: " << e.displayText() << std::endl;
                result = Application::EXIT_IOERR;
            }
        }
        return result;
    }
}

//********************************************************************
//    created:    2018-07-25 9:16 PM
//    file:       httpapplicaton.cpp
//    author:     Mark Buckaway
//    purpose:
//
//*********************************************************************

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
    HTTPApplication::HTTPApplication() : _displayHelp(false), _verbose(false), _fullFile(false)
    {
        Application::setUnixOptions(true);
    }

    HTTPApplication::~HTTPApplication()
    {
    }

    void HTTPApplication::initialize(Application& self)
    {
        // Load our default configuration file
        //loadConfiguration();
        // Configure the subsystems including logging
        Application::initialize(self);
        // open the log
        //Poco::Logger::root().information("HTTP Download Application started version %s", std::string(BUILDVERSION));
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
    }

    void HTTPApplication::displayHelp()
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setUnixStyle(true);
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader(
                "HTTP Download Utility");
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
                HTTPDownload::HTTPDownloader downloader(uri, _verbose);

                // If we have the file file, we download everything, otherwise chunk it
                if (_fullFile)
                {
                    downloader.Download(0, _outputFilename);
                }
                else
                {
                    // Download the chunks - order doesn't really matter
                    // An improvement would be to store the temp files on /tmp or simular, but we use the
                    // current directory for now
                    downloader.Download(1, _outputFilename);
                    downloader.Download(2, _outputFilename);
                    downloader.Download(3, _outputFilename);
                    downloader.Download(4, _outputFilename);

                    // Rebuild the file
                    Poco::FileOutputStream outfile(_outputFilename);
                    for (int fileno = 1; fileno <= 4; fileno++)
                    {
                        std::string infilename = Poco::format("%s-%d", _outputFilename, fileno);
                        Poco::FileInputStream infile(infilename);
                        Poco::StreamCopier::copyStream(infile, outfile);
                        infile.close();
                        Poco::File tmpfile(infilename);
                        tmpfile.remove();
                    }
                    outfile.close();
                    if (_verbose)
                    {
                        std::cout << "Chunk file downloaded and stored in " << _outputFilename << std::endl;
                    }
                }
            }
            catch (Poco::Exception &e)
            {
                std::cerr << "Downloader returned an error: " << e.displayText() << std::endl;
                result = Application::EXIT_IOERR;
            }
        }
        return result;
    }
}

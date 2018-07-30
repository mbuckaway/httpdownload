//********************************************************************
//    created:    2018-07-25 9:16 PM
//    file:       httpapplicaton.h
//    author:     Mark Buckaway
//    purpose:
//
//*********************************************************************


#ifndef HTTPDOWNLOAD_HTTPAPPLICATON_H
#define HTTPDOWNLOAD_HTTPAPPLICATON_H

#include "Poco/Util/Application.h"

namespace HTTPDownload
{
    /// \brief Standard POCO Application subclass defining all the methods
    /// necessary to setup the class, parse the command line, and run the program
    class HTTPApplication : public Poco::Util::Application
    {
    public:
        HTTPApplication();
        virtual ~HTTPApplication();

    protected:
        /// \brief Standard POCO Initialize application method
        /// \param self Application reference to the POCO Application (called by Application class)
        void initialize(Application& self) override;
        /// \brief Standard POCO teardown application method
        void uninitialize() override;

        /// \brief Defines the options used by our app
        void defineOptions(Poco::Util::OptionSet& options) override;

        /// \brief Handles parsing of the command line
        void handleOption(const std::string &name, const std::string &value) override;

        /// \brief main entry point for our application (provided by POCO)
        int main(const std::vector<std::string>& args) override;

    private:
        bool _displayHelp;
        bool _verbose;
        std::string _outputFilename;
        std::string _url;
        bool _fullFile;
        int _numberChunks;
        long _chunkSize;

        /// \brief Display help to the user
        void displayHelp();

        /// \brief Special routine to process the help (stops command line option processing)
        void handleHelp(const std::string& name, const std::string& value);

        // 1 MiB, not 1MB, in bytes
        static long const ONEMiB = 1024*1024;

    };
}
#endif //HTTPDOWNLOAD_HTTPAPPLICATON_H

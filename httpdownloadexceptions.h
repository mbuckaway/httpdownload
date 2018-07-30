//********************************************************************
//    created:    2018-07-26 9:00 PM
//    file:       httpdownloadexceptions.h
//    author:     Mark Buckaway
//    purpose:
//
//*********************************************************************


#ifndef HTTPDOWNLOAD_HTTPDOWNLOADEXCEPTIONS_H
#define HTTPDOWNLOAD_HTTPDOWNLOADEXCEPTIONS_H

#include "Poco/Exception.h"

POCO_DECLARE_EXCEPTION(, BadParameterException, Poco::Exception);
POCO_DECLARE_EXCEPTION(, FailedDownloadException, Poco::Exception);

#endif //HTTPDOWNLOAD_HTTPDOWNLOADEXCEPTIONS_H

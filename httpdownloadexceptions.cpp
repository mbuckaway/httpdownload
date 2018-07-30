//********************************************************************
//    created:    2018-07-26 9:05 PM
//    file:       httpdownloadexceptions.cpp
//    author:     Mark Buckaway
//    purpose:
//
//*********************************************************************


#include <typeinfo>
#include "httpdownloadexceptions.h"

POCO_IMPLEMENT_EXCEPTION(BadParameterException, Poco::Exception, "Invalid Paramter");
POCO_IMPLEMENT_EXCEPTION(FailedDownloadException, Poco::Exception, "Download failed");


/**
 * \file
 * \brief GMock Utils.
 */

#ifndef __GMOCK_UTILS_H__
#define __GMOCK_UTILS_H__

#include "gmock/gmock.h"

namespace testing { 
    namespace GMockUtils {

        // Call once per test class or module to set up everything needed by GoogleMock.
        void InitGoogleMock();

        // Call once per test method to check for GoogleMock messages.
        void CheckGoogleMock();
    }
}

#endif // __GMOCK_UTILS_H__
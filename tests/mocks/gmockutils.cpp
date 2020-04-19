/**
 * \file
 * \brief GMock Utils.
 */

#include "gmockutils.h"

#include <QtTest/QtTest>

namespace testing {
    namespace GMockUtils {
        namespace {

            // Test event listener for use with CppUnitTestFramework
            class GoogleTestEventListener : public EmptyTestEventListener
            {
            public:

                void OnTestStart(const ::testing::TestInfo&) override
                {
                }

                void OnTestPartResult(const ::testing::TestPartResult& test_part_result) override
                {
                    if (test_part_result.failed())
                    {
                        QFAIL(
                            QString("mock objects failed with '%1' at %2:%3")
                                .arg(QString(test_part_result.summary()))
                                .arg(test_part_result.file_name())
                                .arg(test_part_result.line_number())
                                .toLatin1().constData()
                        );
                    }
                }

                // Called after a test ends.
                void OnTestEnd(const ::testing::TestInfo&) override
                {
                }

            } *_listener;
        }

        // Initialize the Google Mock framework for use with Qt test
        void InitGoogleMock()
        {
            // Avoids calling the function unnecessarily
            if (_listener != nullptr)
                return;

            // Dummy command line parameters (could pass exe path here)
            int argc = 0;
            char** argv = nullptr;

            // Initialize the framework
            ::testing::InitGoogleMock(&argc, argv);

            // We don't want exceptions thrown, regardless what the doc says
            GTEST_FLAG(throw_on_failure) = false;

            // Remove default listener
            auto &listeners = UnitTest::GetInstance()->listeners();
            delete listeners.Release(listeners.default_result_printer());

            // Create and install the new listener
            _listener = new GoogleTestEventListener();
            listeners.Append(_listener);
        }
    }
}

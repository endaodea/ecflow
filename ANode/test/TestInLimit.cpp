/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #1 $
//
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
// Description :
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#include <iostream>
#include <stdexcept>

#include <boost/test/unit_test.hpp>

#include "InLimit.hpp"
#include "SerializationTest.hpp"
#include "Task.hpp"

using namespace std;
using namespace ecf;

BOOST_AUTO_TEST_SUITE(NodeTestSuite)

BOOST_AUTO_TEST_CASE(test_inlimit_basics) {
    cout << "ANode:: ...test_inlimit_basics \n";
    {
        InLimit empty;
        InLimit empty2;
        BOOST_CHECK_MESSAGE(empty == empty2, "Equality failed");

        InLimit l1("name", "path");
        InLimit l2("name", "path");
        BOOST_CHECK_MESSAGE(l1 == l2, "Equality failed");

        InLimit a("name", "path", 10);
        InLimit b("name", "path");
        BOOST_CHECK_MESSAGE(!(a == b), "Equality passed when should fail");
    }
    {
        InLimit inlim("fred", "/path/to/node", 1, true);
        {
            InLimit testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Copy constructor failed");
        }
        {
            InLimit testCopy;
            testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Assignment failed");
        }
    }
    {
        InLimit inlim("fred", "/path/to/node", 1, false);
        {
            InLimit testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Copy constructor failed");
        }
        {
            InLimit testCopy;
            testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Assignment failed");
        }
    }
    {
        InLimit inlim("fred", "/path/to/node", 1, false, true);
        {
            InLimit testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Copy constructor failed");
        }
        {
            InLimit testCopy;
            testCopy = inlim;
            BOOST_CHECK_MESSAGE(testCopy == inlim, "Assignment failed");
        }
    }
}

BOOST_AUTO_TEST_CASE(test_inlimit_duplicates) {
    cout << "ANode:: ...test_inlimit_duplicates \n";

    InLimit inlim("fred", "/path/to/node", 1, true);

    task_ptr task = Task::create("task");
    task->addInLimit(inlim);

    // duplicate should throw
    BOOST_REQUIRE_THROW(task->addInLimit(inlim), std::runtime_error);

    // still a duplicate
    InLimit inlim2("fred", "/path/to/node");
    BOOST_REQUIRE_THROW(task->addInLimit(inlim2), std::runtime_error);
}

// Globals used throughout the test
static std::string fileName = "test_InLimit_serialisation.txt";
BOOST_AUTO_TEST_CASE(test_InLimit_serialisation) {
    cout << "ANode:: ...test_InLimit_serialisation\n";
    {
        // save and restore the default constructor
        doSaveAndRestore<InLimit>(fileName);

        InLimit saved("limitName", "/path/to/some/node", 20, true);
        save(fileName, saved);

        InLimit restored;
        restore(fileName, restored);
        BOOST_CHECK_MESSAGE(saved == restored, " save and restored don't match");
        std::remove(fileName.c_str());
    }
    {
        InLimit saved("limitName", "/path/to/some/node", 20, false, true);
        save(fileName, saved);

        InLimit restored;
        restore(fileName, restored);
        BOOST_CHECK_MESSAGE(saved == restored, " save and restored don't match");
        std::remove(fileName.c_str());
    }
}

BOOST_AUTO_TEST_SUITE_END()

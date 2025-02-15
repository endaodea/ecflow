#ifndef OPTIONS_HPP
#define OPTIONS_HPP

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        : Options
// Author      : partio
// Revision    : $Revision$
//
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#include <string>

struct Options
{
    bool verbose{false};                                                        // ECF_RESTAPI_VERBOSE
    bool no_ssl{false};                                                         // ECF_RESTAPI_NOSSL
    int polling_interval{10};                                                   // ECF_RESTAPI_POLLING_INTERVAL
    int port{8080};                                                             // ECF_RESTAPI_PORT
    std::string ecflow_host{"localhost"};                                       // ECF_HOST
    int ecflow_port{3141};                                                      // ECF_PORT
    std::string tokens_file{"api-tokens.json"};                                 // ECF_RESTAPI_TOKENS_FILE
    std::string cert_directory{std::string(getenv("HOME")) + "/.ecflowrc/ssl"}; // ECF_RESTAPI_CERT_DIRECTORY
    int max_polling_interval{300};                                              // ECF_RESTAPI_MAX_POLLING_INTERVAL
};
#endif

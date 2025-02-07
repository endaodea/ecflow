/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        : HttpServer
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

#include "HttpServer.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "ApiV1.hpp"
#include "Options.hpp"
#include "nlohmann/json.hpp"

Options opts;

HttpServer::HttpServer(int argc, char** argv) {
    parse_args(argc, argv);
}

void read_environment() {
    if (getenv("ECF_RESTAPI_VERBOSE") != nullptr) {
        opts.verbose = true;
    }
    if (getenv("ECF_RESTAPI_NOSSL") != nullptr) {
        opts.no_ssl = true;
    }
    if (getenv("ECF_RESTAPI_POLLING_INTERVAL") != nullptr) {
        opts.polling_interval = atoi(getenv("ECF_RESTAPI_POLLING_INTERVAL"));
    }
    if (getenv("ECF_RESTAPI_PORT") != nullptr) {
        opts.port = atoi(getenv("ECF_RESTAPI_PORT"));
    }
    if (getenv("ECF_HOST") != nullptr) {
        opts.ecflow_host = std::string(getenv("ECF_HOST"));
    }
    if (getenv("ECF_PORT") != nullptr) {
        opts.ecflow_port = atoi(getenv("ECF_PORT"));
    }
    if (getenv("ECF_RESTAPI_TOKENS_FILE") != nullptr) {
        opts.tokens_file = std::string(getenv("ECF_RESTAPI_TOKENS_FILE"));
    }
    if (getenv("ECF_RESTAPI_CERT_DIRECTORY") != nullptr) {
        opts.tokens_file = std::string(getenv("ECF_RESTAPI_CERT_DIRECTORY"));
    }
    if (getenv("ECF_RESTAPI_MAX_UPDATE_INTERVAL") != nullptr) {
        opts.max_polling_interval = atoi(getenv("ECF_RESTAPI_MAX_UPDATE_INTERVAL"));
    }
}

void HttpServer::parse_args(int argc, char** argv) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options", 100);

    read_environment();

    // clang-format off

   bool verbose = false;
   bool no_ssl = false;

   desc.add_options()
       ("cert_directory", po::value(&opts.cert_directory), "directory where certificates are found (default: $HOME/.ecflowrc/ssl)")
       ("ecflow_host", po::value(&opts.ecflow_host), "hostname of ecflow server (default: localhost)")
       ("ecflow_port", po::value(&opts.ecflow_port), "port of ecflow server (default: 3141)")
       ("help,h", "print help message")
       ("max_polling_interval", po::value(&opts.max_polling_interval), "maximum polling interval in seconds, set to 0 to disable drift (default: 300)")
       ("no_ssl", po::bool_switch(&no_ssl), "disable ssl (default: false)")
       ("port,p", po::value(&opts.port), "port to listen (default: 8080)")
       ("polling_interval", po::value(&opts.polling_interval), "interval in seconds to poll ecflow server for updates (default: 10)")
       ("tokens_file", po::value(&opts.tokens_file), "location of api tokens file (default: api-tokens.json)")
       ("verbose,v", po::bool_switch(&verbose), "enable verbose mode");

    // clang-format on

    po::variables_map opt;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), opt);
    po::notify(opt);

    if (opt.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }
    if (verbose)
        opts.verbose = true;
    if (no_ssl)
        opts.no_ssl = true;

    setenv("ECF_HOST", opts.ecflow_host.c_str(), 1);
    setenv("ECF_PORT", std::to_string(opts.ecflow_port).c_str(), 1);
    // Unset these, otherwise ClientInvoker will automatically
    // try to use them
    unsetenv("ECF_PASSWD");
    unsetenv("ECF_CUSTOM_PASSWD");
}

void apply_listeners(httplib::Server& http_server) {
    const auto dump_headers = [](const httplib::Headers& headers, std::stringstream& ss) {
        for (const auto& m : headers) {
            ss << m.first << ": " << m.second << "\n";
        }
    };
    const auto format = [&dump_headers](const httplib::Request& req, const httplib::Response& res) {
        std::stringstream ss;

        ss << req.method << " " << req.version << " " << req.path;

        char sep = '?';

        for (const auto& p : req.params) {
            ss << sep << p.first << "=" << p.second;
            sep = '&';
        }

        ss << "\n";
        dump_headers(req.headers, ss);

        ss << "\nresponse: ";
        ss << res.status << " " << res.version << "\n";
        dump_headers(res.headers, ss);

        if (!res.body.empty()) {
            ss << "\n" << res.body;
        }

        return ss.str();
    };

    http_server.set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        }
        catch (const std::exception& e) {
            if (opts.verbose)
                printf("Exception: Error 500: %s\n", e.what());
            nlohmann::json j;
            j["status"]  = res.status;
            j["message"] = e.what();
            j["path"]    = req.path;
            j["method"]  = req.method;
            if (req.body.empty() == false)
                j["body"] = req.body;
            res.set_content(j.dump(), "application/json");
        }
        catch (...) {
            if (opts.verbose)
                printf("Unknown exception occurred\n");
        }
    });

    http_server.set_error_handler([&format](const httplib::Request& req, httplib::Response& res) {
        if (opts.verbose)
            printf("error_handler called: Error: %d\n", res.status);

        // Usually no need to modify response fields -- they should be configured now, as
        // error handler is called *after* httplib finds out that status is >= 400
        //
        // Exception is when errorhandler is called so that it doesn't go through the
        // registered endpoints

        if (res.body.empty()) {
            nlohmann::json j;
            j["path"]   = req.path;
            j["status"] = res.status;
            res.set_content(j.dump(), "application/json");
        }
    });

    http_server.set_logger([&format](const httplib::Request& req, const httplib::Response& res) {
        const std::string str = format(req, res);
        // using std::cout crashes with many threads calling
        if (opts.verbose)
            printf("%s\n", str.c_str());
    });
}

void start_server(httplib::Server& http_server) {
    if (opts.verbose)
        printf("ecFlow server location is %s:%d\n", opts.ecflow_host.c_str(), opts.ecflow_port);

    ApiV1::create(http_server);

    const std::string proto = (opts.no_ssl ? "http" : "https");

    if (opts.verbose)
        printf("%s server listening on port %d\n", proto.c_str(), opts.port);

    try {
        http_server.listen("0.0.0.0", opts.port);
    }
    catch (const std::exception& e) {
        if (opts.verbose)
            printf("Server execution stopped: %s\n", e.what());
    }
}

void HttpServer::run() {
#ifdef ECF_OPENSSL
    if (opts.no_ssl == false) {
        namespace fs = boost::filesystem;

        if (fs::exists(opts.cert_directory + "/server.crt") == false ||
            fs::exists(opts.cert_directory + "/server.key") == false) {
            throw std::runtime_error("Directory " + opts.cert_directory +
                                     " does not contain server.crt and/or server.key");
        }

        const std::string cert = opts.cert_directory + "/server.crt";
        const std::string key  = opts.cert_directory + "/server.key";

        httplib::SSLServer http_server(cert.c_str(), key.c_str());
        apply_listeners(dynamic_cast<httplib::Server&>(http_server));
        start_server(http_server);
    }
#endif

    httplib::Server http_server;

    apply_listeners(http_server);
    start_server(http_server);
}

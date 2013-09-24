//#ifndef STACKTRACE_HPP_
//#define STACKTRACE_HPP_
//
////============================================================================
//// Name        :
//// Author      : Avi
//// Revision    : $Revision: #5 $ 
////
//// Copyright 2009-2012 ECMWF. 
//// This software is licensed under the terms of the Apache Licence version 2.0 
//// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
//// In applying this licence, ECMWF does not waive the privileges and immunities 
//// granted to it by virtue of its status as an intergovernmental organisation 
//// nor does it submit to any jurisdiction. 
////
//// Description : This class is used dump the a stack trace at the point where its called
////               At the moment only works on Linux
////============================================================================
//
//#include <boost/noncopyable.hpp>
//#include <string>
//
//namespace ecf {
//
//class StackTrace : private boost::noncopyable {
//public:
//
//	/// This function will produce a stack trace from where its called
//	static std::string dump( const std::string &file, int line, int depth = 50 );
//
//private:
//	StackTrace(){}
//};
//
//}
//
//#endif

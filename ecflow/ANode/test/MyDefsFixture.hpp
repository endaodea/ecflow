#ifndef MYDEFSFIXTURE_HPP_
#define MYDEFSFIXTURE_HPP_
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #10 $
//
// Copyright 2009-2012 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Description : The structure ONLY used to test the persistence/migration
//               as each new object is created we add it here, to test
//               Serialisation read/write and migration of previous fixtures
//============================================================================
#include <boost/lexical_cast.hpp>

#include "Defs.hpp"
#include "Suite.hpp"
#include "Family.hpp"
#include "Task.hpp"
#include "Alias.hpp"
#include <algorithm> //for for_each()
// =======================================================================
// This struct is used in the node migration tests.
// If we ever add to this , then update TestMigration.cpp
// **Ensure** that we keep old fixture.def file, to test that future ecflow
// versions can migrate old data.
// =======================================================================
struct MyDefsFixture {

	MyDefsFixture(const std::string& fileName = "defsfile.txt") : defsfile_()
	{
		suite_ptr  suite = create_suite();

 		// Must be done last
		defsfile_.addSuite( suite  );
		defsfile_.add_extern("/limits:event");
		defsfile_.add_extern("/a/b/c:meter");
		defsfile_.add_extern("/a/b/c/d");

		// add an empty suite. Needed for CHECK_JOB_GEN_ONLY cmd
		defsfile_.addSuite( Suite::create("EmptySuite" ) );

		// Check expression parse
		std::string errorMsg, warningMsg;
 		bool result = defsfile_.check(errorMsg,warningMsg);
 		if (!result || !errorMsg.empty()) {
 			std::cout << errorMsg;
 			assert(false);
 		}
	}
	~MyDefsFixture() {}

	const Defs& fixtureDefsFile() const { return defsfile_; }

	void remove_host_depedent_server_variables()
	{
	   // Allow test data to be used on other platforms
	   defsfile_.set_server().delete_server_variable("ECF_LOG");
	   defsfile_.set_server().delete_server_variable("ECF_CHECK");
	   defsfile_.set_server().delete_server_variable("ECF_CHECKOLD");
	}


	defs_ptr create_defs() const {

		defs_ptr defs = Defs::create();

 		defs->addSuite(  create_suite()   );
		defs->add_extern("/limits:event");
		defs->add_extern("/a/b/c:meter");
		defs->add_extern("/a/b/c/d");
      defs->set_server().add_or_update_user_variables("MyDefsFixture_user_variable","This is a user variable added to server");

		// add an empty suite. Needed for CHECK_JOB_GEN_ONLY cmd
		defs->addSuite( Suite::create("EmptySuite" ) );

		// Check expression parse
		std::string errorMsg, warningMsg;
 		bool result = defs->check(errorMsg,warningMsg);
 		if (!result || !errorMsg.empty()) {
 			std::cout << errorMsg;
 			assert(false);
 		}
 		return defs;
	}

	Defs defsfile_;

private:
	suite_ptr create_suite() const {
		std::string sname = "suiteName";
		suite_ptr suite = Suite::create(  sname );

      ClockAttr clockAttr(false);
      clockAttr.date(1,1,2009);
      clockAttr.set_gain_in_seconds(3600);
      clockAttr.startStopWithServer(true);
      suite->addClock( clockAttr );

 		suite->addAutoCancel( ecf::AutoCancelAttr(2) );
 		suite->addVariable( Variable("VAR","value") );
		suite->addVariable( Variable("VAR1","\"value\"") );
		suite->addVariable( Variable("ECF_FETCH","\"smsfetch -F %ECF_FILES% -I %ECF_INCLUDE%\"") );

		suite->add_task( "t1" );
		suite->add_task( "t2" );
		task_ptr suiteTask = suite->add_task( "t3" );
		suiteTask->add_part_trigger( PartExpression("t1 == complete") );
		suiteTask->add_part_trigger( PartExpression("t2 == complete",false) );
		suiteTask->add_part_complete( PartExpression("t1 == complete") );
		suiteTask->add_part_complete( PartExpression("t2 == complete",true) );

		std::vector<ecf::Child::CmdType> child_cmds;
		child_cmds.push_back(ecf::Child::INIT);
		child_cmds.push_back(ecf::Child::EVENT);
		child_cmds.push_back(ecf::Child::METER);
		child_cmds.push_back(ecf::Child::LABEL);
		child_cmds.push_back(ecf::Child::WAIT);
		child_cmds.push_back(ecf::Child::ABORT);
		child_cmds.push_back(ecf::Child::COMPLETE);
		suiteTask->addZombie( ZombieAttr(ecf::Child::USER, child_cmds, ecf::User::FOB,10) );
		suiteTask->addZombie( ZombieAttr(ecf::Child::ECF, child_cmds, ecf::User::FAIL,100) );
		suiteTask->addZombie( ZombieAttr(ecf::Child::PATH, child_cmds, ecf::User::BLOCK,100) );


		task_ptr suiteTask4 = suite->add_task( "t4" );
		suiteTask4->addZombie( ZombieAttr(ecf::Child::USER, child_cmds, ecf::User::ADOPT,10) );
		suiteTask4->addZombie( ZombieAttr(ecf::Child::ECF, child_cmds, ecf::User::REMOVE,100) );
		suiteTask4->addZombie( ZombieAttr(ecf::Child::PATH, child_cmds, ecf::User::BLOCK,100) );


 		ecf::CronAttr cronAttr;
 		ecf::TimeSlot start( 0, 0 );
 		ecf::TimeSlot finish( 10, 0 );
 		ecf::TimeSlot incr( 0, 5 );
 		std::vector<int> weekdays;   for(int i=0;i<7;++i) weekdays.push_back(i);
 		std::vector<int> daysOfMonth;for(int i=1;i<32;++i) daysOfMonth.push_back(i);
 		std::vector<int> months;     for(int i=1;i<13;++i) months.push_back(i);
 		cronAttr.addTimeSeries(start,finish,incr);
		cronAttr.addWeekDays( weekdays  );
		cronAttr.addDaysOfMonth(daysOfMonth);
		cronAttr.addMonths(  months );
 		suite->addCron( cronAttr  );

 		ecf::LateAttr lateAttr;
 		lateAttr.addSubmitted( ecf::TimeSlot(3,12) );
 		lateAttr.addActive( ecf::TimeSlot(3,12) );
 		lateAttr.addComplete( ecf::TimeSlot(4,12), true);

		std::string suiteLimit = "suiteLimit";
		suite->addLimit( Limit(suiteLimit,10) );

      std::vector<std::string> stringList; stringList.reserve(3);
      stringList.push_back("10");
      stringList.push_back("20");
      stringList.push_back("30");

      // Add tasks with all the repeat variants
      task_ptr t5 = suite->add_task( "t5" );
      t5->addRepeat( RepeatEnumerated("AEnum",stringList));

      task_ptr t6 = suite->add_task( "t6" );
      t6->addRepeat( RepeatString("aString",stringList));

      task_ptr t7 = suite->add_task( "t7" );
      t7->addRepeat( RepeatInteger("rep",0,100,1) );

      task_ptr t8 = suite->add_task( "t8" );
      t8->addRepeat( RepeatDate("YMD",20090916,20090916,1) );

      task_ptr t9 = suite->add_task( "t9" );
      t9->addRepeat( RepeatDay(2) );


 		for (int i = 0; i < 3; ++i) {
			std::string fname = "familyName";
         std::string tname = "taskName";
			std::string eventName = "eventName";
			std::string labelName = "labelName";
			std::string limitName = "limitName";
			if ( i != 0 ) {
 				fname += boost::lexical_cast< std::string >( i );
				tname += boost::lexical_cast< std::string >( i );
				labelName += boost::lexical_cast< std::string >( i );
			}

         family_ptr fam = suite->add_family( fname );
         fam->addDate( DateAttr(0,0,2009) ); // 0 is equivalent to a *
         fam->addRepeat( RepeatEnumerated("AEnum",stringList));
         fam->addAutoCancel( ecf::AutoCancelAttr( ecf::TimeSlot(1,0), true));
         fam->addVariable( Variable("VAR","value") );
         fam->addTime( ecf::TimeAttr(ecf::TimeSlot(0,0),ecf::TimeSlot(10,1),ecf::TimeSlot(0,1),true) );
         fam->addLimit( Limit(limitName,20) );
         fam->addLate( lateAttr );

         task_ptr task = fam->add_task( tname );
			task->addDate( DateAttr(1,2,2009) );
			task->addDay( DayAttr(DayAttr::MONDAY) );
			task->addVariable( Variable("VAR1","\"value\"") );
			task->addEvent( Event(i) );
			task->addEvent( Event(i+1, eventName ) );
			task->addMeter( Meter("myMeter",0,100,100) );
			task->addLabel( Label(labelName,"\"labelValue\"") );
 			task->addTime( ecf::TimeAttr(ecf::TimeSlot(10,10),true) );
 			task->addToday( ecf::TodayAttr(ecf::TimeSlot(10,12)) );
 			task->addToday( ecf::TodayAttr(ecf::TimeSlot(0,1),ecf::TimeSlot(0,3),ecf::TimeSlot(0,1),true) );
 			task->addDefStatus( DState::COMPLETE );
 			task->addInLimit( InLimit(suiteLimit,"/" + sname ));
 			task->addVerify( VerifyAttr(NState::COMPLETE,3) );
 			task->addLate( lateAttr );
			if (i == 2) {
				 std::string compExpr = "../familyName" + boost::lexical_cast< std::string >( i-1 );
				 compExpr += "/taskName" + boost::lexical_cast< std::string >( i-1 );
				 compExpr += ":myMeter ge 10";
				 task->add_complete( compExpr );

				 std::string expression = "../familyName" + boost::lexical_cast< std::string >( i-1 );
				 expression += "/taskName" + boost::lexical_cast< std::string >( i-1 );
				 expression += " == complete";
				 task->add_trigger( expression );
			}
         task->add_alias_only(); //add alias without creating dir & .usr file
         task->add_alias_only();


			// Add a hierarchical family to the first family
			if (i == 0) {
				std::string heirFamily = "heir_" + fname;
				family_ptr hierFam = fam->add_family( heirFamily );
				hierFam->addVariable( Variable("VAR1","value") );
				hierFam->addRepeat( RepeatString("aString",stringList));

				task_ptr task1 = hierFam->add_task( tname );
				task1->addVariable( Variable("VAR1","value") );
				task1->addEvent( Event(i) );
				task1->addEvent( Event(i+1, eventName ) );
				task1->addMeter( Meter("myMeter",0,100,100) );
		 		task1->addAutoCancel( ecf::AutoCancelAttr( ecf::TimeSlot(0,1), false));
			}
 		}
 		return suite;
	}
};
#endif

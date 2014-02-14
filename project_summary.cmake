message( STATUS "------------------------------------------------------" )

if(Boost_FOUND)
    message( STATUS " Boost    include : [${Boost_INCLUDE_DIRS}]" )
    message( STATUS "          libs    : [${Boost_SYSTEM_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_SERIALIZATION_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_THREAD_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_FILESYSTEM_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_PROGRAM_OPTIONS_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_DATE_TIME_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}]" )
    message( STATUS "          libs    : [${Boost_TEST_EXEC_MONITOR_LIBRARY}]" )
endif()


project(CFE_PUS C)

set(APP_SRC_FILES
  fsw/src/pus.c
  fsw/src/pus_cmds.c
  fsw/src/pus_utils.c
)

if (CFE_EDS_ENABLED)
  list(APPEND APP_SRC_FILES
    fsw/src/pus_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/pus_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(pus ${APP_SRC_FILES})

target_include_directories(pus PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(pus sample_lib)

# Add table
add_cfe_tables(pus fsw/tables/pus_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)

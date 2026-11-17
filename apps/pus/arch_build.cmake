###########################################################
#
# PUS platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PUS configuration
set(PUS_PLATFORM_CONFIG_FILE_LIST
  pus_internal_cfg_values.h
  pus_platform_cfg.h
  pus_perfids.h
  pus_msgids.h
  pus_msgid_values.h
)

generate_configfile_set(${PUS_PLATFORM_CONFIG_FILE_LIST})


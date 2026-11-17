###########################################################
#
# PUS mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the PUS configuration
set(PUS_MISSION_CONFIG_FILE_LIST
  pus_fcncode_values.h
  pus_interface_cfg_values.h
  pus_mission_cfg.h
  pus_perfids.h
  pus_msg.h
  pus_msgdefs.h
  pus_msgstruct.h
  pus_tbl.h
  pus_tbldefs.h
  pus_tblstruct.h
  pus_topicid_values.h
)

generate_configfile_set(${PUS_MISSION_CONFIG_FILE_LIST})


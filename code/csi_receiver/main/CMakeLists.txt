idf_component_register(SRCS "csi_receiver.c"
                    INCLUDE_DIRS ".")
set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS -Wno-format)

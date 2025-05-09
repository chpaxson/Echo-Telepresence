idf_component_register(SRCS "main.c"
                            "rest_server.c"
                            "config_vars.c"
                    INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../../WebGUI/dist FLASH_IN_PROJECT)
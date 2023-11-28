if(IDF_TARGET STREQUAL "esp32") # Za ESP32
idf_component_register(SRC_DIRS "." "network" "controller/blinds" "driver/dht" "driver/servo"
                    INCLUDE_DIRS "." "network" "controller/blinds" "driver/dht" "driver/servo")
elseif(IDF_TARGET STREQUAL "esp32c3") # Za ESP32-C3
idf_component_register(SRC_DIRS "." "network" "controller/lights"
                    INCLUDE_DIRS "." "network" "controller/lights")
else()
endif()

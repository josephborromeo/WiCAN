idf_component_register(
    SRCS 
        "WiCAN_RX.c" 
        "../../common/rgb_led.c"
        "../../common/temp_sensor.c"
        "../../common/wifi.c"
                    
    INCLUDE_DIRS 
        "."
        "../../common"
    )

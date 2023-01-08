idf_component_register(
    SRCS 
        "WiCAN_TX.c" 
        "sdmmc_driver.c"
        "../../common/rgb_led.c"
        "../../common/temp_sensor.c"
                    
    INCLUDE_DIRS 
        "."
        "../../common"
    )

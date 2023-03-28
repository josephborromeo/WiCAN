idf_component_register(
    SRCS 
        "WiCAN_TX.c" 
        "sdmmc_driver.c"
        "usb_msc.c"
        "../../common/rgb_led.c"
        "../../common/temp_sensor.c"
        "../../common/twai_driver.c"
        "../../common/wifi.c"
        "../../common/slcan.c"
                    
    INCLUDE_DIRS 
        "."
        "../../common"
    )
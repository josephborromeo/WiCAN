idf_component_register(
    SRCS 
        "WiCAN_RX.c"
        "usb_driver.c"
        "../../common/rgb_led.c"
        "../../common/temp_sensor.c"
        "../../common/wifi.c"
        "../../common/slcan.c"
                    
    INCLUDE_DIRS 
        "."
        "../../common"
    )

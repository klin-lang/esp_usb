set(KLIN_C "${CMAKE_SOURCE_DIR}/out/cdc.c")
set(USB_C "${CMAKE_SOURCE_DIR}/../../esp_usb/cdc_idf.c")

if(NOT EXISTS "${KLIN_C}")
    message(FATAL_ERROR
        "Missing ${KLIN_C}. From examples/cdc_s3 run: make emit KLIN=...")
endif()

idf_component_register(
    SRCS "app_main.c" "${KLIN_C}" "${USB_C}"
    INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}/out" "${CMAKE_SOURCE_DIR}/../../esp_usb"
    REQUIRES freertos
)

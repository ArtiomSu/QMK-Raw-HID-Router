#include "raw_hid_client.h"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if(record->event.pressed){ // most are only using event pressed
    switch (keycode) {
        case RAW_HID_M_1:
            SEND_STRING("about to send hid. ");
            simple_hid_test_send_key(&trackball, 0x0a);
            SEND_STRING("send hid. ");
        case RAW_HID_TB_DPI_UP:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_DPI, 0x01);
        break;
        case RAW_HID_TB_DPI_DOWN:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_DPI, 0x02);
        break;
        case RAW_HID_TB_DPI_DEFAULT:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_DPI, 0x03);
        break;
        case RAW_HID_TB_SCROLL_UP:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_SCROLL, 0x01);
        break;
        case RAW_HID_TB_SCROLL_DOWN:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_SCROLL, 0x02);
        break;
        case RAW_HID_TB_SCROLL_DEFAULT:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_SCROLL, 0x03);
        break;
        case RAW_HID_TB_SCROLL_INVERT:
            raw_hid_set_setting(&trackball, HID_RAW_TB_S_SCROLL, 0x04);
        break;
    }
    }

    switch (keycode) {
        case RAW_HID_TB_DRAG_SCROLL:
            raw_hid_custom_key(&trackball, 0x01, record->event.pressed);
        break;
        return false;
    }

}
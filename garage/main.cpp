/*
 * Linear MegaCode Garage external app for PortaPack Mayhem.
 *
 * Intended for enrollment as a NEW transmitter on a garage opener
 * that the user is authorized to operate.
 */

#include "ui.hpp"
#include "ui_garage.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::garage {

void initialize_app(ui::NavigationView& nav) {
    nav.push<GarageView>();
}

}  // namespace ui::external_app::garage

extern "C" {

__attribute__((section(".external_app.app_garage.application_information"), used))
application_information_t _application_information_garage = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::garage::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "Garage",
    /*.bitmap_data = */ {
        0x00, 0x00,
        0xF8, 0x1F,
        0x0C, 0x30,
        0x06, 0x60,
        0x03, 0xC0,
        0xFF, 0xFF,
        0x01, 0x80,
        0x7D, 0xBE,
        0x45, 0xA2,
        0x45, 0xA2,
        0x45, 0xA2,
        0x45, 0xA2,
        0x7D, 0xBE,
        0x01, 0x80,
        0xFF, 0xFF,
        0x00, 0x00,
    },
    /*.icon_color = */ ui::Color::green().v,
    /*.menu_location = */ app_location_t::TX,
    /*.desired_menu_position = */ -1,

    // Reuse Mayhem's generic OOK M4 baseband image.
    /*.m4_app_tag = */ {'P', 'O', 'O', 'K'},
    /*.m4_app_offset = */ 0x00000000,
};

}

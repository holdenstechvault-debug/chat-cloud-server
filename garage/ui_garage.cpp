/*
 * Linear MegaCode Garage external app for PortaPack Mayhem.
 */

#include "ui_garage.hpp"

#include "baseband_api.hpp"
#include "string_format.hpp"

using namespace portapack;
using namespace encoders;

namespace ui::external_app::garage {

GarageView::GarageView(NavigationView& nav)
    : nav_{nav} {
    baseband::run_image(portapack::spi_flash::image_tag_ook);

    add_children({
        &labels,
        &text_identity,
        &button_1,
        &button_2,
        &button_3,
        &text_status,
        &progressbar
    });

    text_identity.set(
        "FC " + to_string_dec_uint(kFacility) +
        "  TX " + to_string_dec_uint(kTransmitter));

    transmitter_model.set_target_frequency(kFrequency);
    transmitter_model.set_rf_amp(false);
    transmitter_model.set_tx_gain(0);

    button_1.on_select = [this](Button&) {
        start_tx(1);
    };
    button_2.on_select = [this](Button&) {
        start_tx(2);
    };
    button_3.on_select = [this](Button&) {
        start_tx(3);
    };
}

GarageView::~GarageView() {
    baseband::kill_ook();
    transmitter_model.disable();
    baseband::shutdown();
}

void GarageView::focus() {
    button_1.focus();
}

size_t GarageView::generate_frame(uint8_t button) {
    size_t bitstream_length = 0;

    const uint32_t key =
        (1UL << 23) |
        (static_cast<uint32_t>(kFacility) << 19) |
        (static_cast<uint32_t>(kTransmitter) << 3) |
        (button & 0x07U);

    // MegaCode sends the 24-bit key MSB first.
    // Each data bit occupies six 1 ms OOK slices:
    //   data 0 -> 001000
    //   data 1 -> 000001
    for (uint32_t i = 0; i < 24; ++i) {
        const bool bit = (key >> (23 - i)) & 1U;
        bitstream_append(
            bitstream_length,
            6,
            bit ? 0b000001U : 0b001000U);
    }

    return bitstream_length;
}

void GarageView::start_tx(uint8_t button) {
    baseband::kill_ook();
    transmitter_model.disable();

    active_button_ = button;
    const size_t bitstream_length = generate_frame(button);

    progressbar.set_max(kRepeats - 1);
    progressbar.set_value(0);
    text_status.set("Sending button " + to_string_dec_uint(button) + "...");

    transmitter_model.enable();

    baseband::set_ook_data(
        bitstream_length,
        kSamplesPerMs,
        kRepeats,
        kInterframeGapMs);
}

void GarageView::stop_tx() {
    baseband::kill_ook();
    transmitter_model.disable();
    progressbar.set_value(0);
    text_status.set("Ready");
}

void GarageView::on_tx_progress(uint32_t progress, bool done) {
    if (done) {
        transmitter_model.disable();
        progressbar.set_value(0);
        text_status.set("Button " + to_string_dec_uint(active_button_) + " sent");
        return;
    }

    progressbar.set_value(progress);
    text_status.set(
        "B" + to_string_dec_uint(active_button_) + " " +
        to_string_dec_uint(progress + 1) + "/" +
        to_string_dec_uint(kRepeats));
}

}  // namespace ui::external_app::garage

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
        &text_status,
        &progressbar,
        &tx_view
    });

    text_identity.set(
        "FC " + to_string_dec_uint(kFacility) +
        "  TX " + to_string_dec_uint(kTransmitter) +
        "  KEY " + to_string_hex(kMegaCodeKey, 6));

    // Conservative defaults. The amp stays off. TX gain can still be
    // adjusted in the standard Mayhem transmitter controls if required.
    transmitter_model.set_target_frequency(kFrequency);
    transmitter_model.set_rf_amp(false);
    transmitter_model.set_tx_gain(0);

    tx_view.on_start = [this]() {
        start_tx();
    };

    tx_view.on_stop = [this]() {
        baseband::kill_ook();
        stop_tx();
    };
}

GarageView::~GarageView() {
    baseband::kill_ook();
    transmitter_model.disable();
    baseband::shutdown();
}

void GarageView::focus() {
    tx_view.focus();
}

size_t GarageView::generate_frame() {
    size_t bitstream_length = 0;

    // MegaCode sends the 24-bit key MSB first.
    //
    // Each data bit occupies six 1 ms OOK slices and contains exactly
    // one 1 ms RF pulse:
    //
    //   data 0 -> 001000
    //   data 1 -> 000001
    //
    // The generic Mayhem OOK baseband repeats this frame with a 9 ms
    // all-zero pause between repetitions.
    for (uint32_t i = 0; i < 24; ++i) {
        const bool bit = (kMegaCodeKey >> (23 - i)) & 1U;
        bitstream_append(
            bitstream_length,
            6,
            bit ? 0b000001U : 0b001000U);
    }

    return bitstream_length;
}

void GarageView::start_tx() {
    const size_t bitstream_length = generate_frame();

    progressbar.set_max(kRepeats - 1);
    progressbar.set_value(0);
    text_status.set("Transmitting...");
    tx_view.set_transmitting(true);

    transmitter_model.enable();

    baseband::set_ook_data(
        bitstream_length,
        kSamplesPerMs,
        kRepeats,
        kInterframeGapMs);
}

void GarageView::stop_tx() {
    transmitter_model.disable();
    progressbar.set_value(0);
    tx_view.set_transmitting(false);
    text_status.set("Ready");
}

void GarageView::on_tx_progress(uint32_t progress, bool done) {
    if (done) {
        transmitter_model.disable();
        progressbar.set_value(0);
        tx_view.set_transmitting(false);
        text_status.set("Done");
        return;
    }

    progressbar.set_value(progress);
    text_status.set(
        "Sending " + to_string_dec_uint(progress + 1) +
        "/" + to_string_dec_uint(kRepeats));
}

}  // namespace ui::external_app::garage

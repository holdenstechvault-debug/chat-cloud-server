/*
 * Linear MegaCode Garage external app for PortaPack Mayhem.
 */

#ifndef __UI_LINEAR_GARAGE_H__
#define __UI_LINEAR_GARAGE_H__

#include "ui.hpp"
#include "ui_transmitter.hpp"
#include "transmitter_model.hpp"
#include "app_settings.hpp"
#include "radio_state.hpp"
#include "encoders.hpp"
#include "message.hpp"

namespace ui::external_app::garage {

class GarageView : public View {
   public:
    GarageView(NavigationView& nav);
    ~GarageView();

    void focus() override;
    std::string title() const override { return "Linear Garage"; }

   private:
    NavigationView& nav_;

    // Linear MegaCode is 318 MHz OOK. Keep the normal Mayhem OOK sample rate.
    TxRadioState radio_state_{
        318000000,
        1750000,
        OOK_SAMPLERATE
    };

    app_settings::SettingsManager settings_{
        "tx_linear_garage", app_settings::Mode::TX
    };

    static constexpr rf::Frequency kFrequency = 318000000;
    static constexpr uint32_t kSamplesPerMs = OOK_SAMPLERATE / 1000;
    static constexpr uint32_t kRepeats = 6;
    static constexpr uint32_t kInterframeGapMs = 9;

    // This package's own transmitter identity.
    // Key layout:
    //   bit 23: start bit = 1
    //   bits 22..19: facility code
    //   bits 18..3: transmitter number
    //   bits 2..0: button
    static constexpr uint8_t kFacility = 0;
    static constexpr uint16_t kTransmitter = 44734;
    static constexpr uint8_t kButton = 2;
    static constexpr uint32_t kMegaCodeKey =
        (1UL << 23) |
        (static_cast<uint32_t>(kFacility) << 19) |
        (static_cast<uint32_t>(kTransmitter) << 3) |
        kButton;

    size_t generate_frame();
    void start_tx();
    void stop_tx();
    void on_tx_progress(uint32_t progress, bool done);

    Labels labels{
        {{2 * 8, 1 * 16}, "Linear MegaCode Garage", Theme::getInstance()->fg_light->foreground},
        {{2 * 8, 2 * 16}, "318.000 MHz - learned ID", Theme::getInstance()->fg_light->foreground},
        {{2 * 8, 5 * 16}, "Press START to operate.", Theme::getInstance()->fg_light->foreground},
        {{2 * 8, 6 * 16}, "Enroll once with LEARN.", Theme::getInstance()->fg_light->foreground},
    };

    Text text_identity{
        {2 * 8, 3 * 16, 27 * 8, 16},
        "ID"
    };

    Text text_status{
        {2 * 8, UI_POS_Y_BOTTOM(7), 160, 16},
        "Ready"
    };

    ProgressBar progressbar{
        {2 * 8, UI_POS_Y_BOTTOM(6), UI_POS_WIDTH_REMAINING(4), 16}
    };

    // lock=true keeps frequency/bandwidth fixed while leaving gain controls available.
    TransmitterView tx_view{
        (int16_t)UI_POS_Y_BOTTOM(4),
        0,
        15,
        true
    };

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* const p) {
            const auto message =
                *reinterpret_cast<const TXProgressMessage*>(p);
            on_tx_progress(message.progress, message.done);
        }
    };
};

}  // namespace ui::external_app::garage

#endif

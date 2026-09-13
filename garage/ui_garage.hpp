/*
 * Linear MegaCode Garage external app for PortaPack Mayhem.
 */

#ifndef __UI_LINEAR_GARAGE_H__
#define __UI_LINEAR_GARAGE_H__

#include "ui.hpp"
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

    static constexpr uint8_t kFacility = 0;
    static constexpr uint16_t kTransmitter = 44734;

    size_t generate_frame(uint8_t button);
    void start_tx(uint8_t button);
    void stop_tx();
    void on_tx_progress(uint32_t progress, bool done);

    uint8_t active_button_{0};

    Labels labels{
        {{2 * 8, 1 * 16}, "Linear MegaCode Garage", Theme::getInstance()->fg_light->foreground},
        {{2 * 8, 2 * 16}, "318.000 MHz - learned ID", Theme::getInstance()->fg_light->foreground},
        {{2 * 8, 4 * 16}, "Try command buttons:", Theme::getInstance()->fg_light->foreground},
    };

    Text text_identity{
        {2 * 8, 3 * 16, 27 * 8, 16},
        "ID"
    };

    Button button_1{
        {16, 5 * 16, 64, 32},
        "1"
    };

    Button button_2{
        {88, 5 * 16, 64, 32},
        "2"
    };

    Button button_3{
        {160, 5 * 16, 64, 32},
        "3"
    };

    Text text_status{
        {2 * 8, 8 * 16, 27 * 8, 16},
        "Ready"
    };

    ProgressBar progressbar{
        {2 * 8, 9 * 16, UI_POS_WIDTH_REMAINING(4), 16}
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

#pragma once
#include <JuceHeader.h>

class MiniMetersLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MiniMetersLookAndFeel() {
        minimeters_font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::SpaceMonoRegular_ttf, BinaryData::SpaceMonoRegular_ttfSize));
    }
    juce::Font minimeters_font;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& background_colour, bool, bool is_button_down) override {
        if (is_button_down) {
            g.setColour({ 172 / 2, 192 / 2, 222 / 2 });
        } else {
            g.setColour({ 172, 192, 222 });
        }
        g.fillRect(button.getLocalBounds());
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool is_button_down) override {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(minimeters_font.withHeight(24));
        if (is_button_down) {
            g.setColour({ 255, 255, 255 });
        } else {
            g.setColour({ 0, 0, 0 });
        }

        auto y_indent = juce::jmin(4, button.proportionOfHeight(0.3f));
        auto corner_size = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        auto font_height = juce::roundToInt(font.getHeight() * 0.6f);
        auto left_indent = juce::jmin(font_height, 2 + corner_size / (button.isConnectedOnLeft() ? 4 : 2));
        auto right_indent = juce::jmin(font_height, 2 + corner_size / (button.isConnectedOnRight() ? 4 : 2));
        auto text_width = button.getWidth() - left_indent - right_indent;

        auto edge = 4;
        if (text_width > 0)
            g.drawFittedText(button.getButtonText(),
                             left_indent, y_indent, text_width, button.getHeight() - y_indent * 2 - edge,
                             juce::Justification::centred, 2);
    }
};

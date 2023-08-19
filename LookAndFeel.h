#pragma once
#include "BinaryData.h"
#include <JuceHeader.h>

class MiniMetersLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MiniMetersLookAndFeel() {
        minimeters_font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::SpaceMonoRegular_ttf, BinaryData::SpaceMonoRegular_ttfSize));
    }
    juce::Font minimeters_font;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override {
        if (isButtonDown) {
            g.setColour({ 172 / 2, 192 / 2, 222 / 2 });
        } else {
            g.setColour({ 172, 192, 222 });
        }
        g.fillRect(button.getLocalBounds());
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool isButtonDown) override {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(minimeters_font.withHeight(24));
        if (isButtonDown) {
            g.setColour({ 255, 255, 255 });
        } else {
            g.setColour({ 0, 0, 0 });
        }

        auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
        auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
        auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        auto textWidth = button.getWidth() - leftIndent - rightIndent;

        auto edge = 4;
        if (textWidth > 0)
            g.drawFittedText(button.getButtonText(),
                             leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2 - edge,
                             juce::Justification::centred, 2);
    }
};

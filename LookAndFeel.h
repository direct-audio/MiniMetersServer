#pragma once
#include "Assets/SpaceMono-Bold.h"
#include <JuceHeader.h>

inline static const juce::Font& get_mm_font() {
    static juce::Font font(juce::Font(juce::Typeface::createSystemTypefaceFor(SpaceMono_Bold_ttf, SpaceMono_Bold_ttf_len)));
    return font;
}

class MiniMetersLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MiniMetersLookAndFeel() {}
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override {
        g.setColour({ 172, 192, 222 });
        g.fillRect(button.getLocalBounds());
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool isButtonDown) override {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(get_mm_font().withHeight(24));
        g.setColour({ 0, 0, 0 });

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

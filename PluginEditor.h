#pragma once
#include "LookAndFeel.h"
#include "PluginProcessor.h"

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
    MiniMetersLookAndFeel mm_look_and_feel;

public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::Image background;

private:
    juce::TextButton primary_instance_button { "Click here to set this instance as the primary." };
    juce::TextButton reset_button { "Reset" };
    juce::TextButton open_minimeters_button { "Open MiniMeters" };

    AudioPluginAudioProcessor& processorRef;

    bool m_minimeters_is_open = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

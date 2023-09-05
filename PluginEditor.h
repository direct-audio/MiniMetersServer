#pragma once
#include "LookAndFeel.h"
#include "PluginProcessor.h"
#include <thread>

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
    MiniMetersLookAndFeel m_mm_look_and_feel;

public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    juce::Image background;

private:
    juce::TextButton m_primary_instance_button { "Click here to set this instance as the primary." };
    juce::TextButton m_reset_button { "Reset" };

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

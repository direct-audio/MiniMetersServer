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
    juce::TextButton m_open_minimeters_button { "Open MiniMeters" };

    AudioPluginAudioProcessor& processorRef;

    std::atomic<bool> m_minimeters_is_open = false;

    std::atomic<bool> m_check_for_minimeters = true;
    std::thread m_minimeters_checker_thread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

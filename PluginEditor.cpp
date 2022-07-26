#include "PluginEditor.h"
#include "Assets/bg.h"
#include "PluginProcessor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p) {
    juce::ignoreUnused(processorRef);
    setLookAndFeel(&mm_look_and_feel);
    addAndMakeVisible(primary_instance_button);
    primary_instance_button.onClick = [&]() {
        processorRef.Server_MakePrimary();
        repaint();
    };
    setSize(800, 350);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    background = juce::ImageCache::getFromMemory(bg_png, bg_png_len);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.setColour(juce::Colours::white);
    g.setFont(mm_look_and_feel.minimeters_font.withHeight(24));
    if (processorRef.server_state == processorRef.StatePrimary) {
        g.drawFittedText("Currently sending audio to MiniMeters.",
                         juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight()),
                             juce::Justification::centred, 1);

        primary_instance_button.setVisible(false);
    } else if (processorRef.server_state == processorRef.StateNotPrimary) {
        g.drawFittedText("Another instance is sending audio to MiniMeters.",
                         juce::Rectangle<int>(305, -50, getWidth() - 305, getHeight()),
                             juce::Justification::centred, 1);

        primary_instance_button.setVisible(true);
    } else if (processorRef.server_state == processorRef.StateError) {
        g.drawFittedText("An error occurred while trying to access port 8422.\nPlease contact support.",
                         juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight()),
                             juce::Justification::centred, 2);

        primary_instance_button.setVisible(false);
    }
}

void AudioPluginAudioProcessorEditor::resized() {
    primary_instance_button.setBounds(juce::Rectangle<int>(305 + 30, getHeight() / 2 - 25, getWidth() - 305 - 30 - 30, 50));
}

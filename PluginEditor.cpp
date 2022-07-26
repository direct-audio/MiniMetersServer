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

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() { }

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    background = juce::ImageCache::getFromMemory(bg_png, bg_png_len);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.setColour(juce::Colours::white);
    g.setFont(get_mm_font().withHeight(24));
    if (processorRef.server_state == processorRef.StatePrimary) {
        g.drawFittedText("Currently sending audio to MiniMeters.", juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight()), juce::Justification::centred, 1);
        primary_instance_button.setVisible(false);
    } else if (processorRef.server_state == processorRef.StateNotPrimary) {
        g.drawFittedText("Another instance is sending audio to MiniMeters.", juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight()), juce::Justification::centred, 1);
        primary_instance_button.setVisible(true);
    }
}

void AudioPluginAudioProcessorEditor::resized() {
    const int x_padding = 25;
    primary_instance_button.setBounds(juce::Rectangle<int>(305 + 30, getHeight() / 2 + x_padding, getWidth() - 305 - 30 - x_padding * 2, 50));
}

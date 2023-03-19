#include "PluginEditor.h"
#include "Assets/bg.h"
#include "MiniMetersOpener.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p) {
    juce::ignoreUnused(processorRef);
    setLookAndFeel(&mm_look_and_feel);
    addAndMakeVisible(primary_instance_button);
    primary_instance_button.onClick = [&]() {
        //        processorRef.Server_MakePrimary();
        processorRef.ipc_make_primary();
        repaint();
    };

    addAndMakeVisible(reset_button);
    reset_button.onClick = [&]() {
        processorRef.ipc_setup();
        repaint();
    };

    m_minimeters_is_open = MiniMetersOpener::is_minimeters_running();
    open_minimeters_button.onClick = [&]() {
        MiniMetersOpener::launch_minimeters();
        m_minimeters_is_open = MiniMetersOpener::is_minimeters_running();
    };

    addAndMakeVisible(open_minimeters_button);
    setSize(800, 350);
    background = juce::ImageCache::getFromMemory(bg_png, bg_png_len);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    processorRef.editor_ptr = nullptr;
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    background = juce::ImageCache::getFromMemory(bg_png, bg_png_len);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.setColour(juce::Colours::white);
    g.setFont(mm_look_and_feel.minimeters_font.withHeight(24));
    if (processorRef.server_state == processorRef.StatePrimary) {
        g.drawFittedText("Currently set as primary instance.",
                         juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight()),
                         juce::Justification::centred, 1);

        primary_instance_button.setVisible(false);
    } else if (processorRef.server_state == processorRef.StateCannotFindMiniMeters) {
        g.drawFittedText("Cannot connect to MiniMeters. Please ensure the standalone MiniMeters instance is open.",
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

    if (processorRef.server_state == processorRef.StatePrimary
        && !m_minimeters_is_open) {
        open_minimeters_button.setVisible(true);
    } else {
        open_minimeters_button.setVisible(false);
    }

    {
        g.setColour(Colour(0.0f, 0.0f, 1.0f, 0.25f));
        g.drawText(juce::String(ProjectInfo::versionString), juce::Rectangle<int> { 15, 15, 100, 10 }, Justification::left);
    }
}

void AudioPluginAudioProcessorEditor::resized() {
    primary_instance_button.setBounds(juce::Rectangle<int>(305 + 30, getHeight() / 2 - 25, getWidth() - 305 - 30 - 30, 50));
    reset_button.setBounds(juce::Rectangle<int>(getWidth() - 15 - 70, 15, 70, 25));
    open_minimeters_button.setBounds(juce::Rectangle<int>(305 + 30 + 60, getHeight() / 2 + 50, getWidth() - 305 - 30 - 30 - 120, 50));
}

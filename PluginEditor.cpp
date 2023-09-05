#include "PluginEditor.h"
#include "BinaryData.h"
#include "MiniMetersOpener.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>
#include <atomic>
#include <chrono>
#include <thread>

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p) {
    juce::ignoreUnused(processorRef);
    setLookAndFeel(&m_mm_look_and_feel);
    addAndMakeVisible(m_primary_instance_button);
    m_primary_instance_button.onClick = [&]() {
        processorRef.ipc_make_primary();
        repaint();
    };

    addAndMakeVisible(m_reset_button);
    m_reset_button.onClick = [&]() {
        processorRef.ipc_setup();
        repaint();
    };

    setSize(800, 350);
    background = juce::ImageCache::getFromMemory(BinaryData::bg_png, BinaryData::bg_pngSize);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    processorRef.editor_ptr = nullptr;
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.setColour(juce::Colours::white);
    g.setFont(m_mm_look_and_feel.minimeters_font.withHeight(24));

    auto text_rect = juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight());

    bool draw_open_text = false;

    if (processorRef.server_state == processorRef.StatePrimary) {
        g.drawFittedText("Currently set as primary instance.",
                         text_rect,
                         juce::Justification::centred, 1);

        m_primary_instance_button.setVisible(false);
        draw_open_text = true;
    } else if (processorRef.server_state == AudioPluginAudioProcessor::StateCannotFindMiniMeters) {
        g.drawFittedText("Cannot connect to MiniMeters.\nPlease ensure the standalone MiniMeters instance is open.",
                         text_rect,
                         juce::Justification::centred, 2);

        m_primary_instance_button.setVisible(false);
        draw_open_text = false;
    } else if (processorRef.server_state == AudioPluginAudioProcessor::StateNotPrimary) {
        g.drawFittedText("Another instance is sending audio to MiniMeters.",
                         text_rect,
                         juce::Justification::centred, 1);

        m_primary_instance_button.setVisible(true);
        draw_open_text = true;
    } else if (processorRef.server_state == AudioPluginAudioProcessor::StateError) {
        g.drawFittedText("An error occurred while trying to access port 8422.\nPlease contact support.",
                         text_rect,
                         juce::Justification::centred, 2);

        m_primary_instance_button.setVisible(false);
        draw_open_text = false;
    }

    if (draw_open_text) {
        auto open_rect = text_rect.withTop(getHeight() / 2).withBottom(getHeight());

        g.setColour(Colour(0.0f, 0.0f, 1.0f, 0.5f));

#if defined(__APPLE__)
        g.drawFittedText("If MiniMeters is not open\nplease open it from the Applications Folder.",
#elif defined(_WIN32)
        g.drawFittedText("If MiniMeters is not open\nplease open it from the Start Menu.",
#elif defined(__linux__)
        g.drawFittedText("If MiniMeters is not open\nplease open it from the folder you installed the .AppImage.",
#endif
                         open_rect, juce::Justification::centred, 2);
    }

    {
        g.setColour(Colour(0.0f, 0.0f, 1.0f, 0.25f));
        g.drawText(juce::String(ProjectInfo::versionString), juce::Rectangle<int> { 15, 15, 100, 10 }, Justification::left);
    }
}

void AudioPluginAudioProcessorEditor::resized() {
    m_primary_instance_button.setBounds(juce::Rectangle<int>(305 + 30, getHeight() / 2 - 25, getWidth() - 305 - 30 - 30, 50));
    m_reset_button.setBounds(juce::Rectangle<int>(getWidth() - 15 - 70, 15, 70, 25));
}

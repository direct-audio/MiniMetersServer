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

    // Check if MiniMeters is running
    m_minimeters_is_open.store(MiniMetersOpener::is_minimeters_running(), std::memory_order_relaxed);

    m_minimeters_checker_thread = std::thread([this]() {
        while (m_check_for_minimeters.load(std::memory_order_relaxed) == true) {
            // Check once every second if MiniMeters has been opened.
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // Store the last state so we can compare.
            bool last_minimeters_state = m_minimeters_is_open;

            bool is_minimeters_open = MiniMetersOpener::is_minimeters_running();

            // I believe we may need to order this strongly so that the repaint
            // happens after this bool is updatad.
            m_minimeters_is_open = is_minimeters_open;

            // Repaint if the state changed.
            if (is_minimeters_open != last_minimeters_state) {
                // We already have a safe way to repaint in the plugin processor
                // so lets just use that.
                processorRef.triggerAsyncUpdate();
            }
        }
    });

    m_open_minimeters_button.onClick = [&]() {
        m_minimeters_is_open = MiniMetersOpener::launch_minimeters();
    };

    addAndMakeVisible(m_open_minimeters_button);
    setSize(800, 350);
    background = juce::ImageCache::getFromMemory(BinaryData::bg_png, BinaryData::bg_pngSize);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    // Stop checking if MiniMeters is open.
    m_check_for_minimeters.store(false, std::memory_order_relaxed);
    m_minimeters_checker_thread.join();

    processorRef.editor_ptr = nullptr;
    setLookAndFeel(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    g.setColour(juce::Colours::white);
    g.setFont(m_mm_look_and_feel.minimeters_font.withHeight(24));

    auto text_rect = juce::Rectangle<int>(305, 0, getWidth() - 305, getHeight());

    // MiniMeters does not appear to be open in this case.
    m_open_minimeters_button.setVisible(false);

    if (processorRef.server_state == AudioPluginAudioProcessor::StatePrimary && !m_minimeters_is_open) {
        m_primary_instance_button.setVisible(false);
#if defined(__APPLE__)
        g.drawFittedText("MiniMeters is not open.\nPlease open it from the Applications Folder.",
#elif defined(_WIN32)
        g.drawFittedText("MiniMeters is not open.\nPlease open it from the Start Menu.",
#elif defined(__linux__)
        g.drawFittedText("MiniMeters is not open.\nPlease open it from the folder you installed the .AppImage.",
#endif
                         text_rect, juce::Justification::centred, 2);
        // m_open_minimeters_button.setVisible(false);
    } else {
        if (processorRef.server_state == processorRef.StatePrimary) {
            g.drawFittedText("Currently set as primary instance.",
                             text_rect,
                             juce::Justification::centred, 1);

            m_primary_instance_button.setVisible(false);
        } else if (processorRef.server_state == AudioPluginAudioProcessor::StateCannotFindMiniMeters) {
            g.drawFittedText("Cannot connect to MiniMeters.\nPlease ensure the standalone MiniMeters instance is open.",
                             text_rect,
                             juce::Justification::centred, 2);

            m_primary_instance_button.setVisible(false);
        } else if (processorRef.server_state == AudioPluginAudioProcessor::StateNotPrimary) {
            g.drawFittedText("Another instance is sending audio to MiniMeters.",
                             text_rect,
                             juce::Justification::centred, 1);

            m_primary_instance_button.setVisible(true);
        } else if (processorRef.server_state == AudioPluginAudioProcessor::StateError) {
            g.drawFittedText("An error occurred while trying to access port 8422.\nPlease contact support.",
                             text_rect,
                             juce::Justification::centred, 2);

            m_primary_instance_button.setVisible(false);
        }
    }

    {
        g.setColour(Colour(0.0f, 0.0f, 1.0f, 0.25f));
        g.drawText(juce::String(ProjectInfo::versionString), juce::Rectangle<int> { 15, 15, 100, 10 }, Justification::left);
    }
}

void AudioPluginAudioProcessorEditor::resized() {
    m_primary_instance_button.setBounds(juce::Rectangle<int>(305 + 30, getHeight() / 2 - 25, getWidth() - 305 - 30 - 30, 50));
    m_reset_button.setBounds(juce::Rectangle<int>(getWidth() - 15 - 70, 15, 70, 25));
    m_open_minimeters_button.setBounds(juce::Rectangle<int>(305 + 30 + 60, getHeight() / 2 + 50, getWidth() - 305 - 30 - 30 - 120, 50));
}

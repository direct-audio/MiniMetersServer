#pragma once
#include "SharedMemory.h"
#include "miniaudio-dcec55f7b8b0b9b2ec602069ed2e19cde089be84/extras/speex_resampler/ma_speex_resampler.h"
#include "miniaudio-dcec55f7b8b0b9b2ec602069ed2e19cde089be84/miniaudio.h"
#include <JuceHeader.h>

// #include "httplib.h"

template <class T, size_t size, size_t n_consumers>
class CircleBuffer {
    int read_head[n_consumers] = { 0 };
    int write_head = 0;
    T buffer[size];
    bool is_full[n_consumers] = { false };

public:
    CircleBuffer() = default;
    void write(T input) {
        if (is_full[n_consumers])
            return;

        buffer[write_head] = input;
        write_head = (write_head + 1) % size;
        // FIXME: could be slow hee
        for (int i = 0; i < n_consumers; ++i) {
            if (read_head[i] == write_head) {
                is_full[i] = true;
            }
        }
    }
    template <size_t consumer>
    T read() {
        if (is_empty<consumer>())
            return T();

        T element = buffer[read_head[consumer]];
        read_head[consumer] = (read_head[consumer] + 1) % size;
        is_full[consumer] = false;
        return element;
    }
    template <size_t consumer>
    bool is_empty() {
        return read_head[consumer] == write_head && !is_full[consumer];
    }
};

class AudioPluginAudioProcessor : public juce::AudioProcessor, public juce::AsyncUpdater {
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    juce::AudioProcessorEditor* editor_ptr = nullptr;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    // TODO: Have proper handshake between MiniMeters and the plugin.
    enum ServerState {
        StatePrimary = 0,
        StateNotPrimary,
        StateCannotFindMiniMeters,
        StateError,
    };

    std::atomic<ServerState> server_state;
    void handleAsyncUpdate() override {
        if (editor_ptr)
            editor_ptr->repaint();
    }

    void set_button_state(ServerState s) {
        server_state = s;
        triggerAsyncUpdate();
    }

    ServerState get_button_state() const {
        return server_state;
    }

    void ipc_make_primary();
    void ipc_setup();

private:
    std::array<float, 65536 * 2> m_interleaved_audio;

    int64_t m_uuid_hash = 0;

    IPC_TYPE* m_ipc_ptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};

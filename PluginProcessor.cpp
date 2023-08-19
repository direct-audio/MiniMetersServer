#include <JuceHeader.h>

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "json.hpp"

#include <fcntl.h>
#include <random>
#include <stdio.h>

#ifndef WIN32
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include "windows.h"
#endif

void AudioPluginAudioProcessor::setup_resampler(double sample_rate) {
    m_resampler_sr = sample_rate;
    m_resampler_config = ma_resampler_config_init(ma_format_f32, 2, m_resampler_sr, 44100, ma_resample_algorithm_speex);
    m_resampler_config.speex.quality = 5;

    ma_result result = ma_resampler_init(&m_resampler_config, &m_resampler);
    if (result != MA_SUCCESS) {
        std::cout << "Error loading resampler: " << result << std::endl;
    }
}
void AudioPluginAudioProcessor::close_resampler() {
    //  ma_resampler_uninit(&m_resampler);
}

// Note: This is not really a valid UUID. I believe this has 64bits of
//       randomness which should be sufficient for this plugin, but we might
//       want to replace it in the future.
static std::string get_uuid() {
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char* v = "0123456789abcdef";
    const int dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i : dash) {
        if (i)
            res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

// Note: This is likely increasing our collision rate, but this is faster to
//       compare when the plugin is actually running.
int64_t compute_hash(std::string const& s) {
    const int p = 31;
    const int m = 1e9 + 9;
    int64_t hash_value = 0;
    int64_t p_pow = 1;
    for (char c : s) {
        hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash_value;
}

void AudioPluginAudioProcessor::ipc_make_primary() {
    m_ipc_ptr->current_id = m_uuid_hash;
    set_button_state(StatePrimary);
}

void AudioPluginAudioProcessor::ipc_setup() {
#ifndef WIN32
    int fd = shm_open(IPC_FILE_NAME, O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
        ftruncate(fd, IPC_BLOCK_SIZE);
    } else {
        // We failed to create here so lets try not to create and instead open the already existing.
        fd = shm_open(IPC_FILE_NAME, O_RDWR, 0666);
    }

    m_ipc_ptr = nullptr;
    m_ipc_ptr = (IPC_TYPE*)mmap(nullptr, IPC_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (m_ipc_ptr == nullptr) {
        printf("Error when getting mmap()\n");
        return;
    }
#else
    HANDLE fd;
    // Check if fd exists
    fd = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        IPC_BLOCK_SIZE,
        IPC_FILE_NAME);
    if (fd == nullptr) {
        // Failure
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
        }
        char msg[999];
        sprintf(msg, "ERROR: %lu", GetLastError());
        MessageBox(0, msg, "Title", MB_OK);
        return;
    }
    m_ipc_ptr = (IPC_TYPE*)MapViewOfFile(fd,
                                         FILE_MAP_ALL_ACCESS,
                                         0,
                                         0,
                                         IPC_BLOCK_SIZE);
    if (m_ipc_ptr == nullptr) {
        MessageBox(0, "Error when getting MapViewOfFile()", "Title", MB_OK);
        return;
    }
#endif

    ipc_make_primary();
}

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    m_uuid_hash = compute_hash(get_uuid());
    m_resampler_sr = getSampleRate();
    setup_resampler(m_resampler_sr);
    ipc_setup();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
    m_ipc_ptr->current_id = 0;
    editor_ptr = nullptr;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
    return false;
}

bool AudioPluginAudioProcessor::producesMidi() const {
    return false;
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
    return false;
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return "None";
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName) {
    juce::ignoreUnused(index, newName);
}

void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    if (m_resampler_sr != sampleRate) {
        close_resampler();
        setup_resampler(sampleRate);
    }
}

void AudioPluginAudioProcessor::releaseResources() { }

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals no_denormals;
    int n_input_channels = getTotalNumInputChannels();
    int n_output_channels = getTotalNumOutputChannels();

    if (m_uuid_hash != m_ipc_ptr->current_id) {
        if (get_button_state() != ServerState::StateNotPrimary) {
            set_button_state(ServerState::StateNotPrimary);
        }
        return;
    }

    // blank unused channels
    for (int i = n_input_channels; i < n_output_channels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const int n_samples = buffer.getNumSamples();

    // The DAW should really never send a buffer the larger than 65536 samples, but in the case that it does.
    if (n_samples >= pre_resampling_input.size())
        return;

    if (n_input_channels == 1) {
        auto ptr_l = buffer.getReadPointer(0);

        for (int i = 0; i < n_samples * 2; i += 2) {
            pre_resampling_input[i + 0] = *(ptr_l + (i / 2));
            pre_resampling_input[i + 1] = *(ptr_l + (i / 2));
        }
    } else if (n_input_channels == 2) {
        auto ptr_l = buffer.getReadPointer(0);
        auto ptr_r = buffer.getReadPointer(1);

        for (int i = 0; i < n_samples * 2; i += 2) {
            pre_resampling_input[i + 0] = *(ptr_l + (i / 2));
            pre_resampling_input[i + 1] = *(ptr_r + (i / 2));
        }
    }

    ma_uint64 frame_count_in = n_samples;
    ma_uint64 frame_count_out = 44100;
    ma_resampler_process_pcm_frames(&m_resampler, &pre_resampling_input, &frame_count_in, &resampled_output, &frame_count_out);
    // This temporary only exists for a short time, but the pointer to the
    // data will be fully read by the time the loop continues.

    for (size_t i = 0; i < frame_count_out * 2; i++) {
        m_ipc_ptr->buffer.write(resampled_output[i]);
    }
    m_ipc_ptr->sample_rate = getSampleRate();
    m_ipc_ptr->block_size = getBlockSize();
}

bool AudioPluginAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    editor_ptr = new AudioPluginAudioProcessorEditor(*this);
    return editor_ptr;
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}

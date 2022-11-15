#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "json.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <random>

#ifndef WIN32
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include "windows.h"
#endif

void AudioPluginAudioProcessor::Server_Setup() {
    /*
    httplib::Client cli("localhost", 8422);
    cli.set_read_timeout(1);
    cli.set_connection_timeout(1);
    // Server exists at localhost:8422
    auto res = cli.Get("/check");
    if (res) {
        if (res->status == 200) {
            // /check exists and connection is good.
            // we are certain that another instance of the plugin exists.
            set_button_state(StateNotPrimary);
            return;
        } else if (res->status == 404) {
            // Server exists but /check does not.
            set_button_state(StateError);
            return;
        }
    }
    // No server exists. We are the primary instance.
    set_button_state(StatePrimary);
    Server_Start();
     */
}

bool AudioPluginAudioProcessor::Server_StopOtherInstance() {
    /*
    httplib::Client cli("localhost", 8422);

    cli.set_read_timeout(1);
    cli.set_connection_timeout(1);
    auto res = cli.Get("/stop");
    if (!res)
        return false;

    if (res->status == 200) {
        // server has received the call and is stopping.
        set_button_state(StatePrimary);
        return true;
    } else if (res->status == 404) {
        // server is likely not a MiniMetersServer instance.
        set_button_state(StateError);
        return false;
    }
    return false;
     */
    return false;
}

void AudioPluginAudioProcessor::Server_MakePrimary() {
    if (Server_StopOtherInstance()) {
        Server_Start();
    }
}

void AudioPluginAudioProcessor::Server_Start() {
    /*
    b.resize(65536);
    std::thread([this]() {
        server_has_finished = false;
        svr.set_read_timeout(0, 10000); // 10ms
        svr.new_task_queue = [] { return new httplib::ThreadPool(1); };
        svr.Get("/hi", [&](const httplib::Request&, httplib::Response& res) {
            while (!mm_buffer.is_empty<0>()) {
                sprintf(str, "%fn", mm_buffer.read<0>());
                b += str;
            }
            res.set_content(b.c_str(), "text/plain");
            b = "";
            set_button_state(StatePrimary);
        });

        svr.Get("/check", [&](const httplib::Request&, httplib::Response& res) {
            PluginHostType p;
            auto host_name = p.getHostDescription();
            res.set_content(host_name, "text/plain");
        });

        svr.Get("/data", [&](const httplib::Request& req, httplib::Response& res) {
            auto version_str = req.get_param_value("version");

            PluginHostType p;
            auto block_size = getBlockSize();
            auto sr = getSampleRate();
            auto host_name = p.getHostDescription();

            if (!version_str.empty()) {
                nlohmann::json j;
                j["block_size"] = block_size;
                j["sr"] = sr;
                j["host_name"] = host_name;
                res.set_content(j.dump(4), "text/plain");
            } else {
                // Versions prior to 0.8.4
                res.set_content(std::string(host_name) + '\n' + std::to_string(block_size), "text/plain");
            }
        });

        svr.Get("/stop", [&](const httplib::Request&, httplib::Response& res) {
            svr.stop();
            set_button_state(StateNotPrimary);
        });

        svr.listen("0.0.0.0", 8422);
        server_has_finished = true;
    }).detach();
     */
}

void AudioPluginAudioProcessor::SetupResampler(double sample_rate) {
    resamplerSampleRate = sample_rate;
    config = ma_resampler_config_init(ma_format_f32, 2, resamplerSampleRate, 44100, ma_resample_algorithm_speex);
    config.speex.quality = 5;
    ma_result result = ma_resampler_init(&config, &resampler);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR: " << result << std::endl;
    }
}
void AudioPluginAudioProcessor::CloseResampler() {
//        ma_resampler_uninit(&resampler);
}

// Note: This is not really a valid UUID. I believe this has 64bits of
//       randomness which should be sufficient for this plugin, but we
//       might want to replace it in the future.
std::string get_uuid() {
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
// Note:: This is likely increasing our collision rate, but this is faster to
//        compare when the plugin is actually running.
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
    ptr->current_id = uuid_hash;
    set_button_state(StatePrimary);
}

void AudioPluginAudioProcessor::ipc_setup() {
#ifndef WIN32
    int fd = shm_open(IPC_FILE_NAME, O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
        ftruncate(fd, BLOCK_SIZE);
    } else {
        // We failed to create here so lets try not to create and instead open the already existing.
        fd = shm_open(IPC_FILE_NAME, O_RDWR, 0666);
    }

    ptr = nullptr;
    ptr = (IPC_TYPE*)mmap(nullptr, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == nullptr) {
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
    ptr = (IPC_TYPE*)MapViewOfFile(fd,
                                   FILE_MAP_ALL_ACCESS,
                                   0,
                                   0,
                                   IPC_BLOCK_SIZE);
    if (ptr == nullptr) {
        MessageBox(0, "Error when getting MapViewOfFile()", "Title", MB_OK);
        return;
    }
#endif

    ipc_make_primary();
}

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ) {
    uuid_hash = compute_hash(get_uuid());
    resamplerSampleRate = getSampleRate();
    SetupResampler(resamplerSampleRate);
    //    Server_Setup();
    ipc_setup();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
    editor_ptr = nullptr;
    /*
    if (svr.is_running()) {
        svr.stop();
        while (!server_has_finished) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
     */
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
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
    if (resamplerSampleRate != sampleRate) {
        CloseResampler();
        SetupResampler(sampleRate);
    }
}

void AudioPluginAudioProcessor::releaseResources() {
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    if (uuid_hash != ptr->current_id) {
        if (get_button_state() != ServerState::StateNotPrimary) {
            set_button_state(ServerState::StateNotPrimary);
        }
        return;
    }

    // blank unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    const int n_samples = buffer.getNumSamples();

    // The DAW should really never send a buffer the larger than 65536 samples, but in the case that it does.
    if (n_samples >= pre_resampling_input.size())
        return;

    if (totalNumInputChannels == 1) {
        auto ptr_l = buffer.getReadPointer(0);

        for (int i = 0; i < n_samples * 2; i += 2) {
            pre_resampling_input[i + 0] = *(ptr_l + (i / 2));
            pre_resampling_input[i + 1] = *(ptr_l + (i / 2));
        }
    }

    if (totalNumInputChannels == 2) {
        auto ptr_l = buffer.getReadPointer(0);
        auto ptr_r = buffer.getReadPointer(1);

        for (int i = 0; i < n_samples * 2; i += 2) {
            pre_resampling_input[i + 0] = *(ptr_l + (i / 2));
            pre_resampling_input[i + 1] = *(ptr_r + (i / 2));
        }
    }

    ma_uint64 frame_count_in = n_samples;
    ma_uint64 frame_count_out = 44100;
    ma_resampler_process_pcm_frames(&resampler, &pre_resampling_input, &frame_count_in, &resampled_output, &frame_count_out);
    // This temporary only exists for a short time, but the pointer to the
    // data will be fully read by the time the loop continues.

    for (size_t i = 0; i < frame_count_out * 2; i++) {
        ptr->buffer.write(resampled_output[i]);
    }
    ptr->sample_rate = getSampleRate();
    ptr->block_size = getBlockSize();
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

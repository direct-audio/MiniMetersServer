#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "json.hpp"
#include <JuceHeader.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void AudioPluginAudioProcessor::Server_Setup() {
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
}

bool AudioPluginAudioProcessor::Server_StopOtherInstance() {
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
}

void AudioPluginAudioProcessor::Server_MakePrimary() {
    if (Server_StopOtherInstance()) {
        Server_Start();
    }
}

void AudioPluginAudioProcessor::Server_Start() {
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
}

void AudioPluginAudioProcessor::SetupResampler(double sample_rate) {
    resamplerSampleRate = sample_rate;
    config = ma_resampler_config_init(ma_format_f32, 2, resamplerSampleRate, 44100, ma_resample_algorithm_linear);
    ma_result result = ma_resampler_init(&config, nullptr, &resampler);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR: " << result << std::endl;
    }
}
void AudioPluginAudioProcessor::CloseResampler() { ma_resampler_uninit(&resampler, nullptr); }

void AudioPluginAudioProcessor::ipc_setup() {
    sem_prod = sem_open("/producer", 0);
    if (sem_prod == SEM_FAILED) {
        perror("sem_open/producer");
        exit(EXIT_FAILURE);
    }

    sem_cons = sem_open("/consumer", 0);
    if (sem_cons == SEM_FAILED) {
        perror("sem_open/consumer");
        exit(EXIT_FAILURE);
    }
    block = reinterpret_cast<IPC_TYPE *>(attach_memory_block("/home/joe/ipc1/writeshmem.c", BLOCK_SIZE));
//    block = new IPC_TYPE;
    if (block == nullptr) {
        printf("COULD NOT GET BLOCK\n");
    }
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

    resamplerSampleRate = getSampleRate();
    SetupResampler(resamplerSampleRate);
    Server_Setup();
    ipc_setup();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
    editor_ptr = nullptr;
    if (svr.is_running()) {
        svr.stop();
        while (server_has_finished == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
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

    // blank unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    const int n_samples = buffer.getNumSamples();

    // The DAW should really never send a buffer the larger than 65536 samples, but in the case that it does.
    if (n_samples >= 4096)
        return;
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

    // TODO: this shuold be wrapped in a ring buffer that is placed in the
    //       plugin's class members. Then we can send the pointer to the
    //       ring buffer to the receiver so it can safely call that.
//    MM::IpcChunk temp{};
//    for (size_t i = 0; i < 512; i++) {
//        temp.buffer[i] = resampled_output[i];
//    }

    static int i = 0;
//    (*block).buffer[0] = i;
    block->write(i);
    i++;

    sem_wait(sem_cons);
//    block = &ipc_audio_buffer;
//    *block = i;

    sem_post(sem_prod);
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

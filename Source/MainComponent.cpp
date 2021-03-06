#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "juce_gui_basics\widgets\juce_Slider.h"
#include "juce_gui_basics\widgets\juce_Label.h"

#include <limits>

//==============================================================================
/*
This component lives inside our window, and this is where you should put all
your controls and content.
*/
class MainContentComponent : public AudioAppComponent, public Slider::Listener, public TextButton::Listener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize(480, 140);

        // specify the number of input and output channels that we want to open
        setAudioChannels(2, 2);

        // volume slider
        addAndMakeVisible(volumeSlider);
        volumeSlider.setRange(-96, 6);
        volumeSlider.setTextValueSuffix(" db");
        volumeSlider.setValue(-6);
        volumeSlider.addListener(this);
        //volumeSlider.setSkewFactorFromMidPoint(0.5);

        volumeLabel.setText("Volume", dontSendNotification);
        volumeLabel.attachToComponent(&volumeSlider, true);

        // phase slider
        addAndMakeVisible(phaseSlider);
        phaseSlider.setRange(0.0, 1.0);
        phaseSlider.setTextValueSuffix(" ~");
        phaseSlider.setValue(0.0);
        phaseSlider.addListener(this);

        phaseLabel.setText("Phase", dontSendNotification);
        phaseLabel.attachToComponent(&phaseSlider, true);

        // freq slider
        addAndMakeVisible(freqSlider);
        freqSlider.setRange(10, 22000);
        freqSlider.setTextValueSuffix(" Hz");
        freqSlider.setValue(500.0);
        freqSlider.addListener(this);
        freqSlider.setSkewFactorFromMidPoint(500);

        freqLabel.setText("Freq", dontSendNotification);
        freqLabel.attachToComponent(&freqSlider, true);

        // mute button
        addAndMakeVisible(m_muteButton);
        m_muteButton.setButtonText("Mute");
        m_muteButton.addListener(this);
        m_muteButton.setEnabled(true);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void buttonClicked(Button* button) override
    {
        if (button == &m_muteButton) {
            m_mute = !m_mute;
        }
    }

    void sliderValueChanged(Slider *slider) {
        if (slider == &volumeSlider) {
            m_amplitude = pow(10, ((float)volumeSlider.getValue() / 20.0));
        }

        if (slider == &freqSlider) {
            m_frequency = (float)freqSlider.getValue();
        }

        if (slider == &phaseSlider) {
            m_phase = (float)phaseSlider.getValue();
        }
    }

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.
        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.
        // For more details, see the help for AudioProcessor::prepareToPlay()

        m_amplitude = 0.5;
        m_frequency = 500.0;
        m_phase = 0.0;
        m_time = 0.0;
        m_deltaTime = 1 / sampleRate;
    }

    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        if (m_time >= std::numeric_limits<float>::max()) {
            m_time = 0.0;
        }

        float *monoBuffer = new float[bufferToFill.numSamples];
        
        if (m_mute) return bufferToFill.clearActiveBufferRegion();

        // generate sin wave in mono
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
            float value = m_amplitude * sin(2 * double_Pi * m_frequency * m_time + m_phase);

            monoBuffer[sample] = value;
            m_time += m_deltaTime;
        }

        // iterate over all available output channels
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            // Get a pointer to the start sample in the buffer for this audio output channel
            float* const buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

            for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
                buffer[sample] = monoBuffer[sample];
            }
        }
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.
        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //==============================================================================
    void paint(Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(Colours::white);
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        const int sliderLeft = 50;
        volumeSlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
        phaseSlider.setBounds(sliderLeft, 50, getWidth() - sliderLeft - 10, 20);
        freqSlider.setBounds(sliderLeft, 80, getWidth() - sliderLeft - 10, 20);
        m_muteButton.setBounds(10, 110, getWidth() - 20, 20);
    }

private:
    //==============================================================================

    // Your private member variables go here...
    float m_amplitude;
    float m_frequency;
    float m_phase;
    float m_time;
    float m_deltaTime;

    // GUI
    Slider volumeSlider;
    Slider freqSlider;
    Slider phaseSlider;
    Label volumeLabel;
    Label freqLabel;
    Label phaseLabel;

    TextButton m_muteButton;
    bool m_mute;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED

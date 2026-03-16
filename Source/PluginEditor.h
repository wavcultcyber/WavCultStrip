#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * Custom rotary knob look-and-feel for the channel strip.
 * Dark, modern aesthetic with accent colors per section.
 */
class StripLookAndFeel : public juce::LookAndFeel_V4
{
public:
    StripLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFE8A838));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2A2A2A));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xFFFFFFFF));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Background circle
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Outline
        g.setColour(juce::Colour(0xFF3A3A3A));
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // Arc (value indicator)
        auto arcRadius = radius - 3.0f;
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                          rotaryStartAngle, angle, true);
        auto colour = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(colour);
        g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        // Pointer
        juce::Path pointer;
        auto pointerLength = radius * 0.6f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colour(0xFFDDDDDD));
        g.fillPath(pointer);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

        g.setColour(button.getToggleState() ? juce::Colour(0xFF4CAF50) : juce::Colour(0xFF555555));
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(juce::Colours::white);
        g.setFont(11.0f);
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
    }
};

class WavCultStripEditor : public juce::AudioProcessorEditor,
                            private juce::Timer
{
public:
    WavCultStripEditor(WavCultStripProcessor&);
    ~WavCultStripEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    WavCultStripProcessor& processor;
    StripLookAndFeel stripLnF;

    void timerCallback() override;

    // Helper to create labeled knob
    struct LabeledKnob {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    // Logo
    juce::Image logoImage;

    // Section toggle buttons
    juce::ToggleButton gateOnBtn{"GATE"}, eqOnBtn{"EQ"}, compOnBtn{"COMP"}, satOnBtn{"SAT"},
                       reverbOnBtn{"VERB"}, delayOnBtn{"DLY"}, busCompOnBtn{"BUS"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        gateOnAtt, eqOnAtt, compOnAtt, satOnAtt, reverbOnAtt, delayOnAtt, busCompOnAtt;

    // Input/Output
    LabeledKnob inputGain, outputGain;

    // Noise Gate
    LabeledKnob gateThresh, gateAttack, gateRelease, gateHold, gateRange;

    // EQ knobs
    LabeledKnob eqLowGain, eqLowFreq, eqLMGain, eqLMFreq, eqLMQ;
    LabeledKnob eqHMGain, eqHMFreq, eqHMQ, eqHighGain, eqHighFreq;

    // Compressor knobs
    LabeledKnob compThresh, compRatio, compAttack, compRelease, compMakeup, compMix;

    // Saturation
    LabeledKnob satDrive, satMix;

    // Reverb
    LabeledKnob reverbAmt, reverbDecay, reverbPreDelay, reverbDamp;
    juce::ComboBox reverbModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverbModeAtt;

    // Delay
    LabeledKnob delayAmt, delayTime, delayFeedback, delayHighCut, delayLowCut;
    juce::ComboBox delayModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayModeAtt;

    // Bus Comp
    LabeledKnob busCompThresh, busCompRatio, busCompAttack, busCompRelease;
    LabeledKnob busCompMakeup, busCompMix, busCompSCHPF;
    juce::ToggleButton busCompAutoRelBtn{"AUTO"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> busCompAutoRelAtt;

    // GR meters
    float compGR = 0.0f, busGR = 0.0f;

    void setupKnob(LabeledKnob& knob, const juce::String& paramId,
                   const juce::String& name, juce::Colour colour);
    void addKnobToSection(LabeledKnob& knob);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavCultStripEditor)
};

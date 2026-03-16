#include "PluginEditor.h"
#include "BinaryData.h"

static const juce::Colour kBgColour        (0xFF121212);
static const juce::Colour kSectionBg       (0xFF1E1E1E);
static const juce::Colour kSectionBorder   (0xFF333333);
static const juce::Colour kTextColour      (0xFFCCCCCC);
static const juce::Colour kGateColour      (0xFFB0BEC5);  // Silver/Grey
static const juce::Colour kEQColour        (0xFF4FC3F7);  // Blue
static const juce::Colour kCompColour      (0xFFE8A838);  // Gold
static const juce::Colour kSatColour       (0xFFFF7043);  // Orange-Red
static const juce::Colour kReverbColour    (0xFF9C27B0);  // Purple
static const juce::Colour kDelayColour     (0xFF66BB6A);  // Green
static const juce::Colour kBusCompColour   (0xFFEF5350);  // Red

WavCultStripEditor::WavCultStripEditor(WavCultStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&stripLnF);
    setSize(960, 750);

    // Load logo from binary data
    logoImage = juce::ImageCache::getFromMemory(BinaryData::wavcult_logo_png,
                                                 BinaryData::wavcult_logo_pngSize);

    auto& apvts = processor.getAPVTS();

    // Noise Gate
    setupKnob(gateThresh,  "gateThresh",  "THRESH", kGateColour);
    setupKnob(gateAttack,  "gateAttack",  "ATK",    kGateColour);
    setupKnob(gateRelease, "gateRelease", "REL",    kGateColour);
    setupKnob(gateHold,    "gateHold",    "HOLD",   kGateColour);
    setupKnob(gateRange,   "gateRange",   "RANGE",  kGateColour);

    // Input/Output
    setupKnob(inputGain,  "inputGain",  "IN",  kTextColour);
    setupKnob(outputGain, "outputGain", "OUT", kTextColour);

    // EQ
    setupKnob(eqLowGain,  "eqLowGain",  "LOW",    kEQColour);
    setupKnob(eqLowFreq,  "eqLowFreq",  "FREQ",   kEQColour);
    setupKnob(eqLMGain,   "eqLMGain",   "LO-MID", kEQColour);
    setupKnob(eqLMFreq,   "eqLMFreq",   "FREQ",   kEQColour);
    setupKnob(eqLMQ,      "eqLMQ",      "Q",      kEQColour);
    setupKnob(eqHMGain,   "eqHMGain",   "HI-MID", kEQColour);
    setupKnob(eqHMFreq,   "eqHMFreq",   "FREQ",   kEQColour);
    setupKnob(eqHMQ,      "eqHMQ",      "Q",      kEQColour);
    setupKnob(eqHighGain, "eqHighGain", "HIGH",   kEQColour);
    setupKnob(eqHighFreq, "eqHighFreq", "FREQ",   kEQColour);

    // Compressor
    setupKnob(compThresh,  "compThresh",  "THRESH", kCompColour);
    setupKnob(compRatio,   "compRatio",   "RATIO",  kCompColour);
    setupKnob(compAttack,  "compAttack",  "ATK",    kCompColour);
    setupKnob(compRelease, "compRelease", "REL",    kCompColour);
    setupKnob(compMakeup,  "compMakeup",  "GAIN",   kCompColour);
    setupKnob(compMix,     "compMix",     "MIX",    kCompColour);

    // Saturation
    setupKnob(satDrive, "satDrive", "DRIVE", kSatColour);
    setupKnob(satMix,   "satMix",   "MIX",   kSatColour);

    // Reverb
    setupKnob(reverbAmt,      "reverbAmt",      "AMT",   kReverbColour);
    setupKnob(reverbDecay,    "reverbDecay",    "DECAY", kReverbColour);
    setupKnob(reverbPreDelay, "reverbPreDelay", "PRE-D", kReverbColour);
    setupKnob(reverbDamp,     "reverbDamp",     "DAMP",  kReverbColour);

    reverbModeBox.addItemList({"Plate", "Hall", "Room"}, 1);
    reverbModeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2A2A2A));
    reverbModeBox.setColour(juce::ComboBox::textColourId, kReverbColour);
    addAndMakeVisible(reverbModeBox);
    reverbModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "reverbMode", reverbModeBox);

    // Delay
    setupKnob(delayAmt,      "delayAmt",      "AMT",   kDelayColour);
    setupKnob(delayTime,     "delayTime",     "TIME",  kDelayColour);
    setupKnob(delayFeedback, "delayFeedback", "FDBK",  kDelayColour);
    setupKnob(delayHighCut,  "delayHighCut",  "HI-CUT",kDelayColour);
    setupKnob(delayLowCut,   "delayLowCut",   "LO-CUT",kDelayColour);

    delayModeBox.addItemList({"Stereo", "Ping-Pong", "Tape"}, 1);
    delayModeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2A2A2A));
    delayModeBox.setColour(juce::ComboBox::textColourId, kDelayColour);
    addAndMakeVisible(delayModeBox);
    delayModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "delayMode", delayModeBox);

    // Bus Comp
    setupKnob(busCompThresh,  "busCompThresh",  "THRESH", kBusCompColour);
    setupKnob(busCompRatio,   "busCompRatio",   "RATIO",  kBusCompColour);
    setupKnob(busCompAttack,  "busCompAttack",  "ATK",    kBusCompColour);
    setupKnob(busCompRelease, "busCompRelease", "REL",    kBusCompColour);
    setupKnob(busCompMakeup,  "busCompMakeup",  "GAIN",   kBusCompColour);
    setupKnob(busCompMix,     "busCompMix",     "MIX",    kBusCompColour);
    setupKnob(busCompSCHPF,   "busCompSCHPF",   "SC HPF", kBusCompColour);

    // Toggle buttons
    auto setupToggle = [&](juce::ToggleButton& btn, const juce::String& paramId,
                           std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& att) {
        addAndMakeVisible(btn);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramId, btn);
    };

    setupToggle(gateOnBtn,    "gateOn",    gateOnAtt);
    setupToggle(eqOnBtn,      "eqOn",      eqOnAtt);
    setupToggle(compOnBtn,    "compOn",    compOnAtt);
    setupToggle(satOnBtn,     "satOn",     satOnAtt);
    setupToggle(reverbOnBtn,  "reverbOn",  reverbOnAtt);
    setupToggle(delayOnBtn,   "delayOn",   delayOnAtt);
    setupToggle(busCompOnBtn, "busCompOn", busCompOnAtt);

    addAndMakeVisible(busCompAutoRelBtn);
    busCompAutoRelAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "busCompAutoRel", busCompAutoRelBtn);

    startTimerHz(30);
}

WavCultStripEditor::~WavCultStripEditor()
{
    setLookAndFeel(nullptr);
}

void WavCultStripEditor::setupKnob(LabeledKnob& knob, const juce::String& paramId,
                                    const juce::String& name, juce::Colour colour)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 14);
    knob.slider.setColour(juce::Slider::rotarySliderFillColourId, colour);
    knob.slider.setColour(juce::Slider::textBoxTextColourId, kTextColour);
    knob.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(knob.slider);

    knob.label.setText(name, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setColour(juce::Label::textColourId, colour.withAlpha(0.8f));
    knob.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), paramId, knob.slider);
}

void WavCultStripEditor::paint(juce::Graphics& g)
{
    g.fillAll(kBgColour);

    // Draw logo in top-left corner
    if (logoImage.isValid())
    {
        int logoSize = 32;
        float logoAspect = (float)logoImage.getWidth() / (float)logoImage.getHeight();
        int logoW = (int)(logoSize * logoAspect);
        g.setOpacity(0.9f);
        g.drawImage(logoImage, 10, 4, logoW, logoSize,
                     0, 0, logoImage.getWidth(), logoImage.getHeight());
        g.setOpacity(1.0f);
    }

    // Title
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("WAVCULT STRIP", 0, 4, getWidth(), 24, juce::Justification::centred);

    g.setFont(juce::FontOptions(9.0f));
    g.setColour(juce::Colour(0xFF666666));
    g.drawText("CHANNEL STRIP", 0, 22, getWidth(), 14, juce::Justification::centred);

    // Section backgrounds
    auto drawSection = [&](int x, int y, int w, int h, const juce::String& title, juce::Colour accent) {
        g.setColour(kSectionBg);
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 6.0f);
        g.setColour(kSectionBorder);
        g.drawRoundedRectangle((float)x, (float)y, (float)w, (float)h, 6.0f, 1.0f);

        // Accent line at top
        g.setColour(accent);
        g.fillRoundedRectangle((float)x + 2, (float)y + 1, (float)w - 4, 2.0f, 1.0f);

        // Title
        g.setColour(accent);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(title, x, y + 4, w, 16, juce::Justification::centred);
    };

    // Section positions (matching resized())
    int y0 = 42;
    int sectionH0 = 130;  // Gate + I/O row
    int y1 = y0 + sectionH0 + 6;
    int sectionH1 = 170;  // EQ section (taller - 2 rows)
    int y2 = y1 + sectionH1 + 6;
    int sectionH2 = 130;
    int y3 = y2 + sectionH2 + 6;

    // Row 0: Gate + I/O
    drawSection(8, y0, 430, sectionH0, "NOISE GATE", kGateColour);
    drawSection(446, y0, 506, sectionH0, "I/O", kTextColour);

    // Row 1: EQ
    drawSection(8, y1, 944, sectionH1, "SSL CHANNEL EQ", kEQColour);

    // Row 2: Compressor + Saturation + Reverb
    drawSection(8, y2, 500, sectionH2, "CLA COMPRESSOR", kCompColour);
    drawSection(516, y2, 180, sectionH2, "SATURATION", kSatColour);
    drawSection(704, y2, 248, sectionH2, "REVERB", kReverbColour);

    // Row 3: Delay + Bus Comp
    drawSection(8, y3, 430, sectionH2, "DELAY", kDelayColour);
    drawSection(446, y3, 506, sectionH2, "BUS COMPRESSOR", kBusCompColour);

    // GR meters
    auto drawGRMeter = [&](int mx, int my, int mw, int mh, float gr, const juce::String& label) {
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRect(mx, my, mw, mh);

        float grClamped = juce::jlimit(-30.0f, 0.0f, gr);
        float grNorm = -grClamped / 30.0f;
        int barH = (int)(grNorm * mh);

        g.setColour(juce::Colour(0xFFFF6B6B));
        g.fillRect(mx, my, mw, barH);

        g.setColour(kTextColour);
        g.setFont(9.0f);
        g.drawText(label, mx - 2, my + mh + 2, mw + 4, 12, juce::Justification::centred);

        juce::String grText = juce::String(gr, 1) + " dB";
        g.drawText(grText, mx - 10, my - 14, mw + 20, 12, juce::Justification::centred);
    };

    drawGRMeter(480, y2 + 30, 10, 80, compGR, "GR");
    drawGRMeter(924, y3 + 30, 10, 80, busGR, "GR");
}

void WavCultStripEditor::resized()
{
    int y0 = 42;
    int sectionH0 = 130;
    int y1 = y0 + sectionH0 + 6;
    int sectionH1 = 170;
    int y2 = y1 + sectionH1 + 6;
    int sectionH2 = 130;
    int y3 = y2 + sectionH2 + 6;

    int knobW = 60;
    int knobH = 60;
    int labelH = 14;

    auto placeKnob = [&](LabeledKnob& knob, int x, int y) {
        knob.slider.setBounds(x, y, knobW, knobH);
        knob.label.setBounds(x, y + knobH, knobW, labelH);
    };

    auto placeToggle = [](juce::ToggleButton& btn, int x, int y) {
        btn.setBounds(x, y, 40, 18);
    };

    // Noise Gate Section
    int gateX = 20;
    int gateY = y0 + 28;
    placeToggle(gateOnBtn, 12, y0 + 5);

    placeKnob(gateThresh,  gateX, gateY);
    placeKnob(gateAttack,  gateX + 70, gateY);
    placeKnob(gateRelease, gateX + 140, gateY);
    placeKnob(gateHold,    gateX + 210, gateY);
    placeKnob(gateRange,   gateX + 280, gateY);

    // I/O Section
    placeKnob(inputGain,  510, y0 + 28);
    placeKnob(outputGain, 580, y0 + 28);

    // EQ Section - Row 1: Low + Low-Mid
    int eqX = 22;
    int eqY1 = y1 + 22;
    int eqY2 = y1 + 98;

    placeToggle(eqOnBtn, 12, y1 + 5);

    placeKnob(eqLowGain, eqX + 40, eqY1);
    placeKnob(eqLowFreq, eqX + 110, eqY1);
    placeKnob(eqLMGain,  eqX + 215, eqY1);
    placeKnob(eqLMFreq,  eqX + 285, eqY1);
    placeKnob(eqLMQ,     eqX + 355, eqY1);

    // EQ Section - Row 2: Hi-Mid + High
    placeKnob(eqHMGain,  eqX + 40, eqY2);
    placeKnob(eqHMFreq,  eqX + 110, eqY2);
    placeKnob(eqHMQ,     eqX + 180, eqY2);
    placeKnob(eqHighGain,eqX + 285, eqY2);
    placeKnob(eqHighFreq,eqX + 355, eqY2);

    // Compressor
    int compX = 20;
    int compY = y2 + 28;
    placeToggle(compOnBtn, 12, y2 + 5);

    placeKnob(compThresh,  compX, compY);
    placeKnob(compRatio,   compX + 70, compY);
    placeKnob(compAttack,  compX + 140, compY);
    placeKnob(compRelease, compX + 210, compY);
    placeKnob(compMakeup,  compX + 280, compY);
    placeKnob(compMix,     compX + 350, compY);

    // Saturation
    int satX = 530;
    int satY = y2 + 28;
    placeToggle(satOnBtn, 520, y2 + 5);

    placeKnob(satDrive, satX, satY);
    placeKnob(satMix,   satX + 70, satY);

    // Reverb
    int revX = 718;
    int revY = y2 + 28;
    placeToggle(reverbOnBtn, 708, y2 + 5);
    reverbModeBox.setBounds(760, y2 + 5, 80, 18);

    placeKnob(reverbAmt,      revX, revY);
    placeKnob(reverbDecay,    revX + 60, revY);
    placeKnob(reverbPreDelay, revX + 120, revY);
    placeKnob(reverbDamp,     revX + 180, revY);

    // Delay
    int dlX = 20;
    int dlY = y3 + 28;
    placeToggle(delayOnBtn, 12, y3 + 5);
    delayModeBox.setBounds(60, y3 + 5, 90, 18);

    placeKnob(delayAmt,      dlX, dlY);
    placeKnob(delayTime,     dlX + 70, dlY);
    placeKnob(delayFeedback, dlX + 140, dlY);
    placeKnob(delayHighCut,  dlX + 210, dlY);
    placeKnob(delayLowCut,   dlX + 280, dlY);

    // Bus Compressor
    int bcX = 460;
    int bcY = y3 + 28;
    placeToggle(busCompOnBtn, 450, y3 + 5);
    busCompAutoRelBtn.setBounds(500, y3 + 5, 50, 18);

    placeKnob(busCompThresh,  bcX, bcY);
    placeKnob(busCompRatio,   bcX + 60, bcY);
    placeKnob(busCompAttack,  bcX + 120, bcY);
    placeKnob(busCompRelease, bcX + 180, bcY);
    placeKnob(busCompMakeup,  bcX + 240, bcY);
    placeKnob(busCompMix,     bcX + 300, bcY);
    placeKnob(busCompSCHPF,   bcX + 360, bcY);
}

void WavCultStripEditor::timerCallback()
{
    compGR = processor.getCompressorGR();
    busGR  = processor.getBusCompGR();
    repaint();
}

juce::AudioProcessorEditor* WavCultStripProcessor::createEditor()
{
    return new WavCultStripEditor(*this);
}

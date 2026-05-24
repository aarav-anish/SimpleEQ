/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics &g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider &slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(37u, 0u, 1u));
    g.fillEllipse(bounds);

    g.setColour(Colour(252u, 186u, 4u));
    g.drawEllipse(bounds, 1.f);

    if (auto *rswl = dynamic_cast<RotarySliderWithLabels *>(&slider))
    {
        auto center = bounds.getCentre();

        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - 2.f);
        r.setRight(center.getX() + 2.f);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5f);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngleRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(AffineTransform::rotation(sliderAngleRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto textWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(textWidth + 4.f, rswl->getTextHeight() + 2.f);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;

    if (auto *pb = dynamic_cast<PowerButton *>(&toggleButton))
    {
        Path powerButton;

        auto bounds = toggleButton.getLocalBounds();
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6.f;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

        float ang = 30.f;
        size -= 6.f;

        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5f,
                                  size * 0.5f,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);

        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());

        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2.f);
    }
    else if (auto *analyzerButton = dynamic_cast<AnalyzerButton *>(&toggleButton))
    {
        auto color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
        g.setColour(color);

        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);

        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
}

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;

    auto startAngle = degreesToRadians(180.f + 45.f);
    auto endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAngle,
                                      endAngle,
                                      *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() / 2.f;

    g.setColour(Colour(243u, 243u, 243u));
    g.setFont(getTextHeight());

    auto numLabels = labels.size();
    for (int i = 0; i < numLabels; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos && pos <= 1.f);

        auto str = labels[i].label;
        auto angle = jmap(pos, 0.f, 1.f, startAngle, endAngle);
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, angle);

        Rectangle<float> r;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    auto size = juce::jmin(getWidth(), getHeight());
    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto choicesParam = dynamic_cast<juce::AudioParameterChoice *>(param))
    {
        return choicesParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;

    if (auto *floatParam = dynamic_cast<juce::AudioParameterFloat *>(param))
    {
        float val = floatParam->get();
        if (val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }
        str = juce::String(val, addK ? 2 : 1);
    }
    else
    {
        jassertfalse; // this shouldn't happen, but you could add support for other parameter types if you need to.
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
        {
            str << "k";
        }
        str << suffix;
    }

    return str;
}

//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor &p)
    : audioProcessor(p),
      leftPathProducer(audioProcessor.leftChannelFifo),
      rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto &params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    updateChain();
    startTimerHz(45);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto &params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged = true;
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;

    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            // we have a buffer, but we don't actually need to do anything with it in this component, so we'll just discard it.
            auto size = tempIncomingBuffer.getNumSamples();

            juce::Logger::writeToLog("Received audio buffer with " + juce::String(size) + " samples");

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    /*
    if there are fft data buffers available
    if we can pull from the fifo
    generate fft data for rendering
    */
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

    /*
    48000 / 2048 = 23.4375 hz <- this is the bin width
    */
    const auto binWidth = sampleRate / double(fftSize);

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    /*
    while there are paths available from the path producer
    pull them and add them to a local path
    which the paint method can use to draw the FFT analysis curve.
    */
    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}

void ResponseCurveComponent::timerCallback()
{
    auto fftBounds = getAnalysisArea().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();

    leftPathProducer.process(fftBounds, sampleRate);
    rightPathProducer.process(fftBounds, sampleRate);

    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("Parameters changed, updating response curve");
        // update the monoChain with the new parameters from the processor
        updateChain();

        // signal the editor to repaint itself
        // repaint();
    }
    repaint();
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);

    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);

    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);

    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();

    auto w = responseArea.getWidth();

    auto &lowCut = monoChain.get<ChainPositions::LowCut>();
    auto &peak = monoChain.get<ChainPositions::Peak>();
    auto &highCut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = juce::mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (!lowCut.isBypassed<0>())
                mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<1>())
                mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<2>())
                mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<3>())
                mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highCut.isBypassed<0>())
                mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<1>())
                mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<2>())
                mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<3>())
                mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

    juce::Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input)
    { return juce::jmap(input, -24.0, 24.0, outputMin, outputMax); };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags[0]));

    for (size_t i = 1; i < mags.size(); i++)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(juce::AffineTransform().translation(responseArea.getX(), responseArea.getY()));

    g.setColour(juce::Colours::skyblue);
    g.strokePath(leftChannelFFTPath, juce::PathStrokeType(1.f));

    auto rightChannelFFTPath = rightPathProducer.getPath();
    rightChannelFFTPath.applyTransform(juce::AffineTransform().translation(responseArea.getX(), responseArea.getY()));

    g.setColour(juce::Colours::lightpink);
    g.strokePath(rightChannelFFTPath, juce::PathStrokeType(1.f));

    g.setColour(juce::Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    g.setColour(juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);

    Array<float> freqs{
        20, 50, 100,
        200, 500, 1000,
        2000, 5000, 10000,
        20000};

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    auto height = renderArea.getHeight();

    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::dimgrey);

    for (auto x : xs)
    {
        g.drawVerticalLine(int(x), float(top), float(bottom));
    }

    Array<float> gainLines{-24, -12, 0, 12, 24};

    for (auto gain : gainLines)
    {
        auto y = jmap(gain, -24.f, 24.f, float(bottom), float(top));

        g.setColour(gain == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(int(y), float(left), float(right));
    }

    g.setColour(Colours::lightgrey);
    const int fontHeight = 10.f;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];
        String str;
        if (f >= 1000.f)
            str << (f / 1000.f) << "k";
        else
            str << f;
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<float> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);
    }

    for (auto gain : gainLines)
    {
        auto y = jmap(gain, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gain > 0)
            str << "+";
        str << gain;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<float> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gain == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);
        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);

        str.clear();
        str << (gain - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);

        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);
    }
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),

      peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), " Hz"),
      peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), " dB"),
      peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
      lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), " Hz"),
      highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), " Hz"),
      lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), " dB/Oct"),
      highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), " dB/Oct"),

      responseCurveComponent(audioProcessor),
      peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
      peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
      peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
      lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
      highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
      lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
      highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),

      lowCutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowCutBypassButton),
      highCutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highCutBypassButton),
      peakBypassButtonAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
      analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});

    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});

    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10"});

    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});

    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});

    lowCutSlopeSlider.labels.add({0.f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});

    highCutSlopeSlider.labels.add({0.f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});

    for (auto *comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);
    peakBypassButton.setLookAndFeel(&lnf);
    analyzerEnabledButton.setLookAndFeel(&lnf);

    setSize(600, 480);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    peakBypassButton.setLookAndFeel(nullptr);
    analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    auto analyzerEnabledArea = bounds.removeFromTop(25);
    analyzerEnabledArea.setWidth(100);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);

    analyzerEnabledButton.setBounds(analyzerEnabledArea);

    bounds.removeFromTop(5);

    float hRatio = 25.f / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(5);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component *> SimpleEQAudioProcessorEditor::getComps()
{
    return {&peakFreqSlider,
            &peakGainSlider,
            &peakQualitySlider,
            &lowCutFreqSlider,
            &highCutFreqSlider,
            &lowCutSlopeSlider,
            &highCutSlopeSlider,
            &responseCurveComponent,

            &lowCutBypassButton,
            &highCutBypassButton,
            &peakBypassButton,
            &analyzerEnabledButton};
}
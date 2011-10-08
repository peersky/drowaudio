/*
 *  FilteringAudioTransportSource.h
 *
 *  Created by David Rowland on 24/03/2009.
 *  Copyright 2009 dRowAudio. All rights reserved.
 *
 */

#ifndef __DROWAUDIO_FILTERINGAUDIOSOURCE_H__
#define __DROWAUDIO_FILTERINGAUDIOSOURCE_H__

/**	An AudioSource that contains three settable filters to EQ the audio stream.
	This also creates an internal resampler source that is adjustable.
	
 */
class FilteringAudioSource : public AudioSource
{

public:
	//==============================================================================
    /** Creates an FilteringAudioTransportSource.
	 
	 After creating one of these, use the setSource() method to select an input source.
	 */
    FilteringAudioSource (AudioSource* inputSource,
                          bool deleteInputWhenDeleted,
                          int numChannels = 2);
	
    /** Destructor. */
    ~FilteringAudioSource();
    
    //==============================================================================
	/** Changes the gain of the lowShelfFilter to apply to the output.
	 */
	void setLowEQGain (float newLowEQGain);
	
	/** Changes the gain of the bandPassFilter to apply to the output.
	 */
	void setMidEQGain (float newMidEQGain);
	
	/** Changes the gain of the highShelfFilter to apply to the output.
	 */
	void setHighEQGain (float newHighEQGain);
	
	/** Toggles the filtering of the transport source.
	 */
	void setFilterSource (bool shouldFilter);

	/** Returns whether the source is being filtered or not.
	 */
	bool getFilterSource()				{ return filterSource; }

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
	
    /** Implementation of the AudioSource method. */
    void releaseResources();
	
    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);
		
private:
    //==============================================================================
    enum FilterType {
        low = 0,
        mid,
        high
    };

    OptionalScopedPointer<AudioSource> input;
	IIRFilter lowEQFilterL, midEQFilterL, highEQFilterL;
	IIRFilter lowEQFilterR, midEQFilterR, highEQFilterR;
	
    SpinLock callbackLock;
    double sampleRate;
	float lowEQGain, midEQGain, highEQGain;
	bool filterSource;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilteringAudioSource);
};

#endif //__DROWAUDIO_FILTERINGAUDIOSOURCE_H__
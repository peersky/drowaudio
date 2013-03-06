/*
  ==============================================================================
  
  This file is part of the dRowAudio JUCE module
  Copyright 2004-12 by dRowAudio.
  
  ------------------------------------------------------------------------------
 
  dRowAudio can be redistributed and/or modified under the terms of the GNU General
  Public License (Version 2), as published by the Free Software Foundation.
  A copy of the license is included in the module distribution, or can be found
  online at www.gnu.org/licenses.
  
  dRowAudio is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
  
  ==============================================================================
*/


AudioFilePlayer::AudioFilePlayer()
    : bufferingTimeSliceThread (new TimeSliceThread ("Shared Buffering Thread"), true),
      formatManager (new AudioFormatManager(), true)
{
    bufferingTimeSliceThread->startThread (3);
	formatManager->registerBasicFormats();
    
    commonInitialise();
}

AudioFilePlayer::AudioFilePlayer (TimeSliceThread* threadToUse,
                                  AudioFormatManager* formatManagerToUse)
    : bufferingTimeSliceThread ((threadToUse == nullptr ? new TimeSliceThread ("Shared Buffering Thread")
                                                        : threadToUse),
                                threadToUse == nullptr),
      formatManager ((formatManagerToUse == nullptr ? new AudioFormatManager()
                                                    : formatManagerToUse),
                     formatManagerToUse == nullptr)
{
    commonInitialise();
}

AudioFilePlayer::~AudioFilePlayer()
{
	audioTransportSource->setSource (nullptr);
    audioTransportSource->removeChangeListener (this);
}

//==============================================================================
bool AudioFilePlayer::setInputStream (InputStream* inputStream)
{
    inputType = unknownStream;
    
    return setSourceWithReader (formatManager->createReaderFor (inputStream));
}

InputStream* AudioFilePlayer::getInputStream()
{
    switch (inputType)
    {
        case file:
            return new FileInputStream (currentFile);

        case memoryBlock:
        case memoryInputStream:
        {
            MemoryInputStream* memoryStream = dynamic_cast<MemoryInputStream*> (inputStream);
            
            if (memoryStream != nullptr)
                return new MemoryInputStream (memoryStream->getData(), memoryStream->getDataSize(), false);
            else
                return nullptr;
        }

        case unknownStream:
            return inputStream;

        default:
            return nullptr;
    }
}

//==============================================================================
bool AudioFilePlayer::setFile (const File& newFile)
{
    inputType = file;
    inputStream = nullptr;
    currentFile = newFile;
    
    return setSourceWithReader (formatManager->createReaderFor (currentFile));
}

bool AudioFilePlayer::setMemoryInputStream (MemoryInputStream* newMemoryInputStream)
{
    inputType = memoryInputStream;
    currentFile = File::nonexistent;
    inputStream = newMemoryInputStream;
    
    return setSourceWithReader (formatManager->createReaderFor (inputStream));
}

bool AudioFilePlayer::setMemoryBlock (MemoryBlock& inputBlock)
{
    inputType = memoryBlock;
    currentFile = File::nonexistent;
    inputStream = new MemoryInputStream (inputBlock, false);

    return setSourceWithReader (formatManager->createReaderFor (inputStream));
}

//==============================================================================
void AudioFilePlayer::start()
{
    audioTransportSource->start();
    
    listeners.call (&Listener::playerStoppedOrStarted, this);
}

void AudioFilePlayer::stop()
{
    audioTransportSource->stop();
    
    listeners.call (&Listener::playerStoppedOrStarted, this);
}

void AudioFilePlayer::startFromZero()
{
	if (audioFormatReaderSource == nullptr)
        return;
	
	audioTransportSource->setPosition (0.0);
	audioTransportSource->start();
    
    listeners.call (&Listener::playerStoppedOrStarted, this);
}

void AudioFilePlayer::pause()
{
	if (audioTransportSource->isPlaying())
		audioTransportSource->stop();
	else
		audioTransportSource->start();
    
    listeners.call (&Listener::playerStoppedOrStarted, this);
}

//==============================================================================
void AudioFilePlayer::setPosition (double newPosition, bool /*ignoreAnyLoopPoints*/)
{
    audioTransportSource->setPosition (newPosition);
}

//==============================================================================
void AudioFilePlayer::setAudioFormatManager (AudioFormatManager* newManager, bool deleteWhenNotNeeded)
{
    formatManager.set (newManager, deleteWhenNotNeeded);
}

void AudioFilePlayer::setTimeSliceThread (TimeSliceThread* newThreadToUse, bool deleteWhenNotNeeded)
{
    bufferingTimeSliceThread.set (newThreadToUse, deleteWhenNotNeeded);
}

//==============================================================================
void AudioFilePlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    if (masterSource != nullptr)
        masterSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioFilePlayer::releaseResources()
{
    if (masterSource != nullptr)
        masterSource->releaseResources();
}

void AudioFilePlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (masterSource != nullptr)
        masterSource->getNextAudioBlock (bufferToFill);
}

void AudioFilePlayer::setLooping (bool shouldLoop)
{   
    if (audioFormatReaderSource != nullptr)
        audioFormatReaderSource->setLooping (shouldLoop); 
}

void AudioFilePlayer::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == audioTransportSource)
        listeners.call (&Listener::playerStoppedOrStarted, this);
}

//==============================================================================
void AudioFilePlayer::addListener (AudioFilePlayer::Listener* const listener)
{
    listeners.add (listener);
}

void AudioFilePlayer::removeListener (AudioFilePlayer::Listener* const listener)
{
    listeners.remove (listener);
}

//==============================================================================
bool AudioFilePlayer::setSourceWithReader (AudioFormatReader* reader)
{
    bool shouldBeLooping = isLooping();
	audioTransportSource->setSource (nullptr);

	if (reader != nullptr)
	{										
		// we SHOULD let the AudioFormatReaderSource delete the reader for us..
		audioFormatReaderSource = new AudioFormatReaderSource (reader, true);
        audioTransportSource->setSource (audioFormatReaderSource,
                                         32768,
                                         bufferingTimeSliceThread);
        
        if (shouldBeLooping)
            audioFormatReaderSource->setLooping (true);
        
		// let our listeners know that we have loaded a new file
		audioTransportSource->sendChangeMessage();
        listeners.call (&Listener::fileChanged, this);

		return true;
	}
	
    audioTransportSource->sendChangeMessage();
    listeners.call (&Listener::fileChanged, this);

    return false;    
}

//==============================================================================
void AudioFilePlayer::commonInitialise()
{
    inputType = noInput;
    currentFile = File::nonexistent;
    inputStream = nullptr;
    
    audioTransportSource = new AudioTransportSource();
    audioTransportSource->addChangeListener (this);
    masterSource = audioTransportSource;
}

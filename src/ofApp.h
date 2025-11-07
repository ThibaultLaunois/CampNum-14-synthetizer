#pragma once

#include "ofMain.h"

#include "ofxFft.h"  // Declare the FFT object and data


struct PianoKey {
	ofRectangle rect;
	float frequency;
	bool isBlack;
	char keyChar;
	bool isPressed;
	string noteName;
};


class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
//		void mouseMoved(int x, int y );
//		void mouseDragged(int x, int y, int button);
//		void mousePressed(int x, int y, int button);
//		void mouseReleased(int x, int y, int button);
//		void mouseEntered(int x, int y);
//		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void audioOut(ofSoundBuffer & buffer);
		
		void setupPianoKeys();
		float calculateFrequency(int semitonesFromA4);
		void drawPiano();
		void drawWaveform();
		void drawFrequencySpectrum();
		
		ofSoundStream soundStream;

		// float pan;
		int		sampleRate;
		//		bool 	bNoise;
		float 	volume;

		vector <float> lAudio;
		vector <float> rAudio;

		//------------------- for the simple sine wave synthesis
		// float 	targetFrequency;
		// float 	phase;
		// float 	phaseAdder;
		// float 	phaseAdderTarget;

		// Piano keys
		vector<PianoKey> pianoKeys;
		map<char, int> keyToIndex;
		
		// Synthesis
		map<int, float> activePhases;
		map<int, float> activePhaseAdders;
		ofMutex audioMutex;
		
		// Visual
		int whiteKeyWidth;
		int whiteKeyHeight;
		int blackKeyWidth;
		int blackKeyHeight;
		int pianoStartY;


		// For FFT analysis
		
		ofxFft* fft;

		std::vector<float> fftMagnitude;

		std::vector<float> fftBins;

};

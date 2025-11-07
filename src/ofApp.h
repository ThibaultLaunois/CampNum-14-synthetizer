#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
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

	ofSoundStream soundStream;

	// float pan;
	int sampleRate;
	//		bool 	bNoise;
	float volume;

	vector<float> lAudio;
	vector<float> rAudio;

	//------------------- for the simple sine wave synthesis
	// float 	targetFrequency;
	// float 	phase;
	// float 	phaseAdder;
	// float 	phaseAdderTarget;

	//------------------- Piano key structure
	struct PianoKey {
		ofRectangle rect; // Position and size of the key on screen (for drawing and mouse interaction)
		bool isBlack; // Is it a black key? (affects drawing color and layering)
		string noteName; // e.g., "C4", "C#4", "D4", etc. (for display purposes)
		float frequency; // Frequency of the note produced by the key in Hz
		char keyChar; // Keyboard character that triggers this key
		bool isPressed; // Is the key currently pressed?
	};

	vector<PianoKey> pianoKeys; // All piano keys
	map<char, int> keyToIndex; // Map keyboard characters to `pianoKeys` indices for quick lookup

	//-------------------- Polyphonic synthesis
	// A voice allows playing a piano key
	struct Voice {
		bool active; // Is this voice currently playing?
		float phase; // Current position in the wave
		float phaseAdder; // Speed of advancement in the wave (calculated based on the frequency of the piano key and the sample rate of 44100)
		int keyIndex; // Which piano key activated this voice? (index in the pianoKeys array. For example, the key 'a' = 0)
	};

	static const int MAX_VOICES = 10; // Maximum number of simultaneous voices allowed
	Voice voices[MAX_VOICES]; // Array of voices

	// Visual
	int whiteKeyWidth;
	int whiteKeyHeight;
	int blackKeyWidth;
	int blackKeyHeight;
	int pianoStartY;
};

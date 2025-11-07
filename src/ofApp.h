#pragma once

#include "ofMain.h"
#include "ofxGui.h"  // <--- ajouté par Marie

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

		// Active le clic souris sur les touches (UI only) // <--- modifié par Marie
		void mouseMoved(int x, int y );
//		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
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

		// ---------- UI (nouveau) ---------- ajouté par Marie
		ofxPanel gui;
		ofParameter<float> pVolume;
		ofParameter<bool>  pSustain;
		ofParameter<int>   pOctave;
		ofxButton          startBtn;
		ofxButton          stopBtn;

		// --- UI clavier ---
		int  hoveredKey = -1;     // index de la touche survolée
		bool mouseIsDown = false; // souris enfoncée sur une touche

		// --- Synth ---
		int  octaveOffset = 0;    // miroir de pOctave pour accès rapide
		std::vector<float> baseFreqs;
		std::vector<float> currFreqs;
		// utilitaire pour reconstruire fréquences quand octave change
		void rebuildFrequencies(int baseOctaveOffset);

		// Callbacks UI
		void onVolumeChanged(float& v);
		void onOctaveChanged(int& off);
		void onStartPressed();
		void onStopPressed();

		// fin ajout Marie


};

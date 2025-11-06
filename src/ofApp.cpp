#include "ofApp.h"
#include <cmath>
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	
	int bufferSize		= 512;
	sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.4f;
	bNoise 				= false;

	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	
	pan = 0.5f; // ajout par marie => centre (sinon risque d'envoyer tout à droite/gauche)
	ofSoundStreamSettings settings;

	soundStream.printDeviceList();

	// if you want to set the device id to be different than the default:
	//
	//	auto devices = soundStream.getDeviceList();
	//	settings.setOutDevice(devices[3]);

	// you can also get devices for an specific api:
	//
	//	auto devices = soundStream.getDeviceList(ofSoundDevice::Api::PULSE);
	//	settings.setOutDevice(devices[0]);

	// or get the default device for an specific api:
	//
	// settings.api = ofSoundDevice::Api::PULSE;

	// or by name:
	//
	//	auto devices = soundStream.getMatchingDevices("default");
	//	if(!devices.empty()){
	//		settings.setOutDevice(devices[0]);
	//	}


	// Latest linux versions default to the HDMI output
	// this usually fixes that. Also check the list of available
	// devices if sound doesn't work

	//settings.setApi(ofSoundDevice::MS_ASIO);
	settings.setApi(ofSoundDevice::MS_WASAPI);
	//settings.setApi(ofSoundDevice::MS_DS);


	settings.setOutListener(this);
	settings.sampleRate = sampleRate;
	settings.numOutputChannels = 2;
	settings.numInputChannels = 0;
	settings.bufferSize = bufferSize;


	soundStream.setup(settings);
	soundStream.start();

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.
}


//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(225);
	ofDrawBitmapString("AUDIO OUTPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\npress 'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 150, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < lAudio.size(); i++){
				float x =  ofMap(i, 0, lAudio.size(), 0, 900, true);
				ofVertex(x, 100 -lAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the right channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 350, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < rAudio.size(); i++){
				float x =  ofMap(i, 0, rAudio.size(), 0, 900, true);
				ofVertex(x, 100 -rAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
		
	ofSetColor(225);
	string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\npan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	if( !bNoise ){
		reportString += "sine wave (" + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}else{
		reportString += "noise";	
	}
	ofDrawBitmapString(reportString, 32, 579);

}

// initialisation des notes et des touches 

// helpers pour C4…B4 à partir de A4=440 (ou 400 si tu veux)
static inline double C4_from_A4(double A4) { return A4 * std::pow(2.0, -9.0 / 12.0); }
static inline double noteFromC4(int semi, double A4 = 440.0) {
	return C4_from_A4(A4) * std::pow(2.0, semi / 12.0);
}


// map touche -> index [0..11] (A,W,D,F,T,G,Y,H,U,J,R,I)
// Les 12 touches : A W D F T G Y H U J R I → C4, C#4, D4, D#4, E4, F4, F#4, G4, G#4, A4, A#4, B4.
static inline int keyToIndex12(int key) {
	switch (key) {
	case 'a': case 'A': case 'q' : case 'Q': return 0;   // C4
	case 'w': case 'W': case 'z' : case 'Z': return 1;   // C#4
	case 's': case 'S': return 2;   // D4
	case 'e': case 'E': return 3;   // D#4
	case 'd': case 'D': return 4;   // E4
	case 'f': case 'F': return 5;   // F4
	case 't': case 'T': return 6;   // F#4
	case 'g': case 'G': return 7;   // G4
	case 'y': case 'Y': return 8;   // G#4
	case 'h': case 'H': return 9;   // A4
	case 'u': case 'U': return 10;  // A#4
	case 'j': case 'J': return 11;  // B4
	default: return -1;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	// --- volume ---
	if (key == '-' || key == '_' ){
		volume -= 0.05;
		volume = std::max(volume, 0.f);
	} else if (key == '+' || key == '=' ){
		volume += 0.05;
		volume = std::min(volume, 1.f);
	}

	// start/stop
	if (key == 'o' || key == 'O') soundStream.start();
	if (key == 'p' || key == 'P') soundStream.stop();

	// notes (polyphonie)
	int idx = keyToIndex12(key);
	if (idx >= 0) {
		noteOn[idx] = true;

		// A4 = 440 Hz (mets 400.0 si tu veux partir de 400 Hz)
		double f = noteFromC4(idx, 440.0);
		freqV[idx] = static_cast<float>(f);
		phaseAdderTargetV[idx] = (freqV[idx] / (float)sampleRate) * TWO_PI;
		// pas besoin de toucher aux autres voix
	}
}


//--------------------------------------------------------------
void ofApp::keyReleased  (int key){
	int idx = keyToIndex12(key);
	if (idx >= 0) {
		noteOn[idx] = false;
		// on peut laisser la phase/adder courir, le mix mettra 0 s’il n’y a pas de notes
		phaseAdderTargetV[idx] = 0.0f; // pour revenir gentiment à 0
	}
}

//--------------------------------------------------------------
//void ofApp::mouseMoved(int x, int y ){
//	int width = ofGetWidth();
//	pan = (float)x / (float)width;
//	float height = (float)ofGetHeight();
//	float heightPct = ((height-y) / height);
//	targetFrequency = 2000.0f * heightPct;
//	phaseAdderTarget = (targetFrequency / (float) sampleRate) * glm::two_pi<float>();
//}

//--------------------------------------------------------------
//void ofApp::mouseDragged(int x, int y, int button){
//	int width = ofGetWidth();
//	pan = (float)x / (float)width;
//}

//--------------------------------------------------------------
//void ofApp::mousePressed(int x, int y, int button){
//	bNoise = true;
//}


//--------------------------------------------------------------
//void ofApp::mouseReleased(int x, int y, int button){
//	bNoise = false;
//}

//--------------------------------------------------------------
//void ofApp::mouseEntered(int x, int y){
//
//}

//--------------------------------------------------------------
//void ofApp::mouseExited(int x, int y){
//
//}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer){
	//pan = 0.5f;
	pan = ofClamp(pan, 0.0f, 1.0f);
	float leftScale = 1.0f - pan;
	float rightScale = pan;

	// assure la taille des vecteurs (au cas où)
	if (lAudio.size() < buffer.getNumFrames()) lAudio.resize(buffer.getNumFrames());
	if (rAudio.size() < buffer.getNumFrames()) rAudio.resize(buffer.getNumFrames());


	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-glm::two_pi<float>() like this:
	while (phase > glm::two_pi<float>()){
		phase -= glm::two_pi<float>();
	}

	// changement du code pour polyphonie 

	if (bNoise) {
		for (size_t i = 0; i < buffer.getNumFrames(); i++) {
			lAudio[i] = buffer[i * buffer.getNumChannels()] = ofRandom(0, 1) * volume * leftScale;
			rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = ofRandom(0, 1) * volume * rightScale;
		}
		return;
	}

	// --- polyphonie : somme des 12 voix actives ---
	const float smooth = 0.95f; // même easing que ton mono
	for (size_t i = 0; i < buffer.getNumFrames(); ++i) {
		float sum = 0.0f;
		int active = 0;

		for (int v = 0; v < 12; ++v) {
			// interp vers la cible
			phaseAdderV[v] = smooth * phaseAdderV[v] + (1.0f - smooth) * phaseAdderTargetV[v];

			if (noteOn[v] && phaseAdderV[v] > 0.0f) {
				phaseV[v] += phaseAdderV[v];
				if (phaseV[v] > TWO_PI) phaseV[v] -= TWO_PI;
				sum += std::sin(phaseV[v]);
				++active;
			}
		}

		float sample = 0.0f;
		if (active > 0) {
			// normalisation simple : moyenne
			sample = (sum / (float)active) * volume;
		}

		// sortie stéréo
		float L = sample * leftScale;
		float R = sample * rightScale;
		lAudio[i] = buffer[i * buffer.getNumChannels()] = L;
		rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = R;
	}

	//if ( bNoise == true){
	//	// ---------------------- noise --------------
	//	for (size_t i = 0; i < buffer.getNumFrames(); i++){
	//		lAudio[i] = buffer[i*buffer.getNumChannels()    ] = ofRandom(0, 1) * volume * leftScale;
	//		rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = ofRandom(0, 1) * volume * rightScale;
	//	}
	//} else {
	//	phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
	//	for (size_t i = 0; i < buffer.getNumFrames(); i++){
	//		phase += phaseAdder;
	//		float sample = sin(phase);
	//		lAudio[i] = buffer[i*buffer.getNumChannels()    ] = sample * volume * leftScale;
	//		rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = sample * volume * rightScale;
	//	}
	//}

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

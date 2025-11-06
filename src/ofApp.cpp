#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(34, 34, 34);
	
	int bufferSize		= 512;
	
	sampleRate 			= 44100;
	
	phase 				= 0;
	
	phaseAdder 			= 0.0f;
	
	phaseAdderTarget 	= 0.0f;
	
	volume				= 0.1f;
	
	bNoise 				= false;

	lAudio.assign(bufferSize, 0.0);
	
	rAudio.assign(bufferSize, 0.0);
	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

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
	//settings.setApi(ofSoundDevice::MS_WASAPI);
	//settings.setApi(ofSoundDevice::MS_DS);

	auto devices = soundStream.getMatchingDevices("default");
	
	if(!devices.empty()){
		
		settings.setOutDevice(devices[0]);
	
	}




	settings.setOutListener(this);
	
	settings.sampleRate = sampleRate;
	
	settings.numOutputChannels = 2;
	
	settings.numInputChannels = 0;
	
	settings.bufferSize = bufferSize;

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.

	soundStream.setup(settings);

	// --- Synth key setup (SAM) ---

	baseFrequency = 440.0f; // A4 reference

	// Map keyboard keys to semitone offsets (from A4)
	
	keyMap = {
    	    
	    {'a', -9}, {'q', -9},                  // C4 - 'A' on QWERTY , 'Q' on AZERTY
    	    
	    {'w', -8}, {'z', -8},                 // C#4 - 'Z' on QWERTY , 'W' on AZERTY
    	    
	    {'s', -7},                           // D4
    	    
	    {'e', -6},                          // D#4
    	    
	    {'d', -5},                         // E4
    	    
	    {'f', -4}, 			      // F4
    	    
	    {'t', -3},                       // F#4
    	    
	    {'g', -2},                      // G4
    	    
	    {'y', -1},                     //G#4
    	    
	    {'h', 0}, 		          // A4
    	    
	    {'u', 1},                    // A#4
    	    
	    {'j', 2}                    // B4

	};

	

	// Set a default frequency
	
	targetFrequency = baseFrequency;
	
	phaseAdderTarget = (targetFrequency / (float)sampleRate) * glm::two_pi<float>();

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


//---------------------- (SAM edit) --------------------------------
void ofApp::keyPressed(int key){

	key = std::tolower(key); // normalize to lowercase
	
	// --- Volume control ---
	
	if (key == '-' || key == '_' ){
		
		volume -= 0.05;

		volume = std::max(volume, 0.f);

		return;

	} else if (key == '+' || key == '=' ){
		
		volume += 0.05;
		
		volume = std::min(volume, 1.f);

		return;
	}

	// --- Stream control ---

	if( key == 'o' ){soundStream.start(); return; }
		
	if( key == 'p' ){soundStream.stop(); return; }


	// --- Synthesize key control (SAM edit) ---
	
	if (keyMap.find(key) != keyMap.end()) {
	    
	    // Add key to activate list (only once)
	    activeKeys.insert(key);

	    // Rebuild list of active frequencies
	    
	    activeFreqs.clear();
	    
	    for (auto k : activeKeys) { 

	    	int i = keyMap[key];
	    
	    	float freq = baseFrequency * pow(2.0, i / 12.0);
	    
	    	activeFreqs.push_back(freq);
	    }
	    
	    bNoise = false; // This ensures that a note is played and not noise

	}
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

	key = std::tolower(key);

	if (activeKeys.find(key) != activeKeys.end()) {
	    activeKeys.erase(key);

	    // Rebuild list of active frequencies

	    activeFreqs.clear();

	    for (auto k : activeKeys) {

		int i = keyMap[k];

		float freq = baseFrequency * pow(2.0, i / 12.0);
		
		activeFreqs.push_back(freq);
	    }
	

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
	pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-glm::two_pi<float>() like this:
	while (phase > glm::two_pi<float>()){
		phase -= glm::two_pi<float>();
	}

	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
	    
		float sample = 0.0f;

		// If any keys are active, mix their sine waves
		if (!activeFreqs.empty()) {
		    
		    for (auto freq : activeFreqs) {
		        
			float phaseStep = glm::two_pi<float>() * freq / sampleRate;

			sample += sin(phase + i * phaseStep);
		    } 

		    // average to prevent clipping
		    sample /= activeFreqs.size();
		}

		// send to left/tight channels

		lAudio[i] = buffer[i * buffer.getNumChannels()]   = sample * volume * leftScale;
		rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = sample * volume * rightScale;

	}

	// increase phase for continuity
	
	phase += glm::two_pi<float>() * 440.0 / sampleRate;
	
	}

	//if ( bNoise == true){
		// ---------------------- noise --------------
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


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	ofSetWindowTitle("Mini Piano Synth");
	ofSetWindowShape(1400, 700);

	int bufferSize		= 512;
	sampleRate 			= 44100;
	//	phase 				= 0;
	//	phaseAdder 			= 0.0f;
	//	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	//	bNoise 				= false;

	//	lAudio.assign(bufferSize, 0.0);
	//	rAudio.assign(bufferSize, 0.0);

	// Piano dimensions
	whiteKeyWidth = 40;
	whiteKeyHeight = 200;
	blackKeyWidth = 24;
	blackKeyHeight = 120;
	pianoStartY = 400;
	
	setupPianoKeys();
	
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
	soundStream.setup(settings);

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.

        //------------------ SAM edit --------------------
	
	fft = ofxFft::create(512); //same as buffersize
	
	fftMagnitude.assign(fft->getBinSize(), 0.0);

	fftBins.assign(fft->getBinSize(), 0.0);
}


//--------------------------------------------------------------
void ofApp::setupPianoKeys(){
	// Notes dans une octave: C, C#, D, D#, E, F, F#, G, G#, A, A#, B
	// Pattern des touches noires: après C, D, F, G, A (pas après E et B)
	vector<string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	vector<bool> isBlackKey = {false, true, false, true, false, false, true, false, true, false, true, false};
	
	// Clavier QWERTY pour 3 octaves (36 notes)
	string keyboardKeys = "awsedftgyhujkolp;'[]\\zsxcfvgbnjmk,.";
	
	int whiteKeyIndex = 0;
	int startX = 50;
	
	// 3 octaves (octave 3, 4, et 5)
	for(int octave = 3; octave <= 5; octave++){
		for(int note = 0; note < 12; note++){
			// C de l'octave 4 est 9 demi-tons en dessous de A4 (440Hz)
			// C3 est 21 demi-tons en dessous, C4 est 9 demi-tons en dessous
			int semitonesFromA4 = (octave - 4) * 12 + (note - 9);
			
			PianoKey key;
			key.frequency = calculateFrequency(semitonesFromA4);
			key.isBlack = isBlackKey[note];
			key.isPressed = false;
			key.noteName = noteNames[note] + ofToString(octave);
			
			int keyIndex = (octave - 3) * 12 + note;
			if(keyIndex < keyboardKeys.length()){
				key.keyChar = keyboardKeys[keyIndex];
				keyToIndex[key.keyChar] = pianoKeys.size();
			}
			
			if(!key.isBlack){
				// Touche blanche
				key.rect.x = startX + whiteKeyIndex * whiteKeyWidth;
				key.rect.y = pianoStartY;
				key.rect.width = whiteKeyWidth;
				key.rect.height = whiteKeyHeight;
				whiteKeyIndex++;
			} else {
				// Touche noire - positionnée entre deux touches blanches
				key.rect.x = startX + (whiteKeyIndex - 1) * whiteKeyWidth + whiteKeyWidth - blackKeyWidth/2;
				key.rect.y = pianoStartY;
				key.rect.width = blackKeyWidth;
				key.rect.height = blackKeyHeight;
			}
			
			pianoKeys.push_back(key);
		}
	}
}


//--------------------------------------------------------------
float ofApp::calculateFrequency(int semitonesFromA4){
	// Formule: f = 440 * 2^(n/12) où n est le nombre de demi-tons depuis A4
	return 440.0f * pow(2.0f, semitonesFromA4 / 12.0f);
}

//-------------- SAM update ------------------------------------------------
void ofApp::update(){
	
	// check if the lAudio container is not empty
	
	if (!lAudio.empty()) { 
	    
	    fft->setSignal(&lAudio[0]); // sets the input signal for the FFT object
	    
	    float* amplitude = fft->getAmplitude(); // Retrieves the amplitude spectrum from the FFT

	    // copy the FFT amplitude data into another array fftMagnitude
	    
	    for (int i = 0; i < fft->getBinSize(); i++) {
	        
	        fftMagnitude[i] = amplitude[i];
	    }
	    
	    //fft->update(); //update the FFT object
	}
}


// --------------------------- Dra Frequency Spectrum -------------------------
void ofApp::drawFrequencySpectrum(){
	
	if (fftMagnitude.empty()) return; // empty fft magnitude; nothing to draw yet

        ofNoFill();

        ofPushStyle();

        ofPushMatrix();

        ofTranslate(50, 120, 0); // position on screen

        ofSetColor(225);

        ofDrawBitmapString("Frequency spectrum", 4, -10);

        ofSetLineWidth(1);

        ofSetColor(80, 80, 80);

        ofDrawRectangle(0, 0, 1300, 200);

        ofSetColor(0, 255, 100);

        ofSetLineWidth(2);

        ofBeginShape();

        for (unsigned int i = 0; i < fftMagnitude.size(); i++) {

            float x = ofMap(i, 0, fftMagnitude.size(), 0, 1300, true);

            float y = 200 - ofClamp(fftMagnitude[i] * 30, 0, 200);

            ofVertex(x, y);

        }

        ofEndShape(false);

        ofPopMatrix();

        ofPopStyle();


}



//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(225);
	ofDrawBitmapString("PIANO SYNTH - 3 OCTAVES", 32, 32);
	ofDrawBitmapString("Utilisez les touches du clavier: " + string("awsedftgyhujkolp;'[]\\zsxcfvgbnjmk,."), 32, 52);
	ofDrawBitmapString("Volume: " + ofToString(volume, 2) + " (touches +/- pour modifier)", 32, 72);
	   
	// Dessiner les formes d'onde
	//drawWaveform();
	drawFrequencySpectrum();
	   
	// Dessiner le piano
	drawPiano();
}


//--------------------------------------------------------------
void ofApp::drawWaveform(){
	ofNoFill();
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(50, 120, 0);
	
		
	ofSetColor(225);
	ofDrawBitmapString("Waveform", 4, -10);
	
	ofSetLineWidth(1);
	ofSetColor(80, 80, 80);
	ofDrawRectangle(0, 0, 1300, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(2);
	
	ofBeginShape();
	for (unsigned int i = 0; i < lAudio.size(); i++){
		float x = ofMap(i, 0, lAudio.size(), 0, 1300, true);
		ofVertex(x, 100 - lAudio[i] * 90.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawPiano(){
	// Dessiner d'abord les touches blanches
	for(auto& key : pianoKeys){
		if(!key.isBlack){
			if(key.isPressed){
				ofSetColor(100, 150, 255);
			} else {
				ofSetColor(240, 240, 240);
			}
			ofDrawRectangle(key.rect);
			
			// Bordure
			ofSetColor(40, 40, 40);
			ofNoFill();
			ofDrawRectangle(key.rect);
			ofFill();
			
			// Texte de la touche
			ofSetColor(40, 40, 40);
			ofDrawBitmapString(string(1, key.keyChar),
							 key.rect.x + key.rect.width/2 - 4,
							 key.rect.y + key.rect.height - 10);
			
			// Nom de la note
			ofDrawBitmapString(key.noteName,
							 key.rect.x + 5,
							 key.rect.y + key.rect.height - 30);
		}
	}
	
	// Puis dessiner les touches noires par-dessus
	for(auto& key : pianoKeys){
		if(key.isBlack){
			if(key.isPressed){
				ofSetColor(100, 180, 255);
			} else {
				ofSetColor(20, 20, 20);
			}
			ofDrawRectangle(key.rect);
			
			// Bordure
			ofSetColor(0, 0, 0);
			ofNoFill();
			ofDrawRectangle(key.rect);
			ofFill();
			
			// Texte de la touche
			ofSetColor(240, 240, 240);
			ofDrawBitmapString(string(1, key.keyChar),
							 key.rect.x + key.rect.width/2 - 4,
							 key.rect.y + key.rect.height - 10);
			
			// Nom de la note
			ofDrawBitmapString(key.noteName,
							 key.rect.x + 2,
							 key.rect.y + key.rect.height - 30);
		}
	}
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	if (key == '-' || key == '_') {
		volume -= 0.05;
		volume = std::max(volume, 0.0f);
	} else if (key == '+' || key == '='){
		volume += 0.05;
		volume = std::min(volume, 1.0f);
	}
	
	// Vérifier si c'est une touche de piano
	if(keyToIndex.find(key) != keyToIndex.end()){
		int index = keyToIndex[key];
		pianoKeys[index].isPressed = true;
		
		// Initialiser la phase pour cette note
		float frequency = pianoKeys[index].frequency;
//		audioMutex.lock();
		activePhases[index] = 0.0f;
		activePhaseAdders[index] = (frequency / (float)sampleRate) * glm::two_pi<float>();
//		audioMutex.unlock();
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){
	// Vérifier si c'est une touche de piano
   if(keyToIndex.find(key) != keyToIndex.end()){
	   int index = keyToIndex[key];
	   pianoKeys[index].isPressed = false;
	   
	   // Retirer la note active
//	   audioMutex.lock();
	   activePhases.erase(index);
	   activePhaseAdders.erase(index);
//	   audioMutex.unlock();
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
	// Vérifier que nos buffers audio sont de la bonne taille
		if(lAudio.size() != buffer.getNumFrames()){
			lAudio.resize(buffer.getNumFrames(), 0.0f);
			rAudio.resize(buffer.getNumFrames(), 0.0f);
		}
		
		// Verrouiller l'accès aux données audio
//		audioMutex.lock();
		for (size_t i = 0; i < buffer.getNumFrames(); i++){
			float sample = 0.0f;
			
			// Additionner toutes les notes actives
			for(auto& pair : activePhases){
				int index = pair.first;
				float& phase = pair.second;
				float phaseAdder = activePhaseAdders[index];
				
				phase += phaseAdder;
				
				// Garder la phase dans la plage 0-2π
				while(phase > glm::two_pi<float>()){
					phase -= glm::two_pi<float>();
				}
				
				sample += sin(phase);
			}
			
			// Normaliser si plusieurs notes sont jouées
			if(activePhases.size() > 0){
				sample /= sqrt((float)activePhases.size());
			}
			
			sample *= volume;
			
			// Sauvegarder pour visualisation
			lAudio[i] = sample;
			rAudio[i] = sample;
			
			// Écrire dans le buffer de sortie
			buffer[i * buffer.getNumChannels()] = sample;
			buffer[i * buffer.getNumChannels() + 1] = sample;
		}
//		audioMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

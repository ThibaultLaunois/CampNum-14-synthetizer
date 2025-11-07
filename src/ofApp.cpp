#include "ofApp.h"
#include <vector>

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	ofSetWindowTitle("Mini Piano Synth");
	ofSetWindowShape(1400, 700);

	int bufferSize		= 512;
	sampleRate 			= 10000;
	//	phase 				= 0;
	//	phaseAdder 			= 0.0f;
	//	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	//	bNoise 				= false;

	// mettre nos buffers audio à la bonne taille
	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);

	// additive synthesis
	numberOfHarmonics = 2;

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
}


//--------------------------------------------------------------
void ofApp::setupPianoKeys(){
	// Notes dans une octave: C, C#, D, D#, E, F, F#, G, G#, A, A#, B
	// Pattern des touches noires: après C, D, F, G, A (pas après E et B)
	vector<string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	vector<bool> isBlackKey = {false, true, false, true, false, false, true, false, true, false, true, false};
	
	// Clavier QWERTY pour 3 octaves (36 notes)
	string keyboardKeys = "awsedftgyhujkolp;'[]\\z$xcpvùbnjmk,.";
	
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
			if(keyIndex < (int) keyboardKeys.length()){
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

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(225);
	ofDrawBitmapString("PIANO SYNTH - 3 OCTAVES", 32, 32);
	ofDrawBitmapString("Utilisez les touches du clavier: " + string("awsedftgyhujkolp;'[]\\zsxcfvgbnjmk,."), 32, 52);
	ofDrawBitmapString("Volume: " + ofToString(volume, 2) + " (touches +/- pour modifier)", 32, 72);
	
	// Dessiner les formes d'onde
	drawWaveform();
	
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
			ofFill();
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
			ofDrawBitmapString(
				string(1, key.keyChar),
				key.rect.x + key.rect.width/2 - 4,
				key.rect.y + key.rect.height - 10);
			
			// Nom de la note
			ofDrawBitmapString(
				key.noteName,
				key.rect.x + 5,
				key.rect.y + key.rect.height - 30);
		}
	}
	
	// Puis dessiner les touches noires par-dessus
	for(auto& key : pianoKeys){
		if(key.isBlack){
			ofFill();
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
			ofDrawBitmapString(
				string(1, key.keyChar),
				key.rect.x + key.rect.width/2 - 4,
				key.rect.y + key.rect.height - 10);
			
			// Nom de la note
			ofDrawBitmapString(
				key.noteName,
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
		activePhases[index] = 0;
		activePhaseAdders[index] = (frequency/ (float) sampleRate) * glm::two_pi<float>();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){
	// Vérifier si c'est une touche de piano
    if(keyToIndex.find(key) != keyToIndex.end())
	{
		int index = keyToIndex[key];
		pianoKeys[index].isPressed = false;
		// Retirer la note active
		activePhases.erase(index);
		activePhaseAdders.erase(index);
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
// void ofApp::windowResized(int w, int h){

// }

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer){
	for (size_t i = 0; i < buffer.getNumFrames(); i++){
		float sample = 0.0f;
		
		// Additionner toutes les notes actives
		for(auto &pair : activePhases){
			int index = pair.first;
			float &phase = pair.second;
			float phaseAdder = activePhaseAdders[index];
			
			//std::cout << "index: "<< index << " phase: " << phase << std::endl;
			// add the fundamental of the note
			phase = phase + phaseAdder;
			
			// Garder la phase dans la plage 0-2π
			phase = fmod(phase, glm::two_pi<float>());

			// add fundamental note
			sample+=sin(phase)*0.80;

			// add harmonics
			for (int k = 2; k < numberOfHarmonics+2; k++){
				sample += sin(fmod(phase*k, glm::two_pi<float>()))/(0.2/numberOfHarmonics);
			}
			
			// Normalized the sound
			sample /= activePhases.size();
			sample /= (numberOfHarmonics + 1);

			//add harmonics
			// for (int j=1; j <= numberOfHarmonics; j++){
			// 	phase = phase*(1+j) + phaseAdder;
				
			// 	// Garder la phase dans la plage 0-2π
			// 	phase = fmod(phase, glm::two_pi<float>());

			// 	// Normaliser si plusieurs notes sont jouées
			// 	sample += sin(phase)/activePhases.size()/(numberOfHarmonics+1)*0.9*i;
			// }
		}
		
		// Sauvegarder pour visualisation + écrire dans le buffer
		lAudio[i] = buffer[i * buffer.getNumChannels()    ] = sample * volume;
		rAudio[i] = buffer[i * buffer.getNumChannels() + 1] = sample * volume;
	}

	// Yiran code
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-glm::two_pi<float>() like this:

    // for (size_t i = 0; i < buffer.getNumFrames(); i++){
    //     float sample = 0.0f;
    //     for (const auto &key: pressedKeys) {
    //         freqPhases[key] = fmod(freqPhases[key] + freqPhaseAdders[key], glm::two_pi<float>());
    //         sample += sin(freqPhases[key]) / pressedKeys.size();
    //     }
    //     lAudio[i] = buffer[i*buffer.getNumChannels()    ] = sample * volume;
    //     rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = sample * volume;
    // }
}

//--------------------------------------------------------------
// void ofApp::gotMessage(ofMessage msg){

// }

// //--------------------------------------------------------------
// void ofApp::dragEvent(ofDragInfo dragInfo){ 

// }

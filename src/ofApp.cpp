#include "ofApp.h"
#include "ofxGui.h"  // <--- ajouté par Marie
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


	// === UI === ajouté par Marie
	gui.setup("Synth UI");
	gui.setPosition(50, 330);

	// lie l’UI à des ofParameter (plus portable que des lambdas sur les widgets)
	gui.add(pVolume.set("Volume", volume, 0.0f, 1.0f));
	gui.add(pSustain.set("Sustain", false));
	gui.add(pOctave.set("Octave offset", 0, -1, 1));
	gui.add(startBtn.setup("Start audio"));
	gui.add(stopBtn.setup("Stop audio"));

	// Listeners (sur les parameters et boutons)
	pVolume.addListener(this, &ofApp::onVolumeChanged);
	pOctave.addListener(this, &ofApp::onOctaveChanged);
	startBtn.addListener(this, &ofApp::onStartPressed);
	stopBtn.addListener(this, &ofApp::onStopPressed);
	// fin ajout Marie


	
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
	settings.setApi(ofSoundDevice::MS_WASAPI);
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
			//if(keyIndex < keyboardKeys.length()){
			if (static_cast<size_t>(keyIndex) < keyboardKeys.length()) { //modifié par marie sinon erreur de size
				key.keyChar = keyboardKeys[keyIndex];
				//keyToIndex[key.keyChar] = pianoKeys.size();
				keyToIndex[key.keyChar] = static_cast<int>(pianoKeys.size());// modifé par marie sinon erreur de size
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




// création des fréquences des 3 octavces quand on bouge le slider octaveOffset=> Marie

void ofApp::rebuildFrequencies(int baseOctaveOffset) { // ajouté par Marie
	// on refait exactement la même logique que setupPianoKeys mais on ne touche
	// qu'aux fréquences et aux labels (pas aux rectangles ni aux mappings)
	vector<string> noteNames = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };

	int keyIdx = 0;
	for (int octave = 3 + baseOctaveOffset; octave <= 5 + baseOctaveOffset; ++octave) {
		for (int note = 0; note < 12; ++note) {
			if (keyIdx >= (int)pianoKeys.size()) return;
			int semitonesFromA4 = (octave - 4) * 12 + (note - 9);
			pianoKeys[keyIdx].frequency = calculateFrequency(semitonesFromA4);
			pianoKeys[keyIdx].noteName = noteNames[note] + ofToString(octave);
			++keyIdx;
		}
	}
}// fin modif marie





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

	   gui.draw();// ajouté par marie pour dessiner le panneau
	   // petit VU mètre RMS ajouté par Marie
	   float rms = 0.0f;
	   for (auto s : lAudio) rms += s * s;
	   rms = (lAudio.empty() ? 0.0f : sqrtf(rms / lAudio.size()));
	   ofSetColor(225);
	   ofDrawBitmapString("RMS: " + ofToString(rms, 3), 300, 90);
	   ofSetColor(80, 80, 80); ofDrawRectangle(350, 75, 150, 10);
	   ofSetColor(100, 200, 120); ofDrawRectangle(350, 75, ofMap(rms, 0, 0.6, 0, 150, true), 10); // fin modif marie

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
			} 
			else if (&key - &pianoKeys[0] == hoveredKey) ofSetColor(255, 255, 230); // ajout par marie => pour colorer la souris en noirs sur notes blanches
			else {
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
			} 
			else if (&key - &pianoKeys[0] == hoveredKey) ofSetColor(40, 40, 40); // ajout par marie => pour colorer la souris en blanc sur notes noirs
			else {
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

////--------------------------------------------------------------
//void ofApp::keyReleased  (int key){
//	// Vérifier si c'est une touche de piano
//   if(keyToIndex.find(key) != keyToIndex.end()){
//	   int index = keyToIndex[key];
//	   pianoKeys[index].isPressed = false;
//	   
//	   // Retirer la note active
////	   audioMutex.lock();
//	   activePhases.erase(index);
//	   activePhaseAdders.erase(index);
////	   audioMutex.unlock();
//   }
//}


//modification de KeyRealeased par Marie
void ofApp::keyReleased(int key) {
	// Vérifier si c'est une touche de piano
	auto it = keyToIndex.find(key);
	if (it != keyToIndex.end()) {
		const int index = it->second;

		if (index >= 0 && index < static_cast<int>(pianoKeys.size())) {
			pianoKeys[index].isPressed = false;
		}

		// sustain : si OFF on coupe la note sinon son en continue
		if (!pSustain.get()) {
			// 1) retirer la phase de la map
			activePhases.erase(index);

			// 2) mettre à zéro l'adder correspondant
			if (index >= 0 && static_cast<size_t>(index) < activePhaseAdders.size()) {
				activePhaseAdders[index] = 0.0f;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){ // modifié par marie
	//int width = ofGetWidth();
	//pan = (float)x / (float)width;
	//float height = (float)ofGetHeight();
	//float heightPct = ((height-y) / height);
	//targetFrequency = 2000.0f * heightPct;
	//phaseAdderTarget = (targetFrequency / (float) sampleRate) * glm::two_pi<float>();

	hoveredKey = -1;
	// trouver la touche sous la souris (d'abord noires, au-dessus des blanches)
	for (int pass = 0; pass < 2; ++pass) {
		for (int i = 0; i < (int)pianoKeys.size(); ++i) {
			auto& k = pianoKeys[i];
			if ((pass == 0 && k.isBlack) || (pass == 1 && !k.isBlack)) {
				if (k.rect.inside(x, y)) { hoveredKey = i; return; }
			}
		}
	}
} // fin de modification marie

//--------------------------------------------------------------
//void ofApp::mouseDragged(int x, int y, int button){
//	int width = ofGetWidth();
//	pan = (float)x / (float)width;
//}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){ // modifié par marie
//	bNoise = true;
	mouseIsDown = true;
	if (hoveredKey >= 0) {
		auto& key = pianoKeys[hoveredKey];
		key.isPressed = true;
		float frequency = key.frequency;
		int index = hoveredKey;

		// même logique que keyPressed() pour activer une note mais avec la souris
		activePhases[index] = 0.0f;
		activePhaseAdders[index] = (frequency / (float)sampleRate) * glm::two_pi<float>();
	}
}// fin de modification marie


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){ // modifié par marie
//	bNoise = false;
	mouseIsDown = false;
	if (hoveredKey >= 0 && hoveredKey < static_cast<int>(pianoKeys.size())) {
		auto& key = pianoKeys[hoveredKey];
		key.isPressed = false;

		const int index = hoveredKey;

		// sustain : si OFF on coupe la note
		if (!pSustain.get()) {
			activePhases.erase(index);
			//  met à zéro l'adder correspondant
			if (index >= 0 && static_cast<size_t>(index) < activePhaseAdders.size()) {
				activePhaseAdders[index] = 0.0f;
			}
		}
	}
}// fin de modification marie

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

// ajout par marie
void ofApp::onVolumeChanged(float& v) {
	volume = v;
}

void ofApp::onOctaveChanged(int& off) {
	rebuildFrequencies(off);
	octaveOffset = off;
}

void ofApp::onStartPressed() {
	soundStream.start();
}

void ofApp::onStopPressed() {
	soundStream.stop();
	activePhases.clear();
	activePhaseAdders.clear();
	std::fill(lAudio.begin(), lAudio.end(), 0.0f);
	std::fill(rAudio.begin(), rAudio.end(), 0.0f);
}
// fin ajout marie

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

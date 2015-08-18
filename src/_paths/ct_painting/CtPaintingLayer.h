#pragma once

#include "CustomPaths.h"

using namespace guardacaso;

class CtPaintingLayer : public CustomPaths {
	
public:
	
    CtPaintingLayer(string name);
    ~CtPaintingLayer(){}
	
    void setup();
    void update(ofPolylines_ptr lines, map<string, DrawingObject_ptr> &clients);
    void draw(ofPolylines_ptr lines, map<string, DrawingObject_ptr> &clients);
    void idle();
    void resume();

private:

    ofParameter<float> stroke1_w;
    ofParameter<float> stroke2_w;
    ofParameter<float> control_size;
    ofParameter<float> alpha1;
    ofParameter<float> alpha2;
    ofParameter<float> pulse_size;
    ofParameter<bool> show_lines;
    ofParameter<bool> show_controls;
	
};

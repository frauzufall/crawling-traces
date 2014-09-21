#pragma once

#include "ofMain.h"

class LightObject
{
    public:

        LightObject();

        virtual void update() = 0;

        string getId();
        string getName();
        string getType();
        int getHue();
        ofColor getColor();
        ofPoint getLastPos();
        ofPoint getPos();

        float getLastActionTime();
        float getTimeIdle();
        float getTimeGone();
        string getRestTimeAsString();
        void setFadingOut(bool fading);
        bool getFadingOut();
        void setFadingOutStart(float status);
        float getFadingOutStart();
        bool isGone();

        void setName(string n);
        void setType(string t);
        void setColor(ofColor c);
        void setId(string id);
        void setGone(bool is_gone);

        ~LightObject(){}

    protected:
        string _id;
        string myname;
        string mytype;
        ofColor color;
        ofPoint pos;
        ofPoint last_pos;
        float last_action;
        bool fading_out;
        float fading_out_start;
        bool gone;
        ofColor hsvToRgb(float h, float s, float v);
};

#include "DrawingObject.h"
#include "ObjectController.h"
#include "MappingController.h"
#include "ServerController.h"
#include "Traces.h"
#include "Stuff.h"

#include <algorithm>

using namespace guardacaso;

DrawingObject::DrawingObject() {
}

void DrawingObject::setup(string id) {
    _id = id;
    if(id == "wheels") {
        setNewRandomColor();
    }
    pos.x = MappingController::getInstance().getProjector(0)->getStartPoint().x* Visuals::get().outputWidth();
    pos.y = MappingController::getInstance().getProjector(0)->getStartPoint().y* Visuals::get().outputHeight();

    //send starting position to server
    stringstream possent;
    possent << _id << ";" << pos.x << "|" << pos.y;
    ServerController::getInstance().send(ServerController::getInstance().getClientName(), "moveto", possent.str());

    setPulsing();
    please_redraw = false;
    pulseval = 0;
    pulsestart = 0;
    range = 0;
    backup_line.clear();
    line.clear();
    history_line.clear();
    history_timestamp.clear();
    history_net_line.clear();
    history_net_timestamp.clear();
}

void DrawingObject::update() {

    range = 100*ObjectController::getInstance().getDrawingRange();
    if(getId() == "wheels")
        range = 100*ObjectController::getInstance().getDrawingRangeWheels();

    speed = ObjectController::getInstance().getDrawingSpeed();
    if(getId() == "wheels")
        speed = ObjectController::getInstance().getDrawingSpeedWheels()*3;

}


void DrawingObject::setPos(ofPoint p) {

    ofPoint p_d = p;

    int max_distance = range;

    last_pos = pos;
    speed = speed<0.01?0.01:speed;

    if(abs(p_d.x)>1)
        p_d.x *= speed;
    if(abs(p_d.y)>1)
        p_d.y *= speed;

    ofPoint new_pos = last_pos+p_d;
    ofPoint new_pos_corrected = new_pos;
    if(new_pos != pos) {
        new_pos_corrected = MappingController::getInstance().getPointInMappedArea(last_pos,new_pos);
    }

//    if(new_pos_corrected != pos) {

    string timestamp = ofGetTimestampString();

    last_action = ofGetElapsedTimef();

    pos = new_pos_corrected;

    backup_line.addVertex(pos);
    history_line.addVertex(pos);
    history_timestamp.push_back(timestamp);
    history_net_line.addVertex(pos);
    history_net_timestamp.push_back(timestamp);
    line.addVertex(pos);

    //send new position to server
    stringstream possent;
    possent << _id << ";" << pos.x << "|" << pos.y;
    ServerController::getInstance().send(ServerController::getInstance().getClientName(), "lineto", possent.str());

    //create connection lines to existing drawing points
    for(uint i = 0; i < backup_line.getVertices().size(); i++) {
        ofPoint p_tmp = backup_line.getVertices().at(i);
        if(p_tmp.distance(pos) < max_distance) {
            line.addVertex(p_tmp);
            line.addVertex(pos);
            history_net_line.addVertex(p_tmp);
            history_net_timestamp.push_back(timestamp);
            history_net_line.addVertex(pos);
            history_net_timestamp.push_back(timestamp);
        }
    }

    backup_line.simplify();

    Projector* pj = MappingController::getInstance().getProjector(0);
    MappingQuad_ptr q;

    //add lines to borders of obstacles if they are nearby
    for(uint i = 0; i < pj->quadCount(); i++) {
        q = pj->getQuad(i);
        if(q) {
            ofPolyline polyline = Visuals::get().outlinesRaw()->at(i);
            if(q->nature == "window") {
                for(uint j = 0; j < polyline.size(); j++) {
                    vector<ofPoint> points_between = Stuff::getPointsBetween(polyline[j],
                                                                             polyline[(j+1)%polyline.size()],
                                                                             10);
                    for(uint k = 0; k < points_between.size(); k++) {
                        if(pos.distance(points_between[k]) < max_distance) {
                            line.addVertex(points_between[k]);
                            line.addVertex(pos);
                            history_net_line.addVertex(points_between[k]);
                            history_net_timestamp.push_back(timestamp);
                            history_net_line.addVertex(pos);
                            history_net_timestamp.push_back(timestamp);
                        }
                    }
                }
            }
        }
    }

    please_redraw = true;

//    }
//    else {
//        pos = last_pos;
//    }

}

void DrawingObject::setFinishingPos() {

    string timestamp = ofGetTimestampString();
    pos = last_pos;

    history_line.addVertex(pos);
    history_timestamp.push_back(timestamp);
    history_net_line.addVertex(pos);
    history_net_timestamp.push_back(timestamp);

}


void DrawingObject::clearLines() {
    line.clear();
    backup_line.clear();
}

void DrawingObject::setNewRandomColor() {
    setColor(ofColor::fromHsb(ofRandom(0,255),255,255));
}

bool DrawingObject::needsRedraw() {
    return please_redraw;
}

void DrawingObject::gotRedrawn() {
    please_redraw = false;
}

void DrawingObject::setPulsing() {
    pulsestart = ofGetElapsedTimef();
    pulseval = 1;
    //send to server that client is pulsing
    ServerController::getInstance().send(ServerController::getInstance().getClientName(), "pulsing", _id);
}

float DrawingObject::getPulseStart() {
    return pulsestart;
}

float DrawingObject::getPulseVal() {
    return pulseval;
}

void DrawingObject::setPulseVal(float val) {
    pulseval = val;
}

ofColor DrawingObject::getModColor() {
    return color;
}

ofPolyline DrawingObject::getLine() {
    return backup_line;
}

ofPolyline DrawingObject::getConnections() {
    return line;
}

DrawingObject::~DrawingObject() {

    cout << "DRAWINGOBJECT:: destroyed" << endl;
    closeAndSave();

}

void DrawingObject::saveTimestamps(string path) {
    ofxXmlSettings xml;

    xml.clear();

    xml.addTag("timestamps");
    xml.pushTag("timestamps", 0);

    for(uint i = 0; i < history_timestamp.size(); i++) {
        xml.addValue("p",history_timestamp.at(i));
    }

    xml.popTag();

    xml.saveFile(path);
}

void DrawingObject::saveNetTimestamps(string path) {
    ofxXmlSettings xml;

    xml.clear();

    xml.addTag("timestamps");
    xml.pushTag("timestamps", 0);

    for(uint i = 0; i < history_net_timestamp.size(); i++) {
        xml.addValue("p",history_net_timestamp.at(i));
    }

    xml.popTag();

    xml.saveFile(path);
}

void DrawingObject::closeAndSave() {

    if(getLine().size() > 0) {
        this->setFinishingPos();

        stringstream unique_id, test_path, svg_path, timestamp_path, svg_net_path, timestamp_net_path;

        int file_count = 1;
        unique_id << getId();
        test_path << Traces::get().historyDir() << "/" << unique_id.str() << ".svg";
        while(ofFile::doesFileExist(test_path.str())) {
            unique_id.str("");
            test_path.str("");
            unique_id << getId() << "_" << file_count;
            test_path << Traces::get().historyDir() << "/" << unique_id.str() << ".svg";
            ++file_count;
        }

        svg_path << Traces::get().historyDir() << "/" << unique_id.str() << ".svg";
        svg_net_path << Traces::get().historyDir() << "/net_" << unique_id.str() << ".svg";
        timestamp_path << Traces::get().historyDir() << "/" << unique_id.str() << ".xml";
        timestamp_net_path << Traces::get().historyDir() << "/net_" << unique_id.str() << ".xml";

        cout << "DRAWINGOBJECT::saving object " << unique_id.str() << "..." << endl;

        Stuff::saveLineAsSvg(svg_path.str(),history_line,Visuals::get().outputWidth(), Visuals::get().outputHeight(),getColor());
        Stuff::saveLineAsSvg(svg_net_path.str(),history_net_line,Visuals::get().outputWidth(), Visuals::get().outputHeight(),getColor());
        saveTimestamps(timestamp_path.str());
        saveNetTimestamps(timestamp_net_path.str());
    }

}

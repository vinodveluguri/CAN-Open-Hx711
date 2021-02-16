#include "LoggingRobot.h"

LoggingRobot::LoggingRobot() {
    spdlog::info("New Logging Robot");

    initialiseJoints();
    initialiseInputs();
};

bool LoggingRobot::initialiseInputs() {
    spdlog::info("test");

    inputs.push_back(keyboard = new Keyboard());
    spdlog::info("test");

    inputs.push_back(strainGauge = new HX711(2,10,2,6)); //clock pin 2_10, dout pin 2_6

    spdlog::info("test");
    strainGauge->begin("","",128);

    // Add two crutch sensors
    crutchSensors.push_back(new RobotousRFT(0xf8, 0xf9, 0xfa));
    crutchSensors.push_back(new RobotousRFT(0xf0, 0xf1, 0xf2));

    // Add to input stack
    for (int i = 0; i < 2; i++) {
        inputs.push_back(crutchSensors[i]);
    }

    return true;
}

LoggingRobot::~LoggingRobot() {
    spdlog::debug("Delete LoggingRobot object begins");
    for (auto p : joints) {
        spdlog::info("Delete Joint ID: {}", p->getId());
        delete p;
    }

    joints.clear();
    delete keyboard;
    for (auto cs : crutchSensors) {
        spdlog::info("Delete Crutch Sensor with CommandID: 0x{0:x}", cs->getCommandID());
        delete cs;
    }
    delete strainGauge;
    inputs.clear();
    spdlog::debug("LoggingRobot deleted");
}

Eigen::VectorXd& LoggingRobot::getCrutchReadings() {
    updateCrutchReadings();
    return crutchReadings;
}

void LoggingRobot::updateCrutchReadings(){
    if ((unsigned int)crutchReadings.size() != 6 * crutchSensors.size()) {
        crutchReadings = Eigen::VectorXd::Zero(6 * crutchSensors.size());  // 6 Forces per sensor
    }
    //Update values
    for (int i = 0; i < (int)crutchSensors.size(); i++) {
        Eigen::VectorXd forces = crutchSensors[i]->getForces();
        Eigen::VectorXd torques = crutchSensors[i]->getTorques();
        for (int j = 0; j < 3; j++) {
            crutchReadings[i * 6 + j] = forces[j];
            crutchReadings[i * 6 + 3 + j] = torques[j];
        }
    }
}

void LoggingRobot::setCrutchOffsets(Eigen::VectorXd offsets) {
    for (unsigned int i = 0; i < crutchSensors.size(); i++) {
        crutchSensors[i]->setOffsets(offsets.segment(i * 6, 3), offsets.segment(i * 6+3, 3));
    }
}
bool LoggingRobot::startSensors() {
    if (sensorsOn){
        //do nothing
        return false;
    } else
    {
        for (unsigned int i = 0; i < crutchSensors.size(); i++) {
            crutchSensors[i]->startStream();
        }
        sensorsOn = true; 
        return true;
    }
}
bool LoggingRobot::stopSensors() {
    if (sensorsOn) {
        for (unsigned int i = 0; i < crutchSensors.size(); i++) {
            crutchSensors[i]->stopStream();
        }
        crutchReadings = Eigen::VectorXd::Zero(6 * crutchSensors.size());
        sensorsOn = false;
        return true;
    } else {
        return false;
    }
}
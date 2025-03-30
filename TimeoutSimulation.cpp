/*
    COEN/ELEC 390 - Team 11 - Winter 2025
    Computer Simulation: Idea generation and selection for light timeout

    This program compares two methods for controlling the timeout of a light:
    1. Fixed timeout: The light is turned off after a fixed period of time when motion is not detected
    2. Adaptive timeout: The timeout value is extended if motion is detected

    Figures of merit:
    1. Energy consumption: The total amount of time tgat the light is on over a period of time
    2. User satisfaction: The number of times the light is turned off while s person is still present in the room
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime> 
using namespace std;

// Define constant values for the simulation
const int simulationTime = 1800;    // Total simulation time in seconds (30 minutes)
const int simulationInterval = 10;  // Simulate every 10 seconds
const int dataSize = simulationTime / simulationInterval; // Number of data points that will be generated

const int fixedTimeoutValue = 20;    // Fixed timeout value in seconds
const int adaptiveTimeoutValue = 20; // Adaptive timeout value in seconds
const int maxAdaptiveTimeoutValue = 60; // Maximum adaptive timeout value in seconds
const int personInsideThreshold = 40; // Time in seconds to consider a person inside the room


// Function to generate random movement data
vector<int> generateMovementData() {

    srand(time(0)); // seed the random number generator with current time

    vector<int> movementData;
    for (int i = 0; i < dataSize; i++) {
        movementData.push_back(rand() % 2); // generate 0s or 1s randomly
    }
    return movementData;
}

// Function to simulate person inside the room
vector<bool> generatePersonInside(vector<int> movementData) {
    vector<bool> personInside;
    int lastDetectedMotion = -personInsideThreshold; 

    for (int i = 0; i < dataSize; i++) {
        int currentTime = i * simulationInterval;

        if (movementData[i] == 1) {
            lastDetectedMotion = currentTime;
            personInside.push_back(true);
        } else {
            if ((currentTime - lastDetectedMotion) <= personInsideThreshold) {
                personInside.push_back(true);
            } else {
                personInside.push_back(false);
            }
        }  
    }

    return personInside;
}

// Function to simulate the fixed timeout method:
// When motion is detected (movementData = 1), turn light on
// When motion no longer detected (movementData = 0), start counting (fixedTimeoutValue)
// Turn light off when fixedTimeoutValue is reached
vector<int> fixedTimeout(vector<int> movementData) {
    vector<int> fixedLightState;
    int timeElapsed = 0; // amount of time that has passed since the last movement

    for (int i = 0; i < dataSize; i++) {
        if (movementData[i] == 1) {
            fixedLightState.push_back(1); // if motion is detected, light turns on immediately
            timeElapsed = 0; // reset the amount of time that has passed since the last movement
        } else {
            timeElapsed += simulationInterval; // increment the amount of time that has passed since the last movement
            if (timeElapsed >= fixedTimeoutValue) { // if movement is not detected for fixedTimeoutValue (20s), light turns off
                fixedLightState.push_back(0);
            } else {
                fixedLightState.push_back(1); // light stays on
            }
        }
    }

    return fixedLightState;
}

// Function to simulate the adaptive timeout method
// Start counting (adaptiveTimeoutValue)
// If motion is detected, increment adaptiveTimeoutValue
// Turn light off when adaptiveTimeoutValue is reached
vector<int> adaptiveTimeout(vector<int> movementData) {
    vector<int> adaptiveLightState;
    int countDown = adaptiveTimeoutValue; // default timeout value

    for (int i = 0; i < dataSize; i++) {
        if (movementData[i] == 1) {
            adaptiveLightState.push_back(1); // if motion is detected, light turns on immediately
            countDown = min(countDown + adaptiveTimeoutValue, maxAdaptiveTimeoutValue); // increment timeout value
        } else {
            countDown -= simulationInterval; // count down
            if (countDown <= 0) { 
                adaptiveLightState.push_back(0); 
                countDown = adaptiveTimeoutValue; // reset the timeout value
            } else {
                adaptiveLightState.push_back(1); // light stays on
            }
        }
    }

    return adaptiveLightState;
}

void getStats(vector<int> fixedLightState, vector<int> adaptiveLightState, vector<int> movementData, vector<bool> personInside) {
    int fixedLightOn = 0;
    int adaptiveLightOn = 0;
    int fixedLightOff = 0;
    int adaptiveLightOff = 0;
    int fixedFlaw = 0;
    int adaptiveflaw = 0;

    for (int i = 0; i < dataSize; i++) {

        // Energy consumption
        if (fixedLightState[i] == 1) {
            fixedLightOn++;
        } else {
            fixedLightOff++;
        }

        if (adaptiveLightState[i] == 1) {
            adaptiveLightOn++;
        } else {
            adaptiveLightOff++;
        }

        // User satisfaction
        if (fixedLightState[i] == 0 && personInside[i]) { // light turned off while person is inside
            fixedFlaw++;
        }
        else if (adaptiveLightState[i] == 0 && personInside[i]) { // light turned off while person is inside
            adaptiveflaw++;
        }
    }

    cout << endl << "===== FIXED TIMEOUT STATISTICS =====" << endl;

    cout << "The light was on " << static_cast<double>(fixedLightOn*100/dataSize) << "% of the time" << endl;
    cout << "The light was off " << static_cast<double>(fixedLightOff*100/dataSize) << "% of the time" << endl;
    cout << "The light turned off while the person was inside the room " << fixedFlaw << " times" << endl;

    cout << endl << "===== ADAPTIVE TIMEOUT STATISTICS =====" << endl;

    cout << "The light was on " << static_cast<double>(adaptiveLightOn*100/dataSize) << "% of the time" << endl;
    cout << "The light was off " << static_cast<double>(adaptiveLightOff*100/dataSize) << "% of the time" << endl;
    cout << "The light turned off while the person was inside the room " << adaptiveflaw << " times" << endl << endl;
}


int main() {
    // Initialize random data
    vector<int> movementData = generateMovementData();
    vector<bool> personInside = generatePersonInside(movementData);
    cout << "size of movement data: " << movementData.size() << endl;
    cout << "size of person inside data: " << personInside.size() << endl;

    // Begin simulation
    cout << "===== STARTING LIGHT TIMEOUT SIMULATION =====" << endl;

    // Simulate fixed timeout method    
    vector<int> fixedLightState = fixedTimeout(movementData);
    cout << "size of fixed light state: " << fixedLightState.size() << endl;

    // Simulate adaptive timeout method
    vector<int> adaptiveLightState = adaptiveTimeout(movementData);
    cout << "size of adaptive light state: " << adaptiveLightState.size() << endl;

    // Print simulation results
    cout << "Time\tMotion\tPerson\tFixed\tAdaptive\n"; // titles
    for (int i = 0; i < movementData.size(); i++) {
        
        if (i==0) {
            if (personInside[0]) {
                cout << "PERSON HAS ENTERED THE ROOM" << endl;
            }
        }

        cout << i * simulationInterval << "\t" << movementData[i] << "\t" << personInside[i] << "\t" << fixedLightState[i] << "\t" << adaptiveLightState[i] << endl; // data

        if (i>0 && !personInside[i-1] && personInside[i]) {
            cout << "PERSON HAS ENTERED THE ROOM" << endl;
        }
        else if (i>0 && personInside[i-1] && !personInside[i]) {
            cout << "PERSON HAS LEFT THE ROOM" << endl;
        }
    }

    // Get statistics
    getStats(fixedLightState, adaptiveLightState, movementData, personInside);
    
    return 0;
}
/*
    COEN/ELEC 390 - Team 11 - Winter 2025
    Computer Simulation: Idea generation and selection for light timeout

    This program compares two methods for controlling the timeout of a light:
    1. Fixed timeout: The light is turned off after a fixed period of time when motion is not detected
    2. Adaptive timeout: The timeout value is extended every time motion is detected

    Figures of merit:
    1. Energy consumption: The total amount of time that the light is on over a period of time
    2. False negatives: The number of times the light is turned off while a person is still present in the room
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime> 
#include <random>
#include <fstream>
using namespace std;

// Define constant values for the simulation
const int simulationTime = 600;    // Total simulation time in seconds 
const int simulationInterval = 10;  // Simulate every 10 seconds
const int dataSize = simulationTime / simulationInterval; // Number of data points that will be generated

const int fixedTimeoutValue = 20;    // Fixed timeout value in seconds
const int adaptiveTimeoutValue = 20; // Adaptive timeout value in seconds
const int maxAdaptiveTimeoutValue = 60; // Maximum adaptive timeout value in seconds
const int personInsideThreshold = 40; // Time in seconds to consider a person inside the room


// Function to generate random movement data
vector<int> generateMovementData(int personEntryTime) {
    random_device rd; // random seed
    mt19937 gen(rd()); // get random values with the random_device seed
    uniform_real_distribution<> dis(0.0, 1.0); // make every number between 0 and 1 have equal chance to be generated

    vector<int> movementData(dataSize, false); // vector that holds movement data, initialize to false
    double probability = 0.4; // adjust for movement frequency

    for (int i = 0; i < dataSize; i++) {
        int currentTime = i * simulationInterval;
        if (currentTime >= personEntryTime) { // generate random movement after person enters
            if (dis(gen) < probability) { // enters the "if" block [probability]% of the time
                movementData[i] = 1;
            } else { // enters the "else" block [probability]% of the time
                movementData[i] = 0;
            }
        } else { // if the person has not entered the room yet, movement data is 0
            movementData[i] = 0; // no movement before person enters
        }
    }
    return movementData;
}

// Function to simulate person inside the room
vector<bool> generatePersonInside(vector<int> movementData, int personEntryTime) {
    vector<bool> personInside(dataSize, false);
    int lastDetectedMotion = -personInsideThreshold;

    for (int i = 0; i < dataSize; i++) {
        int currentTime = i * simulationInterval;

        if (currentTime >= personEntryTime) { // if a person has entered
            if (movementData[i] == 1) { // and motion is detected
                lastDetectedMotion = currentTime;
                personInside[i] = true;
            } else { // if no motion detected
                if ((currentTime - lastDetectedMotion) <= personInsideThreshold) { // if no motion but person is assumed to still be inside the room
                    personInside[i] = true;
                } else { // if person is assumed to have left the room
                    personInside[i] = false;
                }
            }
        }
    }
    return personInside;
}

// Function to simulate the fixed timeout method:
// When motion is detected (movementData = 1), turn light on
// When motion no longer detected (movementData = 0), start counting (fixedTimeoutValue)
// Turn light off when fixedTimeoutValue is reached
vector<int> fixedTimeout(vector<int> movementData, const vector<bool>& personInside) {
    vector<int> fixedLightState(dataSize, 0); // initialize to 0
    int timeElapsed = 0;

    for (int i = 0; i < dataSize; i++) {
        if (personInside[i]) { // if a person is inside
            if (movementData[i] == 1) { // and motion is detected
                fixedLightState[i] = 1;
                timeElapsed = 0; // reset the timeout counter
            } else { // if person is inside but no motion detected
                timeElapsed += simulationInterval;
                if (timeElapsed >= fixedTimeoutValue) { // if timeout reached, turn light off
                    fixedLightState[i] = 0;
                } else { // if timeout not reached, keep light on
                    fixedLightState[i] = 1;
                }
            }
        }
    }
    return fixedLightState;
}

// Function to simulate the adaptive timeout method
// Start counting (adaptiveTimeoutValue)
// If motion is detected, increment adaptiveTimeoutValue
// Turn light off when adaptiveTimeoutValue is reached
vector<int> adaptiveTimeout(vector<int> movementData, const vector<bool>& personInside) {
    vector<int> adaptiveLightState(dataSize, 0); // initialize to 0
    int countDown = adaptiveTimeoutValue;
    bool increased = false; // flag to track if countDown has been increased, to fix the bug of countDown being set to 30 instead of 20

    for (int i = 0; i < dataSize; i++) {
        if (personInside[i]) { // if a person is inside
            if (movementData[i] == 1) { // and motion is detected
                adaptiveLightState[i] = 1;
                if (increased) {
                    if (countDown < maxAdaptiveTimeoutValue) {
                        countDown += simulationInterval; // only increment if increased flag is true
                    } else {
                        countDown = maxAdaptiveTimeoutValue;
                    }
                } else {
                  increased = true;
                }
            } else { // if person is inside but no motion detected
                countDown -= simulationInterval;
                if (countDown <= 0) {
                    adaptiveLightState[i] = 0;
                    countDown = adaptiveTimeoutValue;
                    increased = false; // reset the flag when the light is turned off so that next time countDown starts at 20
                } else {
                    adaptiveLightState[i] = 1;
                }
            }
        }
    }
    return adaptiveLightState;
}

// Get statistics based on figures of merit, write to CSV file, and print to console
void getStats(vector<int> fixedLightState, vector<int> adaptiveLightState, vector<int> movementData, vector<bool> personInside) {
    int fixedLightOn = 0;
    int adaptiveLightOn = 0;
    int fixedLightOff = 0;
    int adaptiveLightOff = 0;
    int fixedFlaw = 0;
    int adaptiveflaw = 0;

    for (int i = 0; i < dataSize; i++) {

        // energy consumption
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

        // false negatives
        if (fixedLightState[i] == 0 && personInside[i]) { // light turned off while person is inside
            fixedFlaw++;
        }
        else if (adaptiveLightState[i] == 0 && personInside[i]) { // light turned off while person is inside
            adaptiveflaw++;
        }
        
        // write statistics to CSV file
        ofstream statsFile("statistics_data.csv");
        if (statsFile.is_open()) {
            statsFile << "Method,LightOnPercentage,LightOffPercentage,Flaws\n";
            statsFile << "Fixed," << static_cast<double>(fixedLightOn * 100) / dataSize << "," << static_cast<double>(fixedLightOff * 100) / dataSize << "," << fixedFlaw << "\n";
            statsFile << "Adaptive," << static_cast<double>(adaptiveLightOn * 100) / dataSize << "," << static_cast<double>(adaptiveLightOff * 100) / dataSize << "," << adaptiveflaw << "\n";
            statsFile.close();
        } else {
            cerr << "Error: Unable to open statistics data file" << endl;
        }
    
    }

    // print statistics to console
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
    int personEntryTime = 20; // person enters at 20 seconds into the simulation

    // initialize random data
    vector<int> movementData = generateMovementData(personEntryTime);
    vector<bool> personInside = generatePersonInside(movementData, personEntryTime);

    // begin simulation
    cout << "===== STARTING LIGHT TIMEOUT SIMULATION =====" << endl;

    // simulate fixed timeout method    
    vector<int> fixedLightState = fixedTimeout(movementData, personInside);

    // simulate adaptive timeout method
    vector<int> adaptiveLightState = adaptiveTimeout(movementData, personInside);

    // print simulation results
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

    // write data to CSV file
    ofstream outputFile("simulation_data.csv"); 
    if (outputFile.is_open()) {
        outputFile << "Time,Motion,Person,Fixed,Adaptive\n"; 
        for (int i = 0; i < movementData.size(); i++) {
            outputFile << i * simulationInterval << "," << movementData[i] << "," << personInside[i] << "," << fixedLightState[i] << "," << adaptiveLightState[i] << "\n";
        }
        outputFile.close();
        cout << "Simulation data written to simulation_data.csv" << endl;
    } else {
        cerr << "Error: Unable to open simulation data file" << endl;
    }

    // get statistics
    getStats(fixedLightState, adaptiveLightState, movementData, personInside);
    
    return 0;
}
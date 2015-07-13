#pragma once

class ObserverSearch {
public:
    virtual void receiveObserverSearch(int threadID) = 0;

    virtual void receiveObserverPVSplit(int threadID, int value) = 0;
};

class Subject {
public:
    void registerObserver(ObserverSearch *obs) {
        observer = obs;
    }

    void notifyPVSplit(int threadID, int value) {
        observer->receiveObserverPVSplit(threadID, value);
    }

    void notifySearch(int threadID) {
        observer->receiveObserverSearch(threadID);
    }

private:
    ObserverSearch *observer;
};
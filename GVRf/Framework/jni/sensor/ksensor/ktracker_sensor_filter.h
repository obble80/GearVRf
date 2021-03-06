/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KTRACKER_SENSOR_FILTER_H_
#define KTRACKER_SENSOR_FILTER_H_

#include <stdlib.h>

namespace gvr {

template<typename T>
class CircularBuffer {
protected:
    enum {
        DefaultFilterCapacity = 20
    };

    int LastIdx;   // The index of the last element that was added to the buffer
    int Capacity;                // The buffer size (maximum number of elements)
    int Count;                      // Number of elements in the filter
    T* Elements;

public:
    CircularBuffer(int capacity = DefaultFilterCapacity) :
            LastIdx(-1), Capacity(capacity), Count(0) {
        Elements = (T*) malloc(capacity * sizeof(T));
        for (int i = 0; i < Capacity; i++)
            Elements[i] = T();
    }

    ~CircularBuffer() {
        free(Elements);
    }

    CircularBuffer(const CircularBuffer& other) = delete;
    CircularBuffer& operator=(const CircularBuffer& other) = delete;

public:
    // Add a new element to the filter
    T addElement(const T &e) {
        LastIdx = (LastIdx + 1) % Capacity;
        T previous = Elements[LastIdx];
        Elements[LastIdx] = e;
        if (Count < Capacity) {
            Count++;
        }
        return previous;
    }

    // Get element i.  0 is the most recent, 1 is one step ago, 2 is two steps ago, ...
    T getPrev(int i = 0) const {
        if (i >= Count) // return 0 if the filter doesn't have enough elements
            return T();
        int idx = (LastIdx - i);
        if (idx < 0) // Fix the wraparound case
            idx += Capacity;
        return Elements[idx];
    }
};

// A base class for filters that maintains a buffer of sensor data taken over time and implements
// various simple filters, most of which are linear functions of the data history.
// Maintains the running sum of its elements for better performance on large capacity values
template<typename T> class SensorFilter: private CircularBuffer<T> {
private:
    T RunningTotal;               // Cached sum of the elements

public:
    SensorFilter(int capacity) : CircularBuffer<T>(capacity), RunningTotal() {
    }

    // Add a new element to the filter
    // Updates the running sum value
    void push(const T &e) {
        T previous = this->addElement(e);
        if (this->Count == this->Capacity) {
            RunningTotal -= previous;
        }
        RunningTotal += e;

        if (this->LastIdx == 0) {
            // update the cached total to avoid error accumulation
            RunningTotal = T();
            for (int i = 0; i < this->Count; i++) {
                RunningTotal += this->Elements[i];
            }
        }
    }

    T peekBack() const {
        return this->getPrev(0);
    }

    // Simple statistics
    T total() const {
        return RunningTotal;
    }

    T mean() const {
        return (this->Count == 0) ? T() : (total() / (float) this->Count);
    }

    int size() const {
        return this->Count;
    }

    void clear() {
        RunningTotal = T();
        this->Count = 0;
        this->LastIdx = -1;
    }

    SensorFilter() = delete;
};

}

#endif

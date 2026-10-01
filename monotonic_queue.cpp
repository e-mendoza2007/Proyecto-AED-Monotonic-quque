#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

template <typename T>
struct Deque {
    std::vector<T> buf;
    int front = 0, cnt = 0;

    Deque(int cap = 1) : buf(cap) {}

    bool empty() { return cnt == 0; }
    int size() { return cnt; }

    void grow() {
        std::vector<T> new_(std::max(1, (int)buf.size() * 2));
        for (int i = 0; i < cnt; i++) {
            new_[i] = buf[(front + i) % buf.size()];
        }
        buf = new_;
        front = 0;
    }

    void push_back(T x) {
        if (cnt == (int)buf.size()) grow();
        int pos = (front + cnt) % buf.size();
        buf[pos] = x;
        cnt++;
    }

    void push_front(T x) {
        if (cnt == (int)buf.size()) grow();
        front = (front - 1 + (int)buf.size()) % buf.size();
        buf[front] = x;
        cnt++;
    }

    void pop_front() {
        if (empty()) return;
        front = (front + 1) % buf.size();
        cnt--;
    }

    void pop_back() {
        if (empty()) return;
        cnt--;
    }

    T front_value() {
        if (empty()) return T();
        return buf[front];
    }

    T back_value() {
        if (empty()) return T();
        int pos = (front + cnt - 1) % buf.size();
        return buf[pos];
    }

    std::vector<T> toVector() {
        std::vector<T> out;
        out.reserve(cnt);
        for (int i = 0; i < cnt; i++) {
            out.push_back(buf[(front + i) % buf.size()]);
        }
        return out;
    }
};

struct Step {
    string op;
    int index = -1;
    int value = 0;
    vector<int> qIndices;
    int windowStart = -1;
    int windowEnd = -1;
    bool hasMax = false;
    int maxValue = 0;
};

class StepRecorder {
public:
    void add(const Step& s) { steps.push_back(s); }

    void dumpToFile(const string& path, const vector<int>& original, int k) const {
        ofstream out(path);
        out << "{\n";
        out << "  \"estructura\": \"Monotonic Queue\",\n";
        out << "  \"ejemplo\": \"sliding_window_maximum\",\n";
        out << "  \"k\": " << k << ",\n";
        out << "  \"arreglo\": [";
        for (size_t i = 0; i < original.size(); ++i) {
            out << original[i];
            if (i + 1 < original.size()) out << ", ";
        }
        out << "],\n";
        out << "  \"pasos\": [\n";
        for (size_t i = 0; i < steps.size(); ++i) {
            const Step& s = steps[i];
            out << "    {\n";
            out << "      \"op\": \"" << s.op << "\",\n";
            out << "      \"index\": " << s.index << ",\n";
            out << "      \"value\": " << s.value << ",\n";
            out << "      \"cola_indices\": [";
            for (size_t j = 0; j < s.qIndices.size(); ++j) {
                out << s.qIndices[j];
                if (j + 1 < s.qIndices.size()) out << ", ";
            }
            out << "],\n";
            out << "      \"ventana_inicio\": " << s.windowStart << ",\n";
            out << "      \"ventana_fin\": " << s.windowEnd << ",\n";
            out << "      \"max_valido\": " << (s.hasMax ? "true" : "false") << ",\n";
            out << "      \"max_valor\": " << s.maxValue << "\n";
            out << "    }" << (i + 1 < steps.size() ? "," : "") << "\n";
        }
        out << "  ]\n";
        out << "}\n";
    }

private:
    vector<Step> steps;
};

class MonotonicQueue {
public:
    MonotonicQueue(const vector<int>& values, StepRecorder& rec)
        : arr(values), recorder(rec) {}

    void push(int i, int windowStart, int windowEnd) {
        while (!dq.empty() && arr[dq.back_value()] <= arr[i]) {
            Step s;
            s.op = "pop_back";
            s.index = dq.back_value();
            s.value = arr[dq.back_value()];
            dq.pop_back();
            s.qIndices = dq.toVector();
            s.windowStart = windowStart;
            s.windowEnd = windowEnd;
            recorder.add(s);
        }
        dq.push_back(i);
        Step s;
        s.op = "push_back";
        s.index = i;
        s.value = arr[i];
        s.qIndices = dq.toVector();
        s.windowStart = windowStart;
        s.windowEnd = windowEnd;
        recorder.add(s);
    }

    void expireFront(int windowStart, int windowEnd) {
        while (!dq.empty() && dq.front_value() < windowStart) {
            Step s;
            s.op = "pop_front";
            s.index = dq.front_value();
            s.value = arr[dq.front_value()];
            dq.pop_front();
            s.qIndices = dq.toVector();
            s.windowStart = windowStart;
            s.windowEnd = windowEnd;
            recorder.add(s);
        }
    }

    bool empty() { return dq.empty(); }

    int frontValue() { return arr[dq.front_value()]; }

private:
    const vector<int>& arr;
    Deque<int> dq;
    StepRecorder& recorder;
};

vector<int> slidingWindowMaximum(const vector<int>& nums, int k, StepRecorder& rec) {
    vector<int> result;
    MonotonicQueue mq(nums, rec);
    int n = (int)nums.size();

    for (int r = 0; r < n; ++r) {
        int windowStart = r - k + 1;
        mq.push(r, max(0, windowStart), r);
        mq.expireFront(max(0, windowStart), r);

        if (windowStart >= 0) {
            int m = mq.frontValue();
            result.push_back(m);

            Step s;
            s.op = "record_max";
            s.index = r;
            s.value = nums[r];
            s.windowStart = windowStart;
            s.windowEnd = r;
            s.hasMax = true;
            s.maxValue = m;
            rec.add(s);
        }
    }
    return result;
}

void runCase(const string& name, const vector<int>& nums, int k) {
    cout << "=== Caso: " << name << " (k=" << k << ") ===\n";
    cout << "Arreglo: [";
    for (size_t i = 0; i < nums.size(); ++i) cout << nums[i] << (i + 1 < nums.size() ? ", " : "");
    cout << "]\n";

    StepRecorder rec;
    vector<int> result = slidingWindowMaximum(nums, k, rec);

    cout << "Maximos por ventana: [";
    for (size_t i = 0; i < result.size(); ++i) cout << result[i] << (i + 1 < result.size() ? ", " : "");
    cout << "]\n\n";

    string filename = "steps_" + name + ".json";
    rec.dumpToFile(filename, nums, k);
    cout << "Registro de pasos guardado en: " << filename << "\n\n";
}

int main() {
    runCase("general", {1, 3, -1, -3, 5, 3, 6, 7}, 3);
    runCase("k_igual_1", {4, 2, 9, 1}, 1);
    runCase("un_elemento", {42}, 1);
    runCase("repetidos_decrecientes", {5, 5, 5, 4, 3, 2}, 2);
    return 0;
}
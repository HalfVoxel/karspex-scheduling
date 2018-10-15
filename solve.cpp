#include<vector>
#include<iostream>
#include<set>
#include <random>
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace std;

struct Constraint {
	int time;
	int group;
	vector<int> people;

	Constraint (int time, int group, vector<int> people) : time(time), group(group), people(people) {}
};

struct Input {
	map<string,int> groupIndices;
	vector<string> groupNames;
	vector<string> names;
	vector<string> timeNames;
	vector<int> timeSizes;
	vector<vector<int>> partOfGroup;
	vector<Constraint> atLeastOneConstraints;
	vector<Constraint> noneFromConstraints;
	vector<int> personDistinguishability;
	vector<float> timeHardness;
	vector<bool> timeIsHard;
	vector<pair<int,int>> pairConstraints;

	int addGroup(string name) {
		if (groupIndices.find(name) == groupIndices.end()) {
			groupIndices[name] = groupNames.size();
			groupNames.push_back(name);
		}
		return groupIndices[name];
	}

	vector<int> getGroupMembers(int groupIndex) {
		vector<int> result;
		for (int i = 0; i < partOfGroup.size(); i++) {
			for (int c : partOfGroup[i]) {
				if (c == groupIndex) {
					result.push_back(i);
					break;
				}
			}
		}
		return result;
	}

	void parse() {
		
		int stage = 0;
		string s;
		int ln = 0;
		while(getline(cin, s)) {
			ln++;
			if (s == "") {
				if (stage >= 2) {
					cerr << "Unexpected empty line in input. At line " << ln << endl;
					exit(1);
				}
				stage++;
				continue;
			}

			if (stage == 0) {
				vector<int> currentGroupIndices;

				vector<string> splits;
				boost::split(splits, s, boost::is_any_of("\t"));

				if (splits.size() != 4) {
					cerr << "Expected exactly 4 columns (separated by tabs) on line " << ln << endl;
					exit(1);
				}

				boost::trim(splits[2]);
				boost::trim(splits[3]);
				names.push_back(splits[2] + " " + splits[3]);

				// auto splits = split(s, '\t');
				vector<string> groups1;
				boost::split(groups1, splits[0], boost::is_any_of(","));
				vector<string> groups2;
				boost::split(groups2, splits[1], boost::is_any_of(","));
				for (auto groups : {groups1, groups2}) {
					for (auto g : groups) {
						boost::trim(g);
						if (g.size() == 0) continue;

						int groupIndex = addGroup(g);
						currentGroupIndices.push_back(groupIndex);
						// cout << names[names.size()-1] << " belongs to " << groupIndex << "( " << groupNames[groupIndex] << endl;
					}
				}
				partOfGroup.push_back(currentGroupIndices);
			} else if (stage == 1) {
				// Constraints
				vector<string> splits;
				boost::split(splits, s, boost::is_any_of("\t"));

				if (splits.size() != 3) {
					cerr << "Expected exactly 3 columns (separated by tabs) on line " << ln << endl;
					exit(1);
				}

				float hardness = 1;
				bool hard = false;
				string eventName = splits[0];
				stringstream ss1(splits[1]);
				int timeSize;
				ss1 >> timeSize;
				timeSizes.push_back(timeSize);

				int timeIndex = timeNames.size();
				timeNames.push_back(eventName);
				stringstream ss(splits[2]);
				string token;
				while(ss >> token) {
					if (token == "hard") {
						hardness += 0.4f;
						hard = true;
						continue;
					}

					auto groupName = token.substr(1, token.size()-1);
					if (groupIndices.find(groupName) == groupIndices.end()) {
						cerr << "Could not find any group named " << groupName << ", referenced on line " << ln << endl;
						exit(1);
					}
					int groupIndex = groupIndices[groupName];
					if(token[0] == '>') {
						// At least one
						atLeastOneConstraints.push_back(Constraint(timeIndex, groupIndex, getGroupMembers(groupIndex)));
					} else if (token[0] == '!') {
						// None from
						noneFromConstraints.push_back(Constraint(timeIndex, groupIndex, getGroupMembers(groupIndex)));
					} else {
						cerr << "Unexpected constraint signifier " << token[0] << " for group " << groupName << " on line " << ln << endl;
						exit(1);
					}
				}

				timeHardness.push_back(hardness);
				timeIsHard.push_back(hard);
			} else {
				// Stage == 2
				// Pair constraints
				vector<string> splits;
				boost::split(splits, s, boost::is_any_of("\t"));
				if (splits.size() != 2) {
					cerr << "Expected exactly 2 columns (separated by tabs) on line " << ln << endl;
					exit(1);
				}

				auto timeIndex1 = find(timeNames.begin(), timeNames.end(), splits[0]);
				auto timeIndex2 = find(timeNames.begin(), timeNames.end(), splits[1]);
				if (timeIndex1 == timeNames.end()) {
					cerr << "No time with the name " << splits[0] << " at line " << ln << endl;
					exit(1);
				}
				if (timeIndex2 == timeNames.end()) {
					cerr << "No time with the name " << splits[1] << " at line " << ln << endl;
					exit(1);
				}
				pairConstraints.push_back(make_pair(timeIndex1 - timeNames.begin(), timeIndex2 - timeNames.begin()));
			}
		}

		personDistinguishability = vector<int>(names.size());
		int ci = 0;
		for (auto& constraintList : { atLeastOneConstraints, noneFromConstraints }) {
			for (auto& constraint : constraintList) {
				ci++;
				int hash = (constraint.time * 534124) ^ (constraint.group * 95347) ^ (ci * 3434);
				// cout << "C: " << timeNames[constraint.time] << " " << groupNames[constraint.group] << " " << hash << " " << constraint.people.size() << endl;
				for (int p : constraint.people) {
					personDistinguishability[p] = (personDistinguishability[p] * 8423) ^ hash;
				}
			}
		}

		// for (float c : timeHardness) cout << c << " " << endl;
		// for (int c : personDistinguishability) cout << c << " " << endl;
	}
};

struct State {
	vector<vector<bool>> times;
	float score = 0;
	vector<int> interestingPeople;

	void atleastone(int time, vector<int> peopleRange) {
		for (int i : peopleRange) {
			if (times[i][time]) return;
		}
		for (int i : peopleRange) {
			interestingPeople.push_back(i);
		}
		score += 1000;
	}
	void nonefrom (int time, vector<int> peopleRange) {
		for (int i : peopleRange) {
			if (times[i][time]) {
				score += 1000;
				interestingPeople.push_back(i);
				return;
			}
		}
	}

	void addHardCost(const Input& input) {
		for (int i = 0; i < times.size(); i++) {
			int hard = 0;
			for (int j = 0; j < times[i].size(); j++) if (times[i][j] && input.timeIsHard[j]) hard++;
			if (hard > 1) {
				score += 100*hard*hard;
			}
		}
	}

	void addWorkDistributionCost(const Input& input) {
		float highest = 0;
		float lowest = 1000;
		for (int i = 0; i < times.size(); i++) {
			float c = 0;
			for (int j = 0; j < times[i].size(); j++) if (times[i][j]) c += input.timeHardness[j];
			score += c*c;
			highest = max(highest, c);
			lowest = min(lowest, c);
		}
		for (int i = 0; i < times.size(); i++) {
			float c = 0;
			for (int j = 0; j < times[i].size(); j++) if (times[i][j]) c += input.timeHardness[j];
			if (highest - c < 0.5f) {
				interestingPeople.push_back(i);
			}
			// if (c - lowest < 0.2f) {
			// 	interestingPeople.push_back(i);
			// }
		}
	}

	void addSizeCost() {
		for (int i = 0; i < times[0].size(); i++) {
			int c = 0;
			for (int j = 0; j < times.size(); j++) c += times[j][i];
			if (c < 10) score += 200*(10 - c);
			if (c > 12) score += 200*(c - 12);
			if (c > 12) {
				for (int j = 0; j < times.size(); j++) {
					if (times[j][i]) {
						interestingPeople.push_back(j);
					}
				}
			}
		}
	}

	int hash() {
		int h = 423;
		for (const auto v : times) {
			for (const auto b : v) {
				h *= 75153437;
				h ^= (int)b;
			}
		}
		return h;
	}
};

void printState (const Input& input, const State& state) {
	int npeople = state.times.size();
	int ntimes = state.times[0].size();

	cout << "By person" << endl;
	for (int i = 0; i < npeople; i++) {
		cout << input.names[i] << "\t";
		for (int j = 0; j < ntimes; j++) {
			if (state.times[i][j]) {
				cout << input.timeNames[j] << "\t";
			}
		}
		cout << endl;
	}

	cout << "By time" << endl;
	for (int j = 0; j < ntimes; j++) {
		cout << input.timeNames[j] << "\t";
		for (int i = 0; i < npeople; i++) {
			if (state.times[i][j]) {
				cout << input.names[i];
				// cout << " (";
				// for (auto g : input.partOfGroup[i]) {
				// 	cout << input.groupNames[g] << ", ";
				// }
				// cout << ")" << "\t";
				cout << "\t";
			}
		}
		cout << endl;
	}
}

State toggleManyRandom(const State& origState) {
	int npeople = origState.times.size();
	int ntimes = origState.times[0].size();

	State state = origState;
	state.score = 0;

	int n = (rand() % 4) + 1;
	for (int i = 0; i < n; i++) {
		int r1 = rand() % npeople;
		int g1 = rand() % ntimes;
		if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
			r1 = state.interestingPeople[(rand() % state.interestingPeople.size())];
		}
		state.times[r1][g1] = !state.times[r1][g1];
	}
	return state;
}

State toggleRandom(const State& origState) {
	int npeople = origState.times.size();
	int ntimes = origState.times[0].size();

	State state = origState;
	state.score = 0;

	int r1 = rand() % npeople;
	if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
		r1 = state.interestingPeople[(rand() % state.interestingPeople.size())];
	}
	int g1 = rand() % ntimes;
	state.times[r1][g1] = !state.times[r1][g1];
	return state;
}

State swapRandom(const State& origState) {
	int npeople = origState.times.size();
	int ntimes = origState.times[0].size();

	State state = origState;
	state.score = 0;

	int r1 = rand() % npeople;
	int r2 = rand() % npeople;
	if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
		r1 = state.interestingPeople[(rand() % state.interestingPeople.size())];
	}
	if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
		r2 = state.interestingPeople[(rand() % state.interestingPeople.size())];
	}
	int g1 = rand() % ntimes;
	int g2 = rand() % ntimes;

	for (int i = 0; i < ntimes && !state.times[r1][g1]; i++) g1 = (g1 + 1) % ntimes;
	for (int i = 0; i < ntimes && !state.times[r2][g2]; i++) g2 = (g2 + 1) % ntimes;

	if (g1 == g2) g1 = (g1 + 1) % ntimes;

	state.times[r1][g1] = false;
	state.times[r2][g2] = false;

	state.times[r1][g2] = true;
	state.times[r2][g1] = true;
	return state;
}

State moveRandom(const State& origState) {
	int npeople = origState.times.size();
	int ntimes = origState.times[0].size();

	State state = origState;
	state.score = 0;

	int r1 = rand() % npeople;
	int r2 = rand() % npeople;
	if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
		r1 = state.interestingPeople[(rand() % state.interestingPeople.size())];
	}
	if ((rand() % 1) == 0 && state.interestingPeople.size() > 0) {
		r2 = state.interestingPeople[(rand() % state.interestingPeople.size())];
	}
	int g1 = rand() % ntimes;

	for (int i = 0; i < ntimes && !state.times[r1][g1]; i++) g1 = (g1 + 1) % ntimes;
	for (int i = 0; i < npeople && state.times[r2][g1]; i++) r2 = (r2 + 1) % npeople;

	state.times[r1][g1] = false;
	state.times[r2][g1] = true;
	return state;
}

State distributePeople (const State& origState, const Input& input) {
	int npeople = origState.times.size();
	int ntimes = origState.times[0].size();

	State state = origState;
	state.score = 0;
	
	for (int p1 = 0; p1 < npeople; p1++) {
		int c1 = 0;
		for (int j = 0; j < ntimes; j++) if (state.times[p1][j]) c1 += 1; //input.timeHardness[j];

		for (int p2 = 0; p2 < npeople; p2++) {
			if (p1 == p2) continue;
			if (input.personDistinguishability[p1] != input.personDistinguishability[p2]) continue;

			int c2 = 0;
			for (int j = 0; j < ntimes; j++) if (state.times[p2][j]) c2 += 1; //input.timeHardness[j];

			if (abs(c1 - c2) > 1) {
				int fromPerson = p1;
				int toPerson = p2;
				if (c2 > c1) swap(fromPerson, toPerson);

				// Move a group from fromPerson to toPerson
				for (int g = 0; g < ntimes; g++) {
					if (input.timeIsHard[g]) continue;
					if (state.times[fromPerson][g] && !state.times[toPerson][g]) {
						swap(state.times[fromPerson][g], state.times[toPerson][g]);
						break;
					}
				}
				break;
			}
		}
	}

	return state;
}

void checkConstraints(const Input& input, State& state) {
	for (auto constraint : input.atLeastOneConstraints) {
		state.atleastone(constraint.time, constraint.people);
	}
	for (auto constraint : input.noneFromConstraints) {
		state.nonefrom(constraint.time, constraint.people);
	}
}

template<class T>
bool contains (const vector<T>& arr, const T& v) {
	return find(arr.begin(), arr.end(), v) != arr.end();
}

void greedy (const Input& input) {
	srand(time(NULL));

	int npeople = input.names.size();
	int ntimes = input.timeNames.size();

	State s;
	s.times = vector<vector<bool>>(npeople, vector<bool>(ntimes));

	while(true) {
		// Find person with lowest work burden
		int smallestIndex = -1;
		float smallestBurden = 100000;
		int k = 0;
		for (int i = 0; i < npeople; i++) {
			float c = 0;
			for (int j = 0; j < ntimes; j++) if (s.times[i][j]) c += input.timeHardness[j];
			// Note: uses reservoir sampling
			k++;
			if (c < smallestBurden || (abs(c - smallestBurden) < 0.001f && (rand() % k) == 0)) {
				k = 1;
				smallestBurden = c;
				smallestIndex = i;
			}
		}

		if (smallestIndex == -1) break;

		// Find the group that wants this person the most
		vector<float> timeWeights (ntimes, 0);

		for (auto constraint : input.atLeastOneConstraints) {
			if (contains(constraint.people, smallestIndex)) {
				// Check if satisfied already
				bool satisfied = false;
				for (int p : constraint.people) {
					satisfied |= s.times[p][constraint.time];
				}

				if (!satisfied) {
					timeWeights[constraint.time] += 10;
				}
			}
		}

		for (int j = 0; j < ntimes; j++) {
			int c = 0;
			for (int i = 0; i < npeople; i++) c += s.times[i][j];

			if (c < input.timeSizes[j]) {
				timeWeights[j] += 1.0f / (c+1);
			}

			// Can't add a person to a time more than once
			if (s.times[smallestIndex][j]) {
				timeWeights[j] = -10000;
			}
		}

		for (auto constraint : input.noneFromConstraints) {
			if (contains(constraint.people, smallestIndex)) {
				timeWeights[constraint.time] = -10000;
			}
		}

		for (auto pair : input.pairConstraints) {
			if (s.times[smallestIndex][pair.first]) timeWeights[pair.second] = -10000;
			if (s.times[smallestIndex][pair.second]) timeWeights[pair.first] = -10000;
		}

		int bestTime = -1;
		float bestWeight = 0;
		for (int j = 0; j < ntimes; j++) {
			if (timeWeights[j] > bestWeight) {
				bestWeight = timeWeights[j];
				bestTime = j;
			}
		}

		if (bestTime != -1) {
			assert(!s.times[smallestIndex][bestTime]);
			s.times[smallestIndex][bestTime] = true;
		} else {
			// printState(input, s);
			// exit(1);
			bool allSatisfied = true;
			// Check all constraints
			for (auto constraint : input.atLeastOneConstraints) {
				// Check if satisfied already
				bool satisfied = false;
				for (int p : constraint.people) {
					satisfied |= s.times[p][constraint.time];
				}
				allSatisfied &= satisfied;
			}

			for (int j = 0; j < ntimes; j++) {
				int c = 0;
				for (int i = 0; i < npeople; i++) c += s.times[i][j];
				if (c < input.timeSizes[j]) allSatisfied = false;
			}

			if (allSatisfied) break;
		}
	}

	printState(input, s);

	{
		bool allSatisfied = true;
		// Check all constraints
		for (auto constraint : input.atLeastOneConstraints) {
			// Check if satisfied already
			bool satisfied = false;
			for (int p : constraint.people) {
				satisfied |= s.times[p][constraint.time];
			}
			allSatisfied &= satisfied;
		}

		for (auto constraint : input.noneFromConstraints) {
			for (auto p : constraint.people) {
				if (s.times[p][constraint.time]) {
					allSatisfied = false;
				}
			}
		}

		for (int p = 0; p < npeople; p++) {
			for (auto pair : input.pairConstraints) {
				if (s.times[p][pair.first] && s.times[p][pair.second]) allSatisfied = false;
			}
		}

		cout << "All constraints satisfied? " << (allSatisfied ? "true" : "false") << endl;
	}

	checkConstraints(input, s);
	s.addWorkDistributionCost(input);
	s.addSizeCost();
	// s.addHardCost(input);
	cout << "Score " << s.score << endl;
}

int main () {
	Input input;
	input.parse();

	int npeople = input.names.size();
	int ntimes = input.timeNames.size();
	greedy(input);
	return 0;
    auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine(time(NULL)));
    srand(time(NULL));

    float minScore = 100000;
	while(true) {
		State s;
		s.times = vector<vector<bool>>(npeople, vector<bool>(ntimes));
		for (int i = 0; i < npeople; i++) {
			for (int j = 0; j < ntimes; j++) {
				s.times[i][j] = (bool)((rand() % 1000) <= 200);
			}
		}

		checkConstraints(input, s);
		s.addWorkDistributionCost(input);
		s.addSizeCost();
		s.addHardCost(input);

		int fails = 0;
		int i;
		for (i = 0; i < 1000000 && fails < 1000; i++) {
			State newState;
			bool dist = false;
			if ((rand() % 10) == 0) {
				dist = true;
				newState = distributePeople(s, input);
			} else if (gen()) {
				if (gen()) {
					newState = swapRandom(s);
				} else {
					newState = moveRandom(s);
				}
			} else {
				if (gen()) {
					newState = toggleRandom(s);
				} else {
					newState = toggleManyRandom(s);
				}
			}

			newState.score = 0;
			newState.interestingPeople.clear();
			checkConstraints(input, newState);
			newState.addWorkDistributionCost(input);
			newState.addSizeCost();
			newState.addHardCost(input);

			if (newState.score < s.score || ((rand() % 100) == 0)) {
				//cout << "Improved " << s.score << " -> " << newState.score << endl;
				if (newState.score < s.score) {
					fails = 0;
				}
				s = newState;
			} else {
				fails++;
			}
		}

		cout << "Gave up after " << i << endl;
		if (s.score < minScore) {
			minScore = s.score;
			printState(input, s);
			cout << "Score: " << minScore << endl;
		}
	}
}
/*
 * hsLmCut.cpp
 *
 *  Created on: 13.12.2018
 *      Author: Daniel Höller
 */

#include "hsLmCut.h"
#include <cstring>
#include <deque>

namespace progression {

hsLmCut::hsLmCut(Model* sas) {
	// init hMax-Heuristic
	assert(!sas->isHtnModel);
	m = sas;
	hValInit = new int[m->numStateBits];
	hValReset = new int[m->numStateBits];
	for (int i = 0; i < m->numStateBits; i++) {
		hValInit[i] = UNREACHABLE;
		hValReset[i] = UNREACHABLE;
		
	}
	heap = new IntPairHeap<int>(m->numStateBits * 2);
	unsatPrecs = new int[m->numActions];
	hVal = new int[m->numStateBits];
	hasZeroAchiever = new bool[m->numStateBits];

	maxPrecInit = new int[m->numActions];
	for (int i = 0; i < m->numActions; i++)
		maxPrecInit[i] = UNREACHABLE;

	// init hLmCut
	costs = new int[m->numActions];
	goalZone = new noDelIntSet();
	goalZone->init(m->numStateBits);
	cut = new bucketSet();
	cut->init(m->numActions);
	precsOfCutNodes = new bucketSet();
	precsOfCutNodes->init(m->numStateBits);
	stack.init(m->numActions * 2);

	

	// init reverse mapping
	numAddToTask = new int[m->numStateBits];
	for (int i = 0; i < m->numStateBits; i++)
		numAddToTask[i] = 0;
	for (int i = 0; i < m->numActions; i++) {
		for (int j = 0; j < m->numAdds[i]; j++) {
			int f = m->addLists[i][j];
			numAddToTask[f]++;
		}
	}
	addToTask = new int*[m->numStateBits];
	int *temp = new int[m->numStateBits];
	for (int i = 0; i < m->numStateBits; i++) {
		addToTask[i] = new int[numAddToTask[i]];
		temp[i] = 0;
	}
	for (int iOp = 0; iOp < m->numActions; iOp++) {
		for (int iF = 0; iF < m->numAdds[iOp]; iF++) {
			int f = m->addLists[iOp][iF];
			addToTask[f][temp[f]] = iOp;
			temp[f]++;
			if (temp[f] > 1) { // otherwise, an action has the same effect twice
				assert(addToTask[f][temp[f] - 2] < addToTask[f][temp[f] - 1]);
			}
		}
	}
	delete[] temp;
	remove = new noDelIntSet();
	remove->init(m->numStateBits);
	maxPrec = new int[m->numActions];
	visited = new noDelIntSet();
	visited->init(m->numStateBits);
}

hsLmCut::~hsLmCut() {
	delete[] hValReset;
	delete[] hasZeroAchiever;
	delete[] hValInit;
	delete[] unsatPrecs;
	delete[] hVal;
	delete[] costs;
	delete heap;
	delete goalZone;
	delete cut;
	delete precsOfCutNodes;
	delete[] maxPrecInit;
	delete[] numAddToTask;
	for (int i = 0; i < m->numStateBits; i++)
		delete[] addToTask[i];
	delete[] addToTask;
	delete remove;
	delete[] maxPrec;
	delete visited;

}

int hsLmCut::getHeuristicValue(bucketSet& s, noDelIntSet& g) {
	int hLmCut = 0;

	//std::cout << "PCF TYPE: " << this->pcfType << std::endl;
	// clean up stored cuts

	if(storeCuts) {
		for(LMCutLandmark* cut : *cuts) {
			delete cut;
		}
		cuts->clear();
	}

	memcpy(costs, m->actionCosts, sizeof(int) * m->numActions);

	int hMax = getHMax(s, g, goalZone);
	if ((hMax == 0) || (hMax == UNREACHABLE))
		return hMax;
	memcpy(hValInit, hVal, sizeof(int) * m->numStateBits);
	//cout << endl << "start" << endl;

	for (int i = 0; i < m->numStateBits; i++) {
		hasZeroAchiever[i] = false;
	}

	while (hMax > 0) {
		goalZone->clear();
		cut->clear();
		precsOfCutNodes->clear();

		calcGoalZone(goalZone, cut, precsOfCutNodes);
		assert(cut->getSize() > 0);

		// check forward-reachability
		forwardReachabilityDFS(s, cut, goalZone, precsOfCutNodes);
		assert(cut->getSize());

		// calculate costs
		int minCosts = INT_MAX;

		LMCutLandmark* currendCut = nullptr;
		int ci = 0;
		if (storeCuts) {
            currendCut = new LMCutLandmark(cut->getSize());
			cuts->push_back(currendCut);
		}
		for (int cutted = cut->getFirst(); cutted >= 0; cutted =
				cut->getNext()) {
			if (minCosts > costs[cutted])
				minCosts = costs[cutted];
			if (storeCuts)
                currendCut->lm[ci++] = cutted;
		}
		assert(minCosts > 0);
		hLmCut += minCosts;

		// update costs
		//cout << "cut" << endl;
		for (int op = cut->getFirst(); op >= 0; op = cut->getNext()) {
			//cout << "- [" << op << "] " << m->taskNames[op] << endl;
			costs[op] -= minCosts;
			assert(costs[op] >= 0);
			//assert(allPrecsTrue(op));

			//TODO: update has zero-cost achievers array
			for (int iF = 0; iF < m->numAdds[op]; iF++) {
				int f = m->addLists[op][iF];
				hasZeroAchiever[f] = true;
			}
		}
#ifdef LMCINCHMAX
		hMax = updateHMax(g, cut);
#else
		//hMax = getHMax(s, g, goalZone);
		hMax = updateHMax(g, cut);
#endif
		
	}
	//exit(0);
	//cout << "final lmc " << hLmCut << endl;
	return hLmCut;
}

void hsLmCut::calcGoalZone(noDelIntSet* goalZone, bucketSet* cut,
		bucketSet* precsOfCutNodes) {
	stack.clear();
	stack.push(maxPrecG);
	while (!stack.isEmpty()) {
		int fact = stack.pop();

		for (int i = 0; i < numAddToTask[fact]; i++) {
			int producer = addToTask[fact][i];

			if (unsatPrecs[producer] > 0) // not reachable
				continue;

			int singlePrec = maxPrec[producer];
			if (goalZone->get(singlePrec))
				continue;

			if (costs[producer] == 0) {
				goalZone->insert(singlePrec);
				precsOfCutNodes->erase(singlePrec);
				stack.push(singlePrec);
			} else {
				cut->insert(producer);
				precsOfCutNodes->insert(singlePrec);
			}
		}
	}
}

void hsLmCut::forwardReachabilityDFS(bucketSet& s0, bucketSet* cut,
		noDelIntSet* goalZone, bucketSet* testReachability) {
	stack.clear();
	remove->clear();

	for (int f = testReachability->getFirst(); f >= 0;
			f = testReachability->getNext()) {
		if (s0.get(f))
			continue;
		visited->clear();
		bool reachedS0 = false;
		stack.clear();
		stack.push(f);
		visited->insert(f);
		while (!stack.isEmpty()) { // reachabilityLoop
			int pred = stack.pop();
			for (int i = 0; i < numAddToTask[pred]; i++) {
				int op = addToTask[pred][i];
				if (unsatPrecs[op] > 0)
					continue;
				if (goalZone->get(maxPrec[op]))
					continue;
				if ((m->numPrecs[op] == 0) || (s0.get(maxPrec[op]))) { // reached s0
					reachedS0 = true;
					break;
				} else if (!visited->get(maxPrec[op])) {
					visited->insert(maxPrec[op]);
					stack.push(maxPrec[op]);
				}
			}
			if (reachedS0)
				break;
		}
		if (!reachedS0)
			remove->insert(f);
	}
	for (int op = cut->getFirst(); op >= 0; op = cut->getNext()) {
		if (remove->get(maxPrec[op]))
			cut->erase(op);
	}
}

int hsLmCut::decidePcf(noDelIntSet* goalZone, int newProp, int maxProp){
	if(this->pcfType == PCFType::NONE) return maxProp;
	else if(this->pcfType == PCFType::GZDpBD) return GZDpBD(goalZone, newProp, maxProp);
	else if(this->pcfType == PCFType::GZD) return GZD(goalZone, newProp, maxProp);
	else if(this->pcfType == PCFType::BD) return BD(goalZone, newProp, maxProp);
	else if(this->pcfType == PCFType::VDM) return VDM(goalZone, newProp, maxProp);
}

int hsLmCut::GZD(noDelIntSet* goalZone, int newProp, int maxProp){
	// Goal Zone Detection: prefer pcfs in goal zone
	bool newInGoal = goalZone->get(newProp);
    bool maxInGoal = goalZone->get(maxProp);
	if (newInGoal && !maxInGoal)
		return newProp;
	return maxProp;
}

int hsLmCut::BD(noDelIntSet* goalZone, int newProp, int maxProp){
	//Border Detection (BD): Prefer nodes with non zero-cost achievers
	bool newNoZeroAchiever = !hasZeroAchiever[newProp];
    bool maxNoZeroAchiever = !hasZeroAchiever[maxProp];
    if (newNoZeroAchiever && !maxNoZeroAchiever)
        return newProp;
    return maxProp;
}

int hsLmCut::VDM(noDelIntSet* goalZone, int newProp, int maxProp) {
    // Compute the decrease in hmax for both candidate preconditions.
    // A smaller decrease means the candidate’s current hmax is closer to its initial value,
    // which in turn suggests fewer zero-cost actions between the initial state and this fact.
    int diffNew = hValInit[newProp] - hVal[newProp];
    int diffMax = hValInit[maxProp] - hVal[maxProp];
	// Prefer the candidate with the smaller difference.
    if (diffNew < diffMax)
        return newProp;
    else
        return maxProp;
		//return GZDpBD(goalZone, newProp, maxProp);
}

int hsLmCut::GZDpBD(noDelIntSet* goalZone, int newProp, int maxProp){
	// Goal Zone Detection: prefer pcfs in goal zone
	bool newInGoal = goalZone->get(newProp);
    bool maxInGoal = goalZone->get(maxProp);
	if (newInGoal && !maxInGoal)
		return newProp;
	if (maxInGoal && !newInGoal)
		return maxProp;

	//Border Detection (BD): Prefer nodes with non zero-cost achievers
	bool newNoZeroAchiever = !hasZeroAchiever[newProp];
    bool maxNoZeroAchiever = !hasZeroAchiever[maxProp];
    if (newNoZeroAchiever && !maxNoZeroAchiever)
        return newProp;
    return maxProp;
}

int hsLmCut::getHMax(bucketSet& s, noDelIntSet& g, noDelIntSet* goalZone) {
	if (g.getSize() == 0)
		return 0;
	memcpy(unsatPrecs, m->numPrecs, sizeof(int) * m->numActions);
	memcpy(maxPrec, maxPrecInit, sizeof(int) * m->numActions);
	memcpy(hVal, hValReset, sizeof(int) * m->numStateBits);
	maxPrecG = -1;
	heap->clear();
	for (int f = s.getFirst(); f >= 0; f = s.getNext()) {
		heap->add(0, f);
		hVal[f] = 0;
	}

	for (int iOp = 0; iOp < m->numPrecLessActions; iOp++) {
		int op = m->precLessActions[iOp];
		for (int iAdd = 0; iAdd < m->numAdds[op]; iAdd++) {
			int f = m->addLists[op][iAdd];
			hVal[f] = m->actionCosts[op];
			heap->add(hVal[f], f);
		}
	}
	while (!heap->isEmpty()) {
		int pVal = heap->topKey();
		int prop = heap->topVal();
		heap->pop();
		if (hVal[prop] < pVal)
			continue;
		for (int iOp = 0; iOp < m->precToActionSize[prop]; iOp++) {
			int op = m->precToAction[prop][iOp];
			if ((maxPrec[op] == UNREACHABLE)
					|| (hVal[maxPrec[op]] < hVal[prop])) {
				maxPrec[op] = prop;
			}
			else if(hVal[maxPrec[op]] == hVal[prop]){
				maxPrec[op] = decidePcf(goalZone, prop, maxPrec[op]);
			}
			if (--unsatPrecs[op] == 0) {
				for (int iF = 0; iF < m->numAdds[op]; iF++) {
					int f = m->addLists[op][iF];
					if ((hVal[f] == UNREACHABLE)
							|| ((hVal[maxPrec[op]] + costs[op]) < hVal[f])) {
						hVal[f] = hVal[maxPrec[op]] + costs[op];
						heap->add(hVal[f], f);
					}
				}
			}
		}
	}

	int res = INT_MIN;
	
	for (int f = g.getFirst(); f >= 0; f = g.getNext()) {
		if (hVal[f] == UNREACHABLE) {
			res = UNREACHABLE;
			maxPrecG = -1;
			break;
		} else if (res < hVal[f]) {
			res = hVal[f];
			maxPrecG = f;			
		}
		else if (res == hVal[f]){
			maxPrecG=decidePcf(goalZone, f, maxPrecG);
		}
	}
	return res;
}

int hsLmCut::updateHMax(noDelIntSet& g, bucketSet* cut) {
    std::deque<int> updateQueue;
	// for every operator in the cut, add each add–effect to the update queue.
    for (int op = cut->getFirst(); op >= 0; op = cut->getNext()) {
		//std::cout << "CUT OPERATOR " << m->taskNames[op] << std::endl;
        for (int iF = 0; iF < m->numAdds[op]; iF++) {
            int f = m->addLists[op][iF];
            updateQueue.push_back(f);

			// if the decreased value is lower than fprime, fprime must chang value
			if(hVal[maxPrec[op]] < hVal[f]){
				hVal[f] = hVal[maxPrec[op]] ;
				updateQueue.push_back(f);
			}
		}
    }

	if(updateQueue.empty()) return 0;

	// propagate updates until no more changes occur.
    while (!updateQueue.empty()) {
        int f = updateQueue.front();
		updateQueue.pop_front();
		//std::cout << " fact " << m->factStrs[f] << " h: " << hVal[f] << std::endl;
		// for each operator op that has f as a precondition:
        for (int iOp = 0; iOp < m->precToActionSize[f]; iOp++) {
            int op = m->precToAction[f][iOp];
            // only update operators that are fully satisfied
			// also if pcf of op is not f, op won't change.
            if (unsatPrecs[op] != 0 || maxPrec[op]!=f)
                continue;

            // recompute the best precondition cost for operator op.
            //int best = hVal[f];
			//int pcf =  f;
			int initialPcf = maxPrec[op];
			for (int j = 0; j < m->numPrecs[op]; j++) {
                int p = m->precLists[op][j];
                if (hVal[p] > hVal[maxPrec[op]]) {
                    //best = hVal[p];
					maxPrec[op] = p;
                }
				else if (hVal[maxPrec[op]] == hVal[p]){
					maxPrec[op]=decidePcf(goalZone, p, maxPrec[op]);
				}
            }
			
            // if the new hmax F is different (is lower) than hmax of operator
			// if(hVal[pcf] != hVal[f]){
			// 	maxPrec[op] = pcf;
			// }
			// if the pcf remains f, the value decreased
			//if(maxPrec[op] == f){
			if (initialPcf == maxPrec[op]){
				for (int iF = 0; iF < m->numAdds[op]; iF++) {
					int fprime = m->addLists[op][iF];
					// if the decreased value is lower than fprime, fprime must chang value
					if(hVal[maxPrec[op]] + costs[op] < hVal[fprime]){
						hVal[fprime] = hVal[maxPrec[op]] + costs[op];
						updateQueue.push_back(fprime);
					}
					
				}
			//}
		}
    }

	
    // recompute the overall hmax value as the maximum cost over all goal facts.
    int res = INT_MIN;
	
    for (int f = g.getFirst(); f >= 0; f = g.getNext()) {
		if (hVal[f] == UNREACHABLE) {
            maxPrecG = -1;
		    return UNREACHABLE;
        }
        if (res < hVal[f]) {
		    res = hVal[f];
            maxPrecG = f;
        }
		// else if (res == hVal[f]){
		// 	maxPrecG=decidePcf(goalZone, f, maxPrecG);
		// }
    }
	return res;
}


// int hsLmCut::updateHMax(noDelIntSet& g, bucketSet* cut) {
// 	heap->clear();
// 	for (int op = cut->getFirst(); op >= 0; op = cut->getNext()) {
// 		for (int iF = 0; iF < m->numAdds[op]; iF++) {
// 			int f = m->addLists[op][iF];
// 			if ((hVal[maxPrec[op]] + costs[op]) < hVal[f]) { // that f might be cheaper now
// 				hVal[f] = hVal[maxPrec[op]] + costs[op];
// 				heap->add(hVal[f], f);
// 			}
// 		}
// 	}

// 	while (!heap->isEmpty()) {
// 		int pVal = heap->topKey();
// 		int prop = heap->topVal();
// 		heap->pop();
// 		if (hVal[prop] < pVal) // we have prop's costs DECREASED -> this is fine
// 			continue;
// 		for (int iOp = 0; iOp < m->precToActionSize[prop]; iOp++) {
// 			int op = m->precToAction[prop][iOp];
// 			if ((unsatPrecs[op] == 0) && (prop == maxPrec[op])) { // this may change the costs of the operator and all its successors
// 				int opMaxPrec = -1;
// 				int val = INT_MIN;
// 				for (int iF = 0; iF < m->numPrecs[op]; iF++) {
// 					int f = m->precLists[op][iF];
// 					assert(hVal[f] != UNREACHABLE);
// 					if (hVal[f] > val) {
// 						opMaxPrec = f;
// 						val = hVal[f];
// 					}
// 				}
// 				maxPrec[op] = opMaxPrec;
// 				for (int iF = 0; iF < m->numAdds[op]; iF++) {
// 					int f = m->addLists[op][iF];
// 					if ((hVal[maxPrec[op]] + costs[op]) < hVal[f]) {
// 						hVal[f] = hVal[maxPrec[op]] + costs[op];
// 						assert(hVal[f] >= 0);
// 						heap->add(hVal[f], f);
// 					}
// 				}
// 			}
// 		}
// 	}

// 	int res = INT_MIN;
// 	for (int f = g.getFirst(); f >= 0; f = g.getNext()) {
// 		if (hVal[f] == UNREACHABLE) {
// 			maxPrecG = -1;
// 			break;
// 		}
// 		if (res < hVal[f]) {
// 			res = hVal[f];
// 			maxPrecG = f;
// 		}
// 	}
// 	return res;
// }
} /* namespace progression */

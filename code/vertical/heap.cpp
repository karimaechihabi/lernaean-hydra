#include <iostream>
#include <cstdlib>
#include <cstring>

#include "heap.hpp"

using namespace std;

void init_heapNode(HeapNode* n, double temp_t_ub, double temp_t_lb,int temp_t_li){
  n->UBvalue =temp_t_ub;
  n->LBvalue =temp_t_lb;
  n->tsIndex =temp_t_li;
}

void setnewvalue(HeapNode* n, double temp_t_ub, double temp_t_lb,int temp_t_li){
 
  n->UBvalue = temp_t_ub;
  n->LBvalue = temp_t_lb;
  n->tsIndex = temp_t_li;
}

void init_maxheap(MaxHeap* h, int kKNN){
  h->maxSize = kKNN;
  h->k=0;
  h->boundArray = (HeapNode**) malloc(sizeof(HeapNode*) * h->maxSize);
  //CHECK
  memset(h->boundArray, 0, sizeof(HeapNode*) * h->maxSize);  
}

inline bool IsEmpty(MaxHeap* h){
  return h->k == 0;
}

bool insert(MaxHeap* h, int* heap_pos, double temp_t_ub, double temp_t_lb, int temp_t_li){

  if(h->k == h->maxSize)
    return false;
  HeapNode* newNode = (HeapNode*) malloc(sizeof(HeapNode));
  init_heapNode(newNode, temp_t_ub,temp_t_lb,temp_t_li);
  h->boundArray[h->k] = newNode;
  
  heap_pos[temp_t_li] = h->k;

  bubbleUp(h, heap_pos, h->k);
  h->k++;
  //cout << "a new candidate #" << temp_t_li << " with upper bound value: " << temp_t_ub << " enters the heap\n";

  return true; 
}

void bubbleUp(MaxHeap* h, int* heap_pos, int index){

  int parent = (index - 1) / 2;
  HeapNode* bottom = h->boundArray[index];

  while (index > 0 && 
	 getUBvalue(h->boundArray[parent]) < getUBvalue(bottom)){
    
    h->boundArray[index] = h->boundArray[parent];
    heap_pos[h->boundArray[index]->tsIndex] = index;
    index = parent;
    parent = (parent - 1) / 2;
  }

  h->boundArray[index] = bottom;
  
  heap_pos[h->boundArray[index]->tsIndex] = index;
  //heap_pos[bottom->tsIndex] = -1; //bottom extis the heap
}

void bubbleDown(MaxHeap* h, int* heap_pos, int index){

  int largerChild;
  HeapNode* top = h->boundArray[index];

  while (index < h->k/2){
    
    int leftChild = 2 * index + 1;
    int rightChild = leftChild + 1;
    double topUB = getUBvalue(top);
    // double leftChildUB = getUBvalue(h->boundArray[leftChild]);
    //double rightChildUB = getUBvalue(h->boundArray[rightChild]);
    if (rightChild < h->k && getUBvalue(h->boundArray[leftChild]) < getUBvalue(h->boundArray[rightChild]))
      largerChild = rightChild;
    else
      largerChild = leftChild;
    
    if (topUB >= getUBvalue(h->boundArray[largerChild]))
      break;

    h->boundArray[index] = h->boundArray[largerChild];
    heap_pos[h->boundArray[index]->tsIndex] = index;

    index = largerChild;
  }
  h->boundArray[index] = top;

  heap_pos[h->boundArray[index]->tsIndex] = index;
}

bool update (MaxHeap* h, int* heap_pos, double temp_t_ub, double temp_t_lb,int temp_t_li){

  int index;
  double oldUBValue;
  double oldLBValue;
  int oldIndex;

  if(heap_pos[temp_t_li]<0)
    index = 0;
  else
    index = heap_pos[temp_t_li];
  //cout << "---------------bubble down-------------" << endl;
  bubbleDown(h, heap_pos, index);
  //displayHeap(h);
  //cout << "---------------\bubble down-------------" << endl;

  if (index < 0 || index >= h->k)
    return false;

  oldUBValue = getUBvalue(h->boundArray[index]);
  oldLBValue = getLBvalue(h->boundArray[index]);
  oldIndex = getIndex(h->boundArray[index]);
  
  if (temp_t_ub < oldUBValue){
    heap_pos[oldIndex] = -1;
    setnewvalue(h->boundArray[index], temp_t_ub, temp_t_lb, temp_t_li); 
    bubbleDown(h, heap_pos, index);
    //cout << "The #" << temp_t_li << " candidate with upper bound value " << temp_t_ub << " entered the heap\n";
  }

  else if (temp_t_ub >= oldUBValue) {
    if(heap_pos[temp_t_li] >= 0){
      heap_pos[oldIndex]=-1;
      setnewvalue(h->boundArray[index], temp_t_ub, temp_t_lb, temp_t_li); 
      bubbleUp(h, heap_pos, index);
    }
    else{
      setnewvalue(h->boundArray[index], oldUBValue, oldLBValue, oldIndex);
      //heap_pos[oldIndex] = index;
    }
	//cout << "The #" << temp_t_li <<" candidate with upperbound value " << temp_t_ub <<" was pruned";
  }
  return true;
}

void displayHeap(MaxHeap* h){
  
  int emptyLeaf = 32;
  int itemsPerRow = 1;
  int column = 0;
  int j = 0;
  char separator [] = "...............................";
  
  cout<<endl;
  //cout << "Current upper bound values are: ";

  for (int m = 0; m < h->k; m++){
    if (h->boundArray[m] != NULL)
      cout << getUBvalue(h->boundArray[m]) << " #" << getIndex(h->boundArray[m]) << " ";
    else 
      cout << "-- ";
  }
  cout << endl;
  cout << separator << separator <<endl;
  while (h->k > 0){
    
    if (column == 0){
      for (int l = 0; l < emptyLeaf; l++)
	cout << ' ';
    }

    cout << getUBvalue(h->boundArray[j]) << "#" << getIndex(h->boundArray[j]);
    
    j++;
    if (j == h->k)
      break;

    column++;
    if (column == itemsPerRow){
      emptyLeaf /= 2;
      itemsPerRow *= 2;
      column = 0;
      cout << endl;
    }
    else {
      for(int l = 0; l<emptyLeaf * 2 - 2; l++)
	cout << " ";
    }
  }
  cout << "\n" << separator << separator << endl;
}

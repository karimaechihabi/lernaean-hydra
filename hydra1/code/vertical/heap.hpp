#ifndef HEAP_HPP
#define HEAP_HPP

typedef struct heapnode {
  double UBvalue;
  double LBvalue;
  int tsIndex;
}HeapNode;

typedef struct maxheap{
  HeapNode** boundArray;
  int maxSize;
  int k;

}MaxHeap;

void init_heapNode(HeapNode* n, double temp_t_ub, double temp_t_lb,int temp_t_li);
inline double getUBvalue(HeapNode* n){
  return n->UBvalue;  
}

inline double getLBvalue(HeapNode* n){
  return n->LBvalue;  
}

inline int getIndex(HeapNode* n){
  return n->tsIndex;
}


void setnewvalue(HeapNode* n, double temp_t_ub, double temp_t_lb, int temp_t_li);

void init_maxheap(MaxHeap* h, int kKNN);
bool isEmpty(MaxHeap* h);
bool insert(MaxHeap* h, int* heap_pos, double temp_t_ub, double temp_t_lb, int temp_t_li);
void bubbleUp(MaxHeap* h, int* heap_pos, int index);
void bubbleDown(MaxHeap* h, int* heap_pos, int index);
bool update (MaxHeap* h, int* heap_pos, double temp_t_ub, double temp_t_lb,int temp_t_li);
void displayHeap(MaxHeap* h);

#endif /* HEAP_HPP */
